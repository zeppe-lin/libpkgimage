// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include <libpkgimage/error.h>
#include <libpkgimage/inspection_receipt.h>

#include "identity_writer.h"

#include <utility>

namespace pkgimage {
namespace {

archive_inspection_receipt_identity
identify_receipt(std::uint32_t schema_version,
                 const archive_backend_identity& backend,
                 const complete_archive_digest& archive_digest,
                 const package_image_identity& image_identity,
                 std::uint64_t entry_count)
{
  detail::identity_writer writer;
  writer.length_prefixed("libpkgimage.archive-inspection-receipt.v1");
  writer.u32(schema_version);
  writer.length_prefixed(backend.string());
  writer.digest(archive_digest.representation_version(),
                archive_digest.algorithm(), archive_digest.bytes());
  writer.digest(image_identity.representation_version(),
                image_identity.algorithm(), image_identity.bytes());
  writer.u64(entry_count);
  return archive_inspection_receipt_identity::from_sha256(writer.finish());
}

} // namespace

archive_backend_identity::archive_backend_identity(std::string value)
    : value_(std::move(value))
{
}

archive_backend_identity
archive_backend_identity::parse(std::string_view value)
{
  if (value.empty())
    throw invalid_receipt_error("archive backend identity is empty");
  if (value.find('\0') != std::string_view::npos
      || value.find('\n') != std::string_view::npos
      || value.find('\r') != std::string_view::npos)
  {
    throw invalid_receipt_error("archive backend identity is not line-safe");
  }
  return archive_backend_identity(std::string(value));
}

const std::string&
archive_backend_identity::string() const noexcept
{
  return value_;
}

bool
operator==(const archive_backend_identity& lhs,
           const archive_backend_identity& rhs) noexcept
{
  return lhs.value_ == rhs.value_;
}

bool
operator!=(const archive_backend_identity& lhs,
           const archive_backend_identity& rhs) noexcept
{
  return !(lhs == rhs);
}

archive_inspection_receipt::archive_inspection_receipt(
    archive_backend_identity backend,
    complete_archive_digest archive_digest,
    package_image_identity image_identity,
    std::uint64_t entry_count)
    : backend_identity_(std::move(backend)),
      archive_digest_(std::move(archive_digest)),
      image_identity_(std::move(image_identity)),
      entry_count_(entry_count),
      identity_(identify_receipt(
          schema_version_, backend_identity_, archive_digest_,
          image_identity_, entry_count_))
{
  if (entry_count_ == 0)
    throw invalid_receipt_error("archive inspection receipt has no entries");
}

std::uint32_t
archive_inspection_receipt::schema_version() const noexcept
{
  return schema_version_;
}

const archive_backend_identity&
archive_inspection_receipt::backend_identity() const noexcept
{
  return backend_identity_;
}

const complete_archive_digest&
archive_inspection_receipt::archive_digest() const noexcept
{
  return archive_digest_;
}

const package_image_identity&
archive_inspection_receipt::image_identity() const noexcept
{
  return image_identity_;
}

std::uint64_t
archive_inspection_receipt::entry_count() const noexcept
{
  return entry_count_;
}

const archive_inspection_receipt_identity&
archive_inspection_receipt::identity() const noexcept
{
  return identity_;
}

inspected_package_image::inspected_package_image(
    package_image image,
    archive_inspection_receipt receipt)
    : image_(std::move(image)),
      receipt_(std::move(receipt))
{
  if (receipt_.image_identity() != image_.identity())
    throw invalid_receipt_error("inspection receipt identifies another image");
  if (receipt_.entry_count() != image_.size())
    throw invalid_receipt_error("inspection receipt entry count is inconsistent");
}

const package_image&
inspected_package_image::image() const noexcept
{
  return image_;
}

const archive_inspection_receipt&
inspected_package_image::receipt() const noexcept
{
  return receipt_;
}

} // namespace pkgimage
