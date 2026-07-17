/*
 * Copyright (C) 2026 Alexandr Savca
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*!
 * \file libarchive_backend.h
 * \brief libarchive implementation of package archive access.
 * \copyright See COPYING for license terms and COPYRIGHT for notices.
 */

#pragma once

#include <libpkgimage/archive_backend.h>

namespace pkgimage {

/*!
 * \brief archive_backend implemented with libarchive.
 *
 * This backend accepts regular tar archive files with the compression filters
 * enabled by the linked libarchive build.  It retains an open source
 * descriptor, detects in-place source changes, contains no installation
 * policy, and exposes no libarchive type through the public API.
 */
class libarchive_backend final : public archive_backend {
public:
  /*!
   * \copydoc archive_backend::open
   */
  [[nodiscard]] std::unique_ptr<package_archive>
  open(const std::filesystem::path& filename) const override;
};

} // namespace pkgimage
