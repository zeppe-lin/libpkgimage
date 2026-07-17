/*
 * Copyright (C) 2026 Alexandr Savca
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*!
 * \file archive_backend.h
 * \brief Backend-neutral package archive interface.
 * \copyright See COPYING for license terms and COPYRIGHT for notices.
 */

#pragma once

#include <filesystem>

#include <libpkgimage/package_image.h>

namespace pkgimage {

/*!
 * \brief Backend-neutral package archive reader.
 *
 * Implementations decode an archive transport and emit the same normalized
 * package_image model.  Archive libraries are therefore replaceable without
 * changing the public package-image semantics.
 */
class archive_backend {
public:
  /*!
   * \brief Destroy the archive backend.
   */
  virtual ~archive_backend() = default;

  /*!
   * \brief Read and normalize a package archive image.
   * \param filename Package archive to inspect.
   * \return Ordered, validated package image.
   * \throws archive_error for backend, format, or I/O failures.
   * \throws path_error or manifest_error for invalid package contents.
   */
  [[nodiscard]] virtual package_image
  inspect(const std::filesystem::path& filename) const = 0;
};

} // namespace pkgimage
