/*
 * Copyright (C) 2026 Alexandr Savca
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <libpkgimage/package_entry.h>

#include <utility>

namespace pkgimage {

package_entry::package_entry(pkgimage::package_path package_path,
                             entry_type entry_type)
    : path(std::move(package_path)),
      type(entry_type)
{
}

bool
operator==(const device_number& lhs, const device_number& rhs) noexcept
{
  return lhs.major == rhs.major && lhs.minor == rhs.minor;
}

bool
operator!=(const device_number& lhs, const device_number& rhs) noexcept
{
  return !(lhs == rhs);
}

bool
operator==(const package_entry& lhs, const package_entry& rhs) noexcept
{
  return lhs.id == rhs.id
      && lhs.path == rhs.path
      && lhs.type == rhs.type
      && lhs.mode == rhs.mode
      && lhs.uid == rhs.uid
      && lhs.gid == rhs.gid
      && lhs.size == rhs.size
      && lhs.mtime == rhs.mtime
      && lhs.mtime_nanoseconds == rhs.mtime_nanoseconds
      && lhs.symlink_target == rhs.symlink_target
      && lhs.hardlink_target == rhs.hardlink_target
      && lhs.device == rhs.device;
}

bool
operator!=(const package_entry& lhs, const package_entry& rhs) noexcept
{
  return !(lhs == rhs);
}

} // namespace pkgimage
