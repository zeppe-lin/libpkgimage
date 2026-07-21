// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include <libpkgimage/archive_backend.h>

namespace pkgimage {

std::unique_ptr<package_archive>
archive_backend::open(const std::filesystem::path& filename) const
{
  return open(archive_inspection_request {filename, std::nullopt});
}

inspected_package_image
archive_backend::inspect(const archive_inspection_request& request) const
{
  const std::unique_ptr<package_archive> package = open(request);
  return inspected_package_image(
      package->image(), package->inspection_receipt());
}

inspected_package_image
archive_backend::inspect(const std::filesystem::path& filename) const
{
  return inspect(archive_inspection_request {filename, std::nullopt});
}

} // namespace pkgimage
