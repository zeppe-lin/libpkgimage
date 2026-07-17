/*
 * Copyright (C) 2026 Alexandr Savca
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*!
 * \file package_image.h
 * \brief Ordered and validated package images.
 * \copyright See COPYING for license terms and COPYRIGHT for notices.
 */

#pragma once

#include <cstddef>
#include <vector>

#include <libpkgimage/package_entry.h>

namespace pkgimage {

/*!
 * \brief Ordered, validated image of a package archive.
 *
 * Archive order is preserved because hardlink and payload semantics may
 * depend on it.  package_image assigns stable entry identifiers, rejects
 * duplicate canonical paths, and validates type-specific metadata.  Explicit
 * empty directories remain first-class entries; directories needed only as
 * structural parents are never synthesized into the image.
 */
class package_image final {
public:
  /*!
   * \brief Construct and validate an ordered package image.
   * \param entries Entries in archive order.  Existing identifiers are
   *        replaced with consecutive identifiers beginning at zero.
   * \throws manifest_error on duplicate paths or invalid entry metadata.
   */
  explicit package_image(std::vector<package_entry> entries);

  /*!
   * \brief Return entries in archive order.
   */
  [[nodiscard]] const std::vector<package_entry>& entries() const noexcept;

  /*!
   * \brief Return the number of explicit archive entries.
   */
  [[nodiscard]] std::size_t size() const noexcept;

  /*!
   * \brief Find an entry by stable image identifier.
   * \param id Entry identifier to search for.
   * \return Pointer to the entry, or nullptr when absent.
   */
  [[nodiscard]] const package_entry* entry(entry_id id) const noexcept;

  /*!
   * \brief Find an entry by canonical package path.
   * \param path Path to search for.
   * \return Pointer to the entry, or nullptr when absent.
   */
  [[nodiscard]] const package_entry*
  find(const package_path& path) const noexcept;

private:
  std::vector<package_entry> entries_;
};

} // namespace pkgimage
