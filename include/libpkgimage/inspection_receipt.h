// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file inspection_receipt.h
 * \brief Evidence binding exact archive bytes to normalized archive truth.
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include <libpkgimage/digest.h>
#include <libpkgimage/package_image.h>

namespace pkgimage {

/*! \brief Stable semantic identity of an archive inspection backend. */
class archive_backend_identity final {
public:
  /*!
   * \brief Validate and construct a semantic backend identity.
   * \param value Stable backend-schema identifier.
   */
  [[nodiscard]] static archive_backend_identity parse(std::string_view value);
  [[nodiscard]] const std::string& string() const noexcept;

  friend bool operator==(const archive_backend_identity& lhs,
                         const archive_backend_identity& rhs) noexcept;
  friend bool operator!=(const archive_backend_identity& lhs,
                         const archive_backend_identity& rhs) noexcept;

private:
  explicit archive_backend_identity(std::string value);
  std::string value_;
};

/*! \brief Immutable evidence returned after successful archive inspection. */
class archive_inspection_receipt final {
public:
  /*!
   * \brief Construct structurally valid immutable inspection evidence.
   * \param backend Semantic inspection backend identity.
   * \param archive_digest Digest of the exact retained source bytes.
   * \param image_identity Identity of the normalized output image.
   * \param entry_count Number of explicit entries in that image.
   */
  archive_inspection_receipt(
      archive_backend_identity backend,
      complete_archive_digest archive_digest,
      package_image_identity image_identity,
      std::uint64_t entry_count);

  [[nodiscard]] std::uint32_t schema_version() const noexcept;
  [[nodiscard]] const archive_backend_identity&
  backend_identity() const noexcept;
  [[nodiscard]] const complete_archive_digest&
  archive_digest() const noexcept;
  [[nodiscard]] const package_image_identity&
  image_identity() const noexcept;
  [[nodiscard]] std::uint64_t entry_count() const noexcept;
  [[nodiscard]] const archive_inspection_receipt_identity&
  identity() const noexcept;

private:
  std::uint32_t schema_version_ = 1;
  archive_backend_identity backend_identity_;
  complete_archive_digest archive_digest_;
  package_image_identity image_identity_;
  std::uint64_t entry_count_;
  archive_inspection_receipt_identity identity_;
};

/*! \brief Inspection result that cannot discard its archive binding. */
class inspected_package_image final {
public:
  /*!
   * \brief Bind a package image to matching inspection evidence.
   * \param image Immutable normalized archive image.
   * \param receipt Receipt identifying that exact image.
   */
  inspected_package_image(package_image image,
                          archive_inspection_receipt receipt);

  [[nodiscard]] const package_image& image() const noexcept;
  [[nodiscard]] const archive_inspection_receipt& receipt() const noexcept;

private:
  package_image image_;
  archive_inspection_receipt receipt_;
};

} // namespace pkgimage
