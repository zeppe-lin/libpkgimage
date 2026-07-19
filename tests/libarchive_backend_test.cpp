// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include "test.h"

#include <libpkgimage/error.h>
#include <libpkgimage/libarchive_backend.h>

#include <archive.h>
#include <archive_entry.h>

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

using pkgimage::entry_type;
using pkgimage::libarchive_backend;
using pkgimage::manifest_error;
using pkgimage::package_image;
using pkgimage::package_path;
using pkgimage::path_error;

namespace {

struct archive_spec final {
  std::string path;
  mode_t type;
  mode_t permissions;
  std::string data;
  std::string link_target;
  std::uint64_t device_major = 0;
  std::uint64_t device_minor = 0;
};

class temporary_directory final {
public:
  temporary_directory()
  {
    std::string pattern = "/tmp/libpkgimage-test.XXXXXX";
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

    if (spec.type == AE_IFCHR || spec.type == AE_IFBLK)
    {
      archive_entry_set_rdevmajor(
          raw, static_cast<dev_t>(spec.device_major));
      archive_entry_set_rdevminor(
          raw, static_cast<dev_t>(spec.device_minor));
    }

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

} // namespace

int
main()
{
  temporary_directory temp;
  const libarchive_backend backend;

  const auto valid_archive = temp.file("valid.pkg.tar");
  write_archive(valid_archive, {
    {"./var/lib/example-empty/", AE_IFDIR, 0755, {}, {}},
    {"./etc//example/./file.conf", AE_IFREG, 0640, "value=1\n", {}},
    {"usr/bin/example", AE_IFLNK, 0777, {},
     "../../etc/example/file.conf"},
    {"etc/example/file.hard", AE_IFREG, 0640, {},
     "etc/example/file.conf"},
    {"run/example.pipe", AE_IFIFO, 0600, {}, {}},
    {"dev/example", AE_IFCHR, 0600, {}, {}, 1, 7},
  });

  const package_image image = backend.inspect(valid_archive);
  CHECK(image.size() == 6);
  CHECK(image.entries()[0].path.string() == "var/lib/example-empty");
  CHECK(image.entries()[0].type == entry_type::directory);
  CHECK(image.entries()[1].path.string() == "etc/example/file.conf");
  CHECK(image.entries()[1].type == entry_type::regular);
  CHECK(image.entries()[1].size == 8);
  CHECK(image.entries()[2].type == entry_type::symlink);
  CHECK(image.entries()[2].symlink_target
        == "../../etc/example/file.conf");
  CHECK(image.entries()[3].type == entry_type::hardlink);
  CHECK(image.entries()[3].hardlink_target
        == package_path::parse("etc/example/file.conf"));
  CHECK(image.entries()[4].type == entry_type::fifo);
  CHECK(image.entries()[5].type == entry_type::character_device);
  CHECK(image.entries()[5].device.has_value());
  CHECK(image.entries()[5].device->major == 1);
  CHECK(image.entries()[5].device->minor == 7);
  CHECK(image.find(package_path::parse("var/lib")) == nullptr);
  CHECK(image.find(package_path::parse("usr/bin")) == nullptr);

  const auto duplicate_archive = temp.file("duplicate.pkg.tar");
  write_archive(duplicate_archive, {
    {"./usr/bin/tool", AE_IFREG, 0755, {}, {}},
    {"usr//bin/tool", AE_IFREG, 0755, {}, {}},
  });
  check_throws<manifest_error>(
      [&] { (void)backend.inspect(duplicate_archive); });

  const auto traversal_archive = temp.file("traversal.pkg.tar");
  write_archive(traversal_archive, {
    {"../outside", AE_IFREG, 0644, {}, {}},
  });
  check_throws<path_error>(
      [&] { (void)backend.inspect(traversal_archive); });

  const auto missing_hardlink_archive =
      temp.file("missing-hardlink.pkg.tar");
  write_archive(missing_hardlink_archive, {
    {"usr/bin/tool", AE_IFREG, 0755, {}, "usr/libexec/tool"},
  });
  check_throws<manifest_error>(
      [&] { (void)backend.inspect(missing_hardlink_archive); });

  return EXIT_SUCCESS;
}
