// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file package_path.h
 * \brief Canonical paths inside a package root.
 */

#pragma once

#include <iosfwd>
#include <optional>
#include <string>
#include <string_view>

namespace pkgimage {

/*!
 * \brief Canonical path of an object inside a package root.
 *
 * A package_path is always relative to the installation root.  Parsing
 * removes empty and `.` components, rejects `..`, rejects absolute paths,
 * and removes trailing separators.  Directory identity belongs to the
 * package entry type rather than to path spelling.
 *
 * The class deliberately does not use std::filesystem::path normalization.
 * Package paths are archive data governed by package-image semantics, not
 * paths interpreted through host filesystem rules.
 */
class package_path final {
public:
  /*!
   * \brief Normalize and validate a package path.
   * \param input Archive pathname to normalize.
   * \return Canonical root-relative package path.
   * \throws path_error if the input is empty, absolute, escapes through
   *         `..`, contains a NUL byte, or contains a line separator.
   */
  [[nodiscard]] static package_path parse(std::string_view input);

  /*!
   * \brief Return the canonical path spelling.
   */
  [[nodiscard]] const std::string& string() const noexcept;

  /*!
   * \brief Return the final path component.
   */
  [[nodiscard]] std::string_view filename() const noexcept;

  /*!
   * \brief Return the canonical parent path.
   * \return Empty optional for a top-level path.
   */
  [[nodiscard]] std::optional<package_path> parent() const;

  /*!
   * \brief Test whether this path is a strict ancestor of another path.
   */
  [[nodiscard]] bool is_ancestor_of(const package_path& other) const noexcept;

  /*!
   * \brief Compare two canonical paths for equality.
   */
  friend bool operator==(const package_path& lhs,
                         const package_path& rhs) noexcept;

  /*!
   * \brief Compare two canonical paths for inequality.
   */
  friend bool operator!=(const package_path& lhs,
                         const package_path& rhs) noexcept;

  /*!
   * \brief Order canonical paths lexicographically.
   */
  friend bool operator<(const package_path& lhs,
                        const package_path& rhs) noexcept;

private:
  explicit package_path(std::string normalized);

  std::string value_;
};

/*!
 * \brief Write a canonical package path to a stream.
 * \param out Destination stream.
 * \param path Path to write.
 * \return The destination stream.
 */
std::ostream& operator<<(std::ostream& out, const package_path& path);

} // namespace pkgimage
