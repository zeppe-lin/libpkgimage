// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include <libpkgimage/error.h>
#include <libpkgimage/libarchive_backend.h>

#include "sha256.h"

#include <archive.h>
#include <archive_entry.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace pkgimage {
namespace {

class fd_handle final {
public:
  explicit fd_handle(int fd = -1) noexcept
      : fd_(fd)
  {
  }

  ~fd_handle()
  {
    if (fd_ != -1)
      close(fd_);
  }

  fd_handle(const fd_handle&) = delete;
  fd_handle& operator=(const fd_handle&) = delete;

  fd_handle(fd_handle&& other) noexcept
      : fd_(other.fd_)
  {
    other.fd_ = -1;
  }

  fd_handle& operator=(fd_handle&& other) noexcept
  {
    if (this == &other)
      return *this;

    if (fd_ != -1)
      close(fd_);

    fd_ = other.fd_;
    other.fd_ = -1;
    return *this;
  }

  [[nodiscard]] int get() const noexcept
  {
    return fd_;
  }

private:
  int fd_;
};

struct archive_deleter final {
  void operator()(archive* handle) const noexcept
  {
    if (handle != nullptr)
      archive_read_free(handle);
  }
};

using archive_handle = std::unique_ptr<archive, archive_deleter>;

struct source_time final {
  std::int64_t seconds;
  std::int64_t nanoseconds;
};

struct source_stamp final {
  std::uint64_t device;
  std::uint64_t inode;
  std::uint64_t size;
  source_time mtime;
};

[[nodiscard]] bool
operator==(const source_time& lhs, const source_time& rhs) noexcept
{
  return lhs.seconds == rhs.seconds && lhs.nanoseconds == rhs.nanoseconds;
}

[[nodiscard]] bool
operator==(const source_stamp& lhs, const source_stamp& rhs) noexcept
{
  return lhs.device == rhs.device
      && lhs.inode == rhs.inode
      && lhs.size == rhs.size
      && lhs.mtime == rhs.mtime;
}

[[nodiscard]] source_stamp
read_source_stamp(int fd, const std::filesystem::path& filename)
{
  struct stat status {};
  int result;
  do
  {
    result = fstat(fd, &status);
  }
  while (result == -1 && errno == EINTR);

  if (result == -1)
  {
    throw archive_error(
        "could not inspect package archive '" + filename.string()
        + "': " + std::strerror(errno));
  }

  if (!S_ISREG(status.st_mode))
  {
    throw archive_error(
        "package archive is not a regular file: '"
        + filename.string() + "'");
  }

  if (status.st_size < 0)
  {
    throw archive_error(
        "package archive has a negative size: '"
        + filename.string() + "'");
  }

  return source_stamp {
    static_cast<std::uint64_t>(status.st_dev),
    static_cast<std::uint64_t>(status.st_ino),
    static_cast<std::uint64_t>(status.st_size),
    source_time {
      static_cast<std::int64_t>(status.st_mtim.tv_sec),
      static_cast<std::int64_t>(status.st_mtim.tv_nsec),
    },
  };
}

[[nodiscard]] fd_handle
open_source(const std::filesystem::path& filename)
{
  int fd;
  do
  {
    fd = ::open(filename.c_str(), O_RDONLY | O_CLOEXEC);
  }
  while (fd == -1 && errno == EINTR);

  if (fd == -1)
  {
    throw archive_error(
        "could not open package archive '" + filename.string()
        + "': " + std::strerror(errno));
  }

  return fd_handle(fd);
}

[[nodiscard]] complete_archive_digest
hash_source(int fd, const source_stamp& stamp,
            const std::filesystem::path& filename)
{
  detail::sha256_context hash;
  std::array<unsigned char, 64U * 1024U> buffer {};
  std::uint64_t offset = 0;

  while (offset < stamp.size)
  {
    const std::size_t request = static_cast<std::size_t>(
        std::min<std::uint64_t>(stamp.size - offset, buffer.size()));
    ssize_t count;
    do
    {
      count = pread(fd, buffer.data(), request, static_cast<off_t>(offset));
    }
    while (count == -1 && errno == EINTR);

    if (count == -1)
    {
      throw archive_error(
          "could not hash package archive '" + filename.string()
          + "': " + std::strerror(errno));
    }
    if (count == 0)
    {
      throw source_changed_error(
          "package archive source was truncated while hashing: '"
          + filename.string() + "'");
    }

    hash.update(buffer.data(), static_cast<std::size_t>(count));
    offset += static_cast<std::uint64_t>(count);
  }

  return complete_archive_digest::from_sha256(hash.finish());
}

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

struct reader_context final {
  int fd;
  std::uint64_t size;
  std::uint64_t offset = 0;
  std::array<unsigned char, 64U * 1024U> buffer {};
};

la_ssize_t
read_source(archive* handle, void* client_data, const void** buffer)
{
  auto& source = *static_cast<reader_context*>(client_data);
  if (source.offset >= source.size)
  {
    *buffer = source.buffer.data();
    return 0;
  }

  const std::uint64_t remaining = source.size - source.offset;
  const std::size_t request = static_cast<std::size_t>(
      std::min<std::uint64_t>(remaining, source.buffer.size()));

  ssize_t count;
  do
  {
    count = pread(
        source.fd, source.buffer.data(), request,
        static_cast<off_t>(source.offset));
  }
  while (count == -1 && errno == EINTR);

  if (count == -1)
  {
    archive_set_error(handle, errno, "could not read package archive source");
    return -1;
  }

  if (count == 0)
  {
    archive_set_error(handle, EIO, "package archive source was truncated");
    return -1;
  }

  source.offset += static_cast<std::uint64_t>(count);
  *buffer = source.buffer.data();
  return static_cast<la_ssize_t>(count);
}

[[nodiscard]] archive_handle
open_reader(reader_context& source,
            const std::filesystem::path& filename)
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

