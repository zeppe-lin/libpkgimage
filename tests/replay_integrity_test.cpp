// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include "test.h"

#include <libpkgimage/entry_selection.h>
#include <libpkgimage/error.h>
#include <libpkgimage/libarchive_backend.h>
#include <libpkgimage/payload_sink.h>

#include <archive.h>
#include <archive_entry.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {

class sink final : public pkgimage::payload_sink {
public:
  void begin(const pkgimage::package_entry&) override { began = true; }
  void write(const pkgimage::package_entry&, const std::byte*, std::size_t n)
      override { bytes += n; }
  void end(const pkgimage::package_entry&) override { ended = true; }

  bool began = false;
  bool ended = false;
  std::size_t bytes = 0;
};

class mutating_sink final : public pkgimage::payload_sink {
public:
  mutating_sink(std::filesystem::path path,
                off_t mutation_offset,
                std::string replacement)
      : fd_(open(path.c_str(), O_RDWR | O_CLOEXEC)),
        mutation_offset_(mutation_offset),
        replacement_(std::move(replacement))
  {
    CHECK(fd_ != -1);
  }

  ~mutating_sink() override
  {
    CHECK(close(fd_) == 0);
  }

  void begin(const pkgimage::package_entry&) override
  {
    began = true;
  }

  void write(const pkgimage::package_entry&, const std::byte*, std::size_t n)
      override
  {
    bytes += n;
    if (mutated)
      return;

    CHECK(pwrite(fd_, replacement_.data(), replacement_.size(),
                 mutation_offset_)
          == static_cast<ssize_t>(replacement_.size()));
    mutated = true;
  }

  void end(const pkgimage::package_entry&) override
  {
    ended = true;
  }

  bool began = false;
  bool mutated = false;
  bool ended = false;
  std::size_t bytes = 0;

private:
  int fd_;
  off_t mutation_offset_;
  std::string replacement_;
};

void
write_archive(const std::filesystem::path& path, std::string_view data)
{
  archive* writer = archive_write_new();
  CHECK(writer != nullptr);
  CHECK(archive_write_set_format_pax_restricted(writer) == ARCHIVE_OK);
  CHECK(archive_write_add_filter_none(writer) == ARCHIVE_OK);
  CHECK(archive_write_open_filename(writer, path.c_str()) == ARCHIVE_OK);

  archive_entry* entry = archive_entry_new();
  CHECK(entry != nullptr);
  archive_entry_set_pathname(entry, "payload");
  archive_entry_set_filetype(entry, AE_IFREG);
  archive_entry_set_perm(entry, 0644);
  archive_entry_set_uid(entry, 0);
  archive_entry_set_gid(entry, 0);
  archive_entry_set_mtime(entry, 1, 0);
  archive_entry_set_size(entry, static_cast<la_int64_t>(data.size()));
  CHECK(archive_write_header(writer, entry) == ARCHIVE_OK);
  CHECK(archive_write_data(writer, data.data(), data.size())
        == static_cast<la_ssize_t>(data.size()));
  archive_entry_free(entry);
  CHECK(archive_write_close(writer) == ARCHIVE_OK);
  CHECK(archive_write_free(writer) == ARCHIVE_OK);
}

std::string
read_file(const std::filesystem::path& path)
{
  std::ifstream input(path, std::ios::binary);
  CHECK(input.good());
  return std::string(std::istreambuf_iterator<char>(input),
                     std::istreambuf_iterator<char>());
}

} // namespace

int
main()
{
  char directory[] = "/tmp/libpkgimage-integrity.XXXXXX";
  CHECK(mkdtemp(directory) != nullptr);

  pkgimage::libarchive_backend backend;

  const std::filesystem::path changed_before_path =
      std::filesystem::path(directory) / "changed-before.pkg.tar";
  write_archive(changed_before_path, "old");

  auto changed_before = backend.open(changed_before_path);
  const auto changed_before_selection =
      pkgimage::entry_selection::all_regular(changed_before->image());

  struct stat stamp {};
  CHECK(stat(changed_before_path.c_str(), &stamp) == 0);

  int fd = open(changed_before_path.c_str(), O_RDWR | O_CLOEXEC);
  CHECK(fd != -1);
  std::string archive_bytes = read_file(changed_before_path);
  const std::size_t old_offset = archive_bytes.find("old");
  CHECK(old_offset != std::string::npos);
  CHECK(pwrite(fd, "new", 3, static_cast<off_t>(old_offset)) == 3);
  CHECK(close(fd) == 0);

  const timespec times[2] = {stamp.st_atim, stamp.st_mtim};
  CHECK(utimensat(AT_FDCWD, changed_before_path.c_str(), times, 0) == 0);

  sink before_destination;
  check_throws<pkgimage::source_changed_error>([&] {
    changed_before->replay(changed_before_selection, before_destination);
  });
  CHECK(!before_destination.began);
  CHECK(!before_destination.ended);

  const std::filesystem::path changed_during_path =
      std::filesystem::path(directory) / "changed-during.pkg.tar";
  constexpr std::string_view marker = "ORIGINAL-CONTENT";
  constexpr std::string_view replacement = "MUTATED--CONTENT";
  static_assert(marker.size() == replacement.size());

  std::string large(2U * 1024U * 1024U, 'A');
  large.replace(1024U * 1024U, marker.size(), marker);
  write_archive(changed_during_path, large);

  auto changed_during = backend.open(changed_during_path);
  const auto changed_during_selection =
      pkgimage::entry_selection::all_regular(changed_during->image());

  archive_bytes = read_file(changed_during_path);
  const std::size_t marker_offset = archive_bytes.find(marker);
  CHECK(marker_offset != std::string::npos);

  mutating_sink during_destination(
      changed_during_path, static_cast<off_t>(marker_offset),
      std::string(replacement));

  bool caught_content_mismatch = false;
  try
  {
    changed_during->replay(changed_during_selection, during_destination);
  }
  catch (const pkgimage::regular_payload_digest_mismatch_error& error)
  {
    caught_content_mismatch = true;
    CHECK(error.path() == "payload");
    CHECK(error.expected() != error.actual());
  }
  CHECK(caught_content_mismatch);
  CHECK(during_destination.began);
  CHECK(during_destination.mutated);
  CHECK(during_destination.bytes != 0);
  CHECK(!during_destination.ended);

  std::error_code ignored;
  std::filesystem::remove_all(directory, ignored);
  return EXIT_SUCCESS;
}
