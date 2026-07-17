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
#include <memory>

#include <libpkgimage/package_archive.h>

namespace pkgimage {

/*!
 * \brief Backend-neutral package archive reader.
 *
 * Implementations decode an archive transport and emit the same normalized
 * package_image model.  Archive libraries are therefore replaceable without
 * changing package-image or payload-replay semantics.
 */
class archive_backend {
public:
  /*!
   * \brief Destroy the archive backend.
   */
  virtual ~archive_backend() = default;

  /*!
   * \brief Open, inspect, and retain a package archive source.
   * \param filename Regular package archive file to open.
   * \return Stable package archive with normalized image and payload access.
   * \throws archive_error for source, backend, format, or I/O failures.
   * \throws path_error or manifest_error for invalid package contents.
   */
  [[nodiscard]] virtual std::unique_ptr<package_archive>
  open(const std::filesystem::path& filename) const = 0;

  /*!
   * \brief Inspect a package archive without retaining payload access.
   * \param filename Package archive to inspect.
   * \return Ordered, validated package image.
   *
   * This convenience operation opens the archive and copies its immutable
   * image before releasing the stable source handle.
   */
  [[nodiscard]] package_image
  inspect(const std::filesystem::path& filename) const;
};

} // namespace pkgimage
