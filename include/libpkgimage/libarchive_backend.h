/*
 * Copyright (C) 2026 Alexandr Savca
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*!
 * \file libarchive_backend.h
 * \brief libarchive implementation of package archive inspection.
 * \copyright See COPYING for license terms and COPYRIGHT for notices.
 */

#pragma once

#include <libpkgimage/archive_backend.h>

namespace pkgimage {

/*!
 * \brief archive_backend implemented with libarchive.
 *
 * This backend accepts tar archives with the compression filters enabled by
 * the linked libarchive build.  It contains no installation or package
 * policy and exposes no libarchive type through the public API.
 */
class libarchive_backend final : public archive_backend {
public:
  /*!
   * \copydoc archive_backend::inspect
   */
  [[nodiscard]] package_image
  inspect(const std::filesystem::path& filename) const override;
};

} // namespace pkgimage
