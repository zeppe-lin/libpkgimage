// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include "test.h"

#include <libpkgimage/libarchive_backend.h>

#include <archive.h>
#include <archive_entry.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

namespace {

enum class compression {
  none,
  gzip,
};

class temporary_directory final {
public:
  temporary_directory()
  {
    std::string pattern = "/tmp/libpkgimage-identity.XXXXXX";
    pattern.push_back('\0');
    char* result = mkdtemp(pattern.data());
    CHECK(result != nullptr);
    path_ = result;
  }

  ~temporary_directory()
  {
    std::error_code ignored;
    std::filesystem::remove_all(path_, ignored);
  }

  [[nodiscard]] std::filesystem::path file(const char* name) const
  {
    return path_ / name;
  }

private:
  std::filesystem::path path_;
};

void
write_entry(archive* writer, const char* path, mode_t mode,
            std::string_view data)
{
  archive_entry* entry = archive_entry_new();
  CHECK(entry != nullptr);
  archive_entry_set_pathname(entry, path);
  archive_entry_set_filetype(entry, AE_IFREG);
  archive_entry_set_perm(entry, mode);
  archive_entry_set_uid(entry, 11);
  archive_entry_set_gid(entry, 22);
  archive_entry_set_mtime(entry, 33, 44);
  archive_entry_set_size(entry, static_cast<la_int64_t>(data.size()));
  CHECK(archive_write_header(writer, entry) == ARCHIVE_OK);
  if (!data.empty())
  {
    CHECK(archive_write_data(writer, data.data(), data.size())
          == static_cast<la_ssize_t>(data.size()));
  }
  archive_entry_free(entry);
}

void
write_archive(const std::filesystem::path& path,
              compression filter,
              std::string_view first,
              mode_t mode = 0644)
{
  archive* writer = archive_write_new();
  CHECK(writer != nullptr);
  CHECK(archive_write_set_format_pax_restricted(writer) == ARCHIVE_OK);
  const int filter_status = filter == compression::none
      ? archive_write_add_filter_none(writer)
      : archive_write_add_filter_gzip(writer);
  CHECK(filter_status == ARCHIVE_OK);
  CHECK(archive_write_open_filename(writer, path.c_str()) == ARCHIVE_OK);
  write_entry(writer, "payload", mode, first);
  write_entry(writer, "empty", 0600, {});
  CHECK(archive_write_close(writer) == ARCHIVE_OK);
  CHECK(archive_write_free(writer) == ARCHIVE_OK);
}

} // namespace

int
main()
{
  temporary_directory temp;
  const auto plain_path = temp.file("same.pkg.tar");
  const auto gzip_path = temp.file("same.pkg.tar.gz");
  const auto copy_path = temp.file("renamed.pkg.tar");
  const auto changed_path = temp.file("changed.pkg.tar");
  const auto mode_path = temp.file("mode.pkg.tar");
  const auto trailing_path = temp.file("trailing.pkg.tar");

  write_archive(plain_path, compression::none, "abc");
  write_archive(gzip_path, compression::gzip, "abc");
  write_archive(changed_path, compression::none, "xyz");
  write_archive(mode_path, compression::none, "abc", 0600);
  std::filesystem::copy_file(plain_path, copy_path);
  std::filesystem::copy_file(plain_path, trailing_path);
  {
    std::ofstream trailing(trailing_path, std::ios::binary | std::ios::app);
    CHECK(trailing.good());
    const std::string extra(512, '\0');
    trailing.write(extra.data(), static_cast<std::streamsize>(extra.size()));
    CHECK(trailing.good());
  }

  pkgimage::libarchive_backend backend;
  const auto plain = backend.inspect(plain_path);
  const auto repeated = backend.inspect(plain_path);
  const auto gzip = backend.inspect(gzip_path);
  const auto renamed = backend.inspect(copy_path);
  const auto changed = backend.inspect(changed_path);
  const auto changed_mode = backend.inspect(mode_path);
  const auto trailing = backend.inspect(trailing_path);

  CHECK(plain.receipt().archive_digest()
        == repeated.receipt().archive_digest());
  CHECK(plain.image().identity() == repeated.image().identity());
  CHECK(plain.receipt().identity() == repeated.receipt().identity());

  CHECK(plain.receipt().archive_digest()
        != gzip.receipt().archive_digest());
  CHECK(plain.image().identity() == gzip.image().identity());
  CHECK(plain.receipt().identity() != gzip.receipt().identity());

  CHECK(plain.receipt().archive_digest()
        == renamed.receipt().archive_digest());
  CHECK(plain.image().identity() == renamed.image().identity());
  CHECK(plain.receipt().identity() == renamed.receipt().identity());

  CHECK(plain.image().identity() == trailing.image().identity());
  CHECK(plain.receipt().archive_digest()
        != trailing.receipt().archive_digest());
  CHECK(plain.receipt().identity() != trailing.receipt().identity());

  CHECK(plain.image().entries()[0].regular_content
        != changed.image().entries()[0].regular_content);
  CHECK(plain.image().identity() != changed.image().identity());
  CHECK(plain.receipt().archive_digest()
        != changed.receipt().archive_digest());
  CHECK(plain.receipt().identity() != changed.receipt().identity());

  CHECK(plain.image().identity() != changed_mode.image().identity());

  CHECK(plain.image().entries()[1].regular_content->string()
        == "v1:sha256:e3b0c44298fc1c149afbf4c8996fb924"
           "27ae41e4649b934ca495991b7852b855");

  return EXIT_SUCCESS;
}
