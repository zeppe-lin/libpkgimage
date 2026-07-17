/*
 * Copyright (C) 2026 Alexandr Savca
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*!
 * \file package_entry.h
 * \brief Backend-neutral package entry metadata.
 * \copyright See COPYING for license terms and COPYRIGHT for notices.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>

#include <libpkgimage/package_path.h>

namespace pkgimage {

/*!
 * \brief Stable identifier of an entry inside one package image.
 *
 * Identifiers are assigned consecutively in archive order by package_image.
 */
using entry_id = std::size_t;

/*!
 * \brief Sentinel used before an entry is admitted into a package image.
 */
inline constexpr entry_id invalid_entry_id =
    std::numeric_limits<entry_id>::max();

/*!
 * \brief Semantic type of a package archive entry.
 */
enum class entry_type {
  regular,          //!< Regular file with payload data.
  directory,        //!< Explicit directory archive entry.
  symlink,          //!< Symbolic link with an uninterpreted target string.
  hardlink,         //!< Hard link to another regular package entry.
  fifo,             //!< Named pipe.
  character_device, //!< Character device node.
  block_device,     //!< Block device node.
};

/*!
 * \brief Device identifier carried by a character or block device entry.
 */
struct device_number final {
  std::uint64_t major = 0; //!< Device class identifier.
  std::uint64_t minor = 0; //!< Device instance identifier.
};

/*!
 * \brief Compare device identifiers for equality.
 */
[[nodiscard]] bool operator==(const device_number& lhs,
                              const device_number& rhs) noexcept;

/*!
 * \brief Compare device identifiers for inequality.
 */
[[nodiscard]] bool operator!=(const device_number& lhs,
                              const device_number& rhs) noexcept;

/*!
 * \brief Metadata normalized from one package archive entry.
 *
 * package_entry contains no backend-specific handle.  It is the stable
 * semantic object produced by archive backends and consumed by callers.
 */
struct package_entry final {
  /*!
   * \brief Construct an entry with its mandatory semantic fields.
   * \param package_path Canonical path inside the package root.
   * \param type Object type recorded by the archive.
   */
  package_entry(pkgimage::package_path package_path, entry_type type);

  entry_id id = invalid_entry_id;            //!< Stable image identifier.
  pkgimage::package_path path;                //!< Canonical root-relative path.
  entry_type type;                            //!< Semantic object type.
  std::uint32_t mode = 0;                     //!< POSIX permission and special bits.
  std::uint64_t uid = 0;                      //!< Numeric owner identifier.
  std::uint64_t gid = 0;                      //!< Numeric group identifier.
  std::uint64_t size = 0;                     //!< Regular-file payload size.
  std::int64_t mtime = 0;                     //!< Modification time in Unix seconds.
  std::uint32_t mtime_nanoseconds = 0;        //!< Subsecond modification time.
  std::optional<std::string> symlink_target;  //!< Raw symbolic-link target.
  std::optional<pkgimage::package_path> hardlink_target; //!< Canonical target.
  std::optional<device_number> device;        //!< Device-node identifier.
};

/*!
 * \brief Compare normalized package entries for semantic equality.
 */
[[nodiscard]] bool operator==(const package_entry& lhs,
                              const package_entry& rhs) noexcept;

/*!
 * \brief Compare normalized package entries for semantic inequality.
 */
[[nodiscard]] bool operator!=(const package_entry& lhs,
                              const package_entry& rhs) noexcept;

} // namespace pkgimage
