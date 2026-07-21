// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file error.h
 * \brief Typed errors reported by libpkgimage.
 */

#pragma once

#include <stdexcept>
#include <string>

#include <libpkgimage/digest.h>

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
 * \brief Base class for digest representation and computation failures.
 */
class digest_error : public error {
public:
  using error::error;
};

/*! \brief Reports malformed caller-supplied digest syntax. */
class invalid_digest_error final : public digest_error {
public:
  using digest_error::digest_error;
};

/*! \brief Reports a digest algorithm not supported by this build. */
class unsupported_digest_algorithm_error final : public digest_error {
public:
  using digest_error::digest_error;
};

/*! \brief Reports an invalid archive-inspection receipt invariant. */
class invalid_receipt_error final : public error {
public:
  using error::error;
};

/*! \brief Reports a package path that cannot be normalized safely. */
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
 * \brief Reports an archive decoding, format, source, or I/O failure.
 */
class archive_error : public error {
public:
  using error::error;
};

/*! \brief Reports decoded payload length inconsistent with its archive header. */
class declared_size_mismatch_error final : public archive_error {
public:
  using archive_error::archive_error;
};

/*! \brief Reports an expected exact archive digest that did not match. */
class complete_archive_digest_mismatch_error final : public archive_error {
public:
  complete_archive_digest_mismatch_error(
      complete_archive_digest expected,
      complete_archive_digest actual);

  [[nodiscard]] const complete_archive_digest& expected() const noexcept;
  [[nodiscard]] const complete_archive_digest& actual() const noexcept;

private:
  complete_archive_digest expected_;
  complete_archive_digest actual_;
};

/*! \brief Reports that an opened archive source changed after inspection. */
class source_changed_error final : public archive_error {
public:
  using archive_error::archive_error;
};

} // namespace pkgimage
