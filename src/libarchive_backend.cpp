/*
 * Copyright (C) 2026 Alexandr Savca
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <libpkgimage/error.h>
#include <libpkgimage/libarchive_backend.h>

#include <archive.h>
#include <archive_entry.h>

#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace pkgimage {
namespace {

struct archive_deleter final {
  void operator()(archive* handle) const noexcept
  {
    if (handle != nullptr)
      archive_read_free(handle);
  }
};

using archive_handle = std::unique_ptr<archive, archive_deleter>;

[[noreturn]] void
backend_error(archive* handle, const std::filesystem::path& filename,
              const std::string& operation)
{
  const char* detail = archive_error_string(handle);
  std::ostringstream message;
  message << operation << " '" << filename.string() << "'";
  if (detail != nullptr)
    message << ": " << detail;
  throw archive_error(message.str());
}

void
enable_reader(archive* handle, int status, const char* feature)
{
  if (status == ARCHIVE_OK || status == ARCHIVE_WARN)
    return;

  const char* detail = archive_error_string(handle);
  std::ostringstream message;
  message << "could not enable libarchive " << feature;
  if (detail != nullptr)
    message << ": " << detail;
  throw archive_error(message.str());
}

std::uint64_t
nonnegative_value(la_int64_t value, const package_path& path,
                  const char* field)
{
  if (value < 0)
  {
    throw manifest_error(
        std::string("negative ") + field + " for '" + path.string() + "'");
  }
  return static_cast<std::uint64_t>(value);
}

entry_type
normalize_entry_type(archive_entry* entry, const package_path& path)
{
  if (archive_entry_hardlink(entry) != nullptr)
    return entry_type::hardlink;

  switch (archive_entry_filetype(entry))
  {
    case AE_IFREG: return entry_type::regular;
    case AE_IFDIR: return entry_type::directory;
    case AE_IFLNK: return entry_type::symlink;
    case AE_IFIFO: return entry_type::fifo;
    case AE_IFCHR: return entry_type::character_device;
    case AE_IFBLK: return entry_type::block_device;
    default:
      throw manifest_error(
          "unsupported archive entry type for '" + path.string() + "'");
  }
}

package_entry
normalize_entry(archive_entry* raw)
{
  const char* raw_path = archive_entry_pathname(raw);
  if (raw_path == nullptr)
    throw manifest_error("archive entry has no pathname");

  package_entry entry(package_path::parse(raw_path), entry_type::regular);
  entry.mode = static_cast<std::uint32_t>(archive_entry_perm(raw));
  entry.mtime = static_cast<std::int64_t>(archive_entry_mtime(raw));

  const long nanoseconds = archive_entry_mtime_nsec(raw);
  if (nanoseconds < 0 || nanoseconds > 999999999L)
  {
    throw manifest_error(
        "invalid mtime nanoseconds for '" + entry.path.string() + "'");
  }
  entry.mtime_nanoseconds = static_cast<std::uint32_t>(nanoseconds);

  entry.type = normalize_entry_type(raw, entry.path);
  entry.uid = nonnegative_value(archive_entry_uid(raw), entry.path, "uid");
  entry.gid = nonnegative_value(archive_entry_gid(raw), entry.path, "gid");

  if (entry.type == entry_type::regular)
  {
    entry.size =
        nonnegative_value(archive_entry_size(raw), entry.path, "size");
  }

  if (entry.type == entry_type::symlink)
  {
    const char* target = archive_entry_symlink(raw);
    if (target == nullptr)
    {
      throw manifest_error(
          "symlink has no target: '" + entry.path.string() + "'");
    }
    entry.symlink_target = std::string(target);
  }

  if (entry.type == entry_type::hardlink)
  {
    const char* target = archive_entry_hardlink(raw);
    if (target == nullptr)
    {
      throw manifest_error(
          "hardlink has no target: '" + entry.path.string() + "'");
    }
    entry.hardlink_target = package_path::parse(target);
  }

  if (entry.type == entry_type::character_device
      || entry.type == entry_type::block_device)
  {
    entry.device = device_number {
      nonnegative_value(
          archive_entry_rdevmajor(raw), entry.path, "device major"),
      nonnegative_value(
          archive_entry_rdevminor(raw), entry.path, "device minor"),
    };
  }

  return entry;
}

} // namespace

package_image
libarchive_backend::inspect(const std::filesystem::path& filename) const
{
  archive_handle handle(archive_read_new());
  if (!handle)
    throw archive_error("could not allocate libarchive reader");

  enable_reader(
      handle.get(), archive_read_support_filter_none(handle.get()),
      "none filter");
  enable_reader(
      handle.get(), archive_read_support_filter_gzip(handle.get()),
      "gzip filter");
  enable_reader(
      handle.get(), archive_read_support_filter_bzip2(handle.get()),
      "bzip2 filter");
  enable_reader(
      handle.get(), archive_read_support_filter_xz(handle.get()),
      "xz filter");
  enable_reader(
      handle.get(), archive_read_support_filter_lzip(handle.get()),
      "lzip filter");
  enable_reader(
      handle.get(), archive_read_support_filter_zstd(handle.get()),
      "zstd filter");
  enable_reader(
      handle.get(), archive_read_support_format_tar(handle.get()),
      "tar format");

  constexpr std::size_t block_size = 20U * 512U;
  if (archive_read_open_filename(handle.get(), filename.c_str(), block_size)
      != ARCHIVE_OK)
  {
    backend_error(handle.get(), filename, "could not open package archive");
  }

  std::vector<package_entry> entries;
  archive_entry* raw = nullptr;
  int status = ARCHIVE_OK;

  while ((status = archive_read_next_header(handle.get(), &raw)) == ARCHIVE_OK)
  {
    entries.push_back(normalize_entry(raw));

    if (archive_read_data_skip(handle.get()) != ARCHIVE_OK)
    {
      backend_error(
          handle.get(), filename, "could not skip package payload");
    }
  }

  if (status != ARCHIVE_EOF)
    backend_error(handle.get(), filename, "could not read package archive");

  if (archive_read_close(handle.get()) != ARCHIVE_OK)
    backend_error(handle.get(), filename, "could not close package archive");

  return package_image(std::move(entries));
}

} // namespace pkgimage