  if (archive_read_open(
          handle.get(), &source, nullptr, read_source, nullptr)
      != ARCHIVE_OK)
  {
    backend_error(handle.get(), filename, "could not open package archive");
  }

  return handle;
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

[[nodiscard]] package_image
inspect_source(int fd, const source_stamp& stamp,
               const std::filesystem::path& filename)
{
  reader_context source {fd, stamp.size};
  archive_handle handle = open_reader(source, filename);

  std::vector<package_entry> entries;
  std::array<unsigned char, 64U * 1024U> payload {};
  archive_entry* raw = nullptr;
  int status = ARCHIVE_OK;

  while ((status = archive_read_next_header(handle.get(), &raw)) == ARCHIVE_OK)
  {
    package_entry entry = normalize_entry(raw);

    if (entry.type == entry_type::regular)
    {
      detail::sha256_context content_hash;
      std::uint64_t total = 0;
      la_ssize_t count;
      while ((count = archive_read_data(
                  handle.get(), payload.data(), payload.size())) > 0)
      {
        const std::uint64_t bytes = static_cast<std::uint64_t>(count);
        if (bytes > entry.size - total)
        {
          throw declared_size_mismatch_error(
              "package payload exceeds declared size for '"
              + entry.path.string() + "'");
        }
        content_hash.update(payload.data(), static_cast<std::size_t>(count));
        total += bytes;
      }

      if (count < 0)
      {
        backend_error(
            handle.get(), filename, "could not read package payload");
      }
      if (total != entry.size)
      {
        throw declared_size_mismatch_error(
            "package payload size mismatch for '" + entry.path.string() + "'");
      }

      entry.regular_content = regular_content_digest::from_sha256(
          content_hash.finish());
    }
    else if (archive_read_data_skip(handle.get()) != ARCHIVE_OK)
    {
      backend_error(
          handle.get(), filename, "could not skip package payload");
    }

    entries.push_back(std::move(entry));
  }

  if (status != ARCHIVE_EOF)
    backend_error(handle.get(), filename, "could not read package archive");

  if (archive_read_close(handle.get()) != ARCHIVE_OK)
    backend_error(handle.get(), filename, "could not close package archive");

  return package_image(std::move(entries));
}

void
verify_source(int fd, const source_stamp& expected,
              const std::filesystem::path& filename)
{
  if (!(read_source_stamp(fd, filename) == expected))
  {
    throw source_changed_error(
        "package archive source changed after inspection: '"
        + filename.string() + "'");
  }
}

void
verify_sealed_source(int fd, const source_stamp& expected,
                     const complete_archive_digest& expected_digest,
                     const std::filesystem::path& filename)
{
  verify_source(fd, expected, filename);
  const complete_archive_digest actual = hash_source(fd, expected, filename);
  verify_source(fd, expected, filename);
  if (actual != expected_digest)
  {
    throw source_changed_error(
        "package archive bytes changed after inspection: '"
        + filename.string() + "'");
  }
}

[[nodiscard]] bool
same_entry_header(const package_entry& actual,
                  const package_entry& expected) noexcept
{
  return actual.id == expected.id
      && actual.path == expected.path
      && actual.type == expected.type
      && actual.mode == expected.mode
      && actual.uid == expected.uid
      && actual.gid == expected.gid
      && actual.size == expected.size
      && actual.mtime == expected.mtime
      && actual.mtime_nanoseconds == expected.mtime_nanoseconds
      && actual.symlink_target == expected.symlink_target
      && actual.hardlink_target == expected.hardlink_target
      && actual.device == expected.device;
}

class libarchive_package final : public package_archive {
public:
  libarchive_package(std::filesystem::path filename,
                     fd_handle source,
                     source_stamp stamp,
                     archive_inspection_receipt receipt,
                     package_image image)
      : filename_(std::move(filename)),
        source_(std::move(source)),
        stamp_(stamp),
        receipt_(std::move(receipt)),
        image_(std::move(image))
  {
  }

  [[nodiscard]] const package_image& image() const noexcept override
  {
    return image_;
  }

