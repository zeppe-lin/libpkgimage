/*
 * Copyright (C) 2026 Alexandr Savca
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "test.h"

#include <libpkgimage/entry_selection.h>
#include <libpkgimage/error.h>
#include <libpkgimage/libarchive_backend.h>
#include <libpkgimage/payload_sink.h>

#include <archive.h>
#include <archive_entry.h>

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <fcntl.h>
#include <unistd.h>

using pkgimage::entry_id;
using pkgimage::entry_selection;
using pkgimage::entry_type;
using pkgimage::libarchive_backend;
using pkgimage::package_archive;
using pkgimage::package_entry;
using pkgimage::package_path;
using pkgimage::payload_sink;
using pkgimage::source_changed_error;

namespace {

struct archive_spec final {
  std::string path;
  mode_t type;
  mode_t permissions;
  std::string data;
  std::string link_target;
};

class temporary_directory final {
public:
  temporary_directory()
  {
    std::string pattern = "/tmp/libpkgimage-payload.XXXXXX";
    pattern.push_back('\0');
    char* created = mkdtemp(pattern.data());
    if (created == nullptr)
    {
      std::perror("mkdtemp");
      std::exit(EXIT_FAILURE);
    }
    path_ = created;
  }

  ~temporary_directory()
  {
    std::error_code ignored;
    std::filesystem::remove_all(path_, ignored);
  }

  temporary_directory(const temporary_directory&) = delete;
  temporary_directory& operator=(const temporary_directory&) = delete;

  [[nodiscard]] std::filesystem::path file(const char* name) const
  {
    return path_ / name;
  }

private:
  std::filesystem::path path_;
};

[[noreturn]] void
write_failure(archive* writer, const char* operation)
{
  const char* detail = archive_error_string(writer);
  std::cerr << operation;
  if (detail != nullptr)
    std::cerr << ": " << detail;
  std::cerr << '\n';
  std::exit(EXIT_FAILURE);
}

void
write_archive(const std::filesystem::path& filename,
              const std::vector<archive_spec>& specs)
{
  archive* writer = archive_write_new();
  CHECK(writer != nullptr);
  CHECK(archive_write_set_format_pax_restricted(writer) == ARCHIVE_OK);
  CHECK(archive_write_add_filter_none(writer) == ARCHIVE_OK);

  if (archive_write_open_filename(writer, filename.c_str()) != ARCHIVE_OK)
    write_failure(writer, "archive_write_open_filename");

  for (const archive_spec& spec : specs)
  {
    archive_entry* raw = archive_entry_new();
    CHECK(raw != nullptr);
    archive_entry_set_pathname(raw, spec.path.c_str());
    archive_entry_set_filetype(raw, spec.type);
    archive_entry_set_perm(raw, spec.permissions);
    archive_entry_set_uid(raw, 0);
    archive_entry_set_gid(raw, 0);
    archive_entry_set_mtime(raw, 1, 0);

    if (spec.type == AE_IFREG && spec.link_target.empty())
      archive_entry_set_size(raw, static_cast<la_int64_t>(spec.data.size()));
    else
      archive_entry_set_size(raw, 0);

    if (spec.type == AE_IFLNK)
      archive_entry_set_symlink(raw, spec.link_target.c_str());
    else if (!spec.link_target.empty())
      archive_entry_set_hardlink(raw, spec.link_target.c_str());

    if (archive_write_header(writer, raw) != ARCHIVE_OK)
      write_failure(writer, "archive_write_header");

    if (!spec.data.empty()
        && archive_write_data(writer, spec.data.data(), spec.data.size())
           != static_cast<la_ssize_t>(spec.data.size()))
    {
      write_failure(writer, "archive_write_data");
    }

    archive_entry_free(raw);
  }

  if (archive_write_close(writer) != ARCHIVE_OK)
    write_failure(writer, "archive_write_close");
  CHECK(archive_write_free(writer) == ARCHIVE_OK);
}

class collecting_sink final : public payload_sink {
public:
  void begin(const package_entry& entry) override
  {
    CHECK(active_ == pkgimage::invalid_entry_id);
    active_ = entry.id;
    events.push_back("begin:" + entry.path.string());
  }

  void write(const package_entry& entry,
             const std::byte* data,
             std::size_t size) override
  {
    CHECK(active_ == entry.id);
    payloads[entry.path.string()].append(
        reinterpret_cast<const char*>(data), size);
    ++writes[entry.path.string()];
  }

  void end(const package_entry& entry) override
  {
    CHECK(active_ == entry.id);
    events.push_back("end:" + entry.path.string());
    active_ = pkgimage::invalid_entry_id;
  }

  std::vector<std::string> events;
  std::map<std::string, std::string> payloads;
  std::map<std::string, std::size_t> writes;

private:
  entry_id active_ = pkgimage::invalid_entry_id;
};

class sink_failure final : public std::runtime_error {
public:
  sink_failure()
      : std::runtime_error("sink failure")
  {
  }
};

class failing_sink final : public payload_sink {
public:
  void begin(const package_entry&) override
  {
  }

  void write(const package_entry&, const std::byte*, std::size_t) override
  {
    throw sink_failure();
  }

  void end(const package_entry&) override
  {
    CHECK(false);
  }
};

[[nodiscard]] entry_id
id_of(const package_archive& package, const char* path)
{
  const package_entry* entry =
      package.image().find(package_path::parse(path));
  CHECK(entry != nullptr);
  return entry->id;
}

void
overwrite_one_byte(const std::filesystem::path& filename)
{
  int fd = ::open(filename.c_str(), O_WRONLY | O_CLOEXEC);
  CHECK(fd != -1);

  const char byte = 'X';
  ssize_t count;
  do
  {
    count = pwrite(fd, &byte, 1, 0);
  }
  while (count == -1 && errno == EINTR);
  CHECK(count == 1);
  CHECK(close(fd) == 0);
}

} // namespace

int
main()
{
  temporary_directory temp;
  const libarchive_backend backend;

  const std::string first_data = "first payload\n";
  const std::string large_data(1024U * 1024U, 'L');
  const auto archive_path = temp.file("payload.pkg.tar");
  write_archive(archive_path, {
    {"usr/share/example", AE_IFDIR, 0755, {}, {}},
    {"usr/share/example/first", AE_IFREG, 0644, first_data, {}},
    {"usr/share/example/empty", AE_IFREG, 0644, {}, {}},
    {"usr/share/example/large", AE_IFREG, 0644, large_data, {}},
    {"usr/bin/example", AE_IFLNK, 0777, {},
     "../share/example/first"},
    {"usr/share/example/first.hard", AE_IFREG, 0644, {},
     "usr/share/example/first"},
  });

  std::unique_ptr<package_archive> package = backend.open(archive_path);
  CHECK(package->image().size() == 6);

  const entry_id empty_id = id_of(*package, "usr/share/example/empty");
  const entry_id large_id = id_of(*package, "usr/share/example/large");

  const entry_selection selected =
      entry_selection::from_ids(package->image(), {large_id, empty_id});
  collecting_sink selected_sink;
  package->replay(selected, selected_sink);

  CHECK(selected_sink.events.size() == 4);
  CHECK(selected_sink.events[0] == "begin:usr/share/example/empty");
  CHECK(selected_sink.events[1] == "end:usr/share/example/empty");
  CHECK(selected_sink.events[2] == "begin:usr/share/example/large");
  CHECK(selected_sink.events[3] == "end:usr/share/example/large");
  CHECK(selected_sink.payloads["usr/share/example/empty"].empty());
  CHECK(selected_sink.writes["usr/share/example/empty"] == 0);
  CHECK(selected_sink.payloads["usr/share/example/large"] == large_data);
  CHECK(selected_sink.writes["usr/share/example/large"] > 1);
  CHECK(selected_sink.payloads.count("usr/share/example/first") == 0);

  collecting_sink all_sink;
  package->replay(
      entry_selection::all_regular(package->image()), all_sink);
  CHECK(all_sink.payloads["usr/share/example/first"] == first_data);
  CHECK(all_sink.payloads["usr/share/example/empty"].empty());
  CHECK(all_sink.payloads["usr/share/example/large"] == large_data);
  CHECK(all_sink.payloads.count("usr/share/example/first.hard") == 0);

  collecting_sink empty_sink;
  package->replay(
      entry_selection::from_ids(package->image(), {}), empty_sink);
  CHECK(empty_sink.events.empty());

  failing_sink broken_sink;
  check_throws<sink_failure>([&] {
    package->replay(
        entry_selection::from_ids(package->image(), {large_id}),
        broken_sink);
  });

  const auto replacement_path = temp.file("replacement.pkg.tar");
  write_archive(replacement_path, {
    {"payload", AE_IFREG, 0644, "old", {}},
  });
  std::unique_ptr<package_archive> stable = backend.open(replacement_path);
  const entry_selection stable_selection =
      entry_selection::all_regular(stable->image());

  const auto new_path = temp.file("replacement.new");
  write_archive(new_path, {
    {"payload", AE_IFREG, 0644, "new", {}},
  });
  std::filesystem::rename(new_path, replacement_path);

  collecting_sink stable_sink;
  stable->replay(stable_selection, stable_sink);
  CHECK(stable_sink.payloads["payload"] == "old");

  const auto changed_path = temp.file("changed.pkg.tar");
  write_archive(changed_path, {
    {"payload", AE_IFREG, 0644, "unchanged", {}},
  });
  std::unique_ptr<package_archive> changed = backend.open(changed_path);
  const entry_selection changed_selection =
      entry_selection::all_regular(changed->image());
  overwrite_one_byte(changed_path);

  collecting_sink changed_sink;
  check_throws<source_changed_error>(
      [&] { changed->replay(changed_selection, changed_sink); });
  CHECK(changed_sink.events.empty());

  return EXIT_SUCCESS;
}
