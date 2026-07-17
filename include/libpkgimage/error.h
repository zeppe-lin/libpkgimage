/*
 * Copyright (C) 2026 Alexandr Savca
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*!
 * \file error.h
 * \brief Typed errors reported by libpkgimage.
 * \copyright See COPYING for license terms and COPYRIGHT for notices.
 */

#pragma once

#include <stdexcept>
#include <string>

namespace pkgimage {

/*!
 * \brief Base class for all libpkgimage failures.
 *
 * Callers may catch this type when the distinction between path,
 * manifest, selection, and archive failures is not relevant.
 */
class error : public std::runtime_error {
public:
  /*!
   * \brief Construct an error with a diagnostic message.
   * \param message Human-readable failure description.
   */
  explicit error(const std::string& message);
};

/*!
 * \brief Reports a package path that cannot be normalized safely.
 */
class path_error final : public error {
public:
  using error::error;
};

/*!
 * \brief Reports an invalid package entry or image invariant.
 */
class manifest_error final : public error {
public:
  using error::error;
};

/*!
 * \brief Reports an invalid payload entry selection.
 */
class selection_error final : public error {
public:
  using error::error;
};

/*!
 * \brief Reports an archive decoding, format, or I/O failure.
 */
class archive_error final : public error {
public:
  using error::error;
};

} // namespace pkgimage
