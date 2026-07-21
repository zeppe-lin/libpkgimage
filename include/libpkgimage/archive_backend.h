// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file archive_backend.h
 * \brief Backend-neutral package archive interface.
 */

#pragma once

#include <filesystem>
#include <memory>
#include <optional>

#include <libpkgimage/inspection_receipt.h>
#include <libpkgimage/package_archive.h>

namespace pkgimage {

/*! \brief Explicit inputs to one exact-byte archive inspection. */
struct archive_inspection_request final {
  std::filesystem::path source; //!< Path opened once as the retained source.
  //! Optional assertion over the exact retained source bytes.
  std::optional<complete_archive_digest> expected_archive_digest;
};

/*! \brief Backend-neutral package archive reader. */
class archive_backend {
public:
  virtual ~archive_backend() = default;

  /*!
   * \brief Inspect and retain a package archive for later replay.
   * \param request Exact source and optional exact-byte assertion.
   */
  [[nodiscard]] virtual std::unique_ptr<package_archive>
  open(const archive_inspection_request& request) const = 0;

  /*!
   * \brief Inspect and retain a package archive by pathname.
   * \param filename Regular archive source to open.
   */
  [[nodiscard]] std::unique_ptr<package_archive>
  open(const std::filesystem::path& filename) const;

  /*!
   * \brief Inspect an archive and return image plus receipt.
   * \param request Exact source and optional exact-byte assertion.
   */
  [[nodiscard]] inspected_package_image
  inspect(const archive_inspection_request& request) const;

  /*!
   * \brief Inspect an archive by pathname and return image plus receipt.
   * \param filename Regular archive source to inspect.
   */
  [[nodiscard]] inspected_package_image
  inspect(const std::filesystem::path& filename) const;
};

} // namespace pkgimage