  [[nodiscard]] const archive_inspection_receipt&
  inspection_receipt() const noexcept override
  {
    return receipt_;
  }

  void replay(const entry_selection& selection,
              payload_sink& sink) const override
  {
    selection.validate(image_);
    verify_sealed_source(
        source_.get(), stamp_, receipt_.archive_digest(), filename_);

    reader_context source {source_.get(), stamp_.size};
    archive_handle handle = open_reader(source, filename_);
    std::array<unsigned char, 64U * 1024U> payload {};

    archive_entry* raw = nullptr;
    int status = ARCHIVE_OK;
    std::size_t index = 0;
    std::size_t replayed = 0;

    while ((status = archive_read_next_header(handle.get(), &raw))
           == ARCHIVE_OK)
    {
      const package_entry* expected = image_.entry(index);
      if (expected == nullptr)
      {
        throw source_changed_error(
            "package archive manifest changed during replay: '"
            + filename_.string() + "'");
      }

      package_entry actual = normalize_entry(raw);
      actual.id = index;
      if (!same_entry_header(actual, *expected))
      {
        throw source_changed_error(
            "package archive manifest changed during replay at '"
            + expected->path.string() + "'");
      }

      if (selection.contains(expected->id))
      {
        sink.begin(*expected);

        detail::sha256_context content_hash;
        std::uint64_t total = 0;
        la_ssize_t count;
        while ((count = archive_read_data(
                    handle.get(), payload.data(), payload.size())) > 0)
        {
          const auto bytes = static_cast<std::uint64_t>(count);
          if (bytes > expected->size - total)
          {
            throw declared_size_mismatch_error(
                "package payload exceeds declared size for '"
                + expected->path.string() + "'");
          }

          sink.write(
              *expected,
              reinterpret_cast<const std::byte*>(payload.data()),
              static_cast<std::size_t>(count));
          content_hash.update(payload.data(), static_cast<std::size_t>(count));
          total += bytes;
        }

        if (count < 0)
        {
          backend_error(
              handle.get(), filename_, "could not read package payload");
        }

        if (total != expected->size)
        {
          throw declared_size_mismatch_error(
              "package payload size mismatch for '"
              + expected->path.string() + "'");
        }

        if (!expected->regular_content)
        {
          throw source_changed_error(
              "sealed package image lost regular-content identity at '"
              + expected->path.string() + "'");
        }

        const regular_content_digest actual_content =
            regular_content_digest::from_sha256(content_hash.finish());
        if (actual_content != *expected->regular_content)
        {
          throw regular_payload_digest_mismatch_error(
              *expected->regular_content, actual_content,
              expected->path.string());
        }

        sink.end(*expected);
        ++replayed;
      }
      else if (archive_read_data_skip(handle.get()) != ARCHIVE_OK)
      {
        backend_error(
            handle.get(), filename_, "could not skip package payload");
      }

      ++index;
    }

    if (status != ARCHIVE_EOF)
      backend_error(handle.get(), filename_, "could not read package archive");

    if (index != image_.size() || replayed != selection.size())
    {
      throw source_changed_error(
          "package archive manifest changed during replay: '"
          + filename_.string() + "'");
    }

    if (archive_read_close(handle.get()) != ARCHIVE_OK)
      backend_error(handle.get(), filename_, "could not close package archive");

    verify_sealed_source(
        source_.get(), stamp_, receipt_.archive_digest(), filename_);
  }

private:
  std::filesystem::path filename_;
  fd_handle source_;
  source_stamp stamp_;
  archive_inspection_receipt receipt_;
  package_image image_;
};

} // namespace

std::unique_ptr<package_archive>
libarchive_backend::open(const archive_inspection_request& request) const
{
  const std::filesystem::path& filename = request.source;
  fd_handle source = open_source(filename);
  const source_stamp before = read_source_stamp(source.get(), filename);
  const complete_archive_digest archive_digest =
      hash_source(source.get(), before, filename);
  verify_source(source.get(), before, filename);

  if (request.expected_archive_digest
      && *request.expected_archive_digest != archive_digest)
  {
    throw complete_archive_digest_mismatch_error(
        *request.expected_archive_digest, archive_digest);
  }

  package_image image = inspect_source(source.get(), before, filename);
  verify_source(source.get(), before, filename);

  const complete_archive_digest after_digest =
      hash_source(source.get(), before, filename);
  verify_source(source.get(), before, filename);
  if (archive_digest != after_digest)
  {
    throw source_changed_error(
        "package archive source changed during inspection: '"
        + filename.string() + "'");
  }

  archive_inspection_receipt receipt(
      archive_backend_identity::parse("libpkgimage.libarchive.tar.v1"),
      archive_digest, image.identity(),
      static_cast<std::uint64_t>(image.size()));

  return std::make_unique<libarchive_package>(
      filename, std::move(source), before, std::move(receipt),
      std::move(image));
}

} // namespace pkgimage
