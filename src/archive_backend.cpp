/*
 * Copyright (C) 2026 Alexandr Savca
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <libpkgimage/archive_backend.h>

namespace pkgimage {

package_image
archive_backend::inspect(const std::filesystem::path& filename) const
{
  const std::unique_ptr<package_archive> package = open(filename);
  return package->image();
}

} // namespace pkgimage
