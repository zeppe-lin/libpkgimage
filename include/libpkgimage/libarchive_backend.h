// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file libarchive_backend.h
 * \brief libarchive implementation of package archive access.
 */

#pragma once

#include <libpkgimage/archive_backend.h>

namespace pkgimage {

/*! \brief Tar archive backend implemented with libarchive. */
class libarchive_backend final : public archive_backend {
public:
  using archive_backend::open;

  [[nodiscard]] std::unique_ptr<package_archive>
  open(const archive_inspection_request& request) const override;
};

} // namespace pkgimage
