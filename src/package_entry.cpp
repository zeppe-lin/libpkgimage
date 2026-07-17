/*
 * Copyright (C) 2026 Alexandr Savca
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <libpkgimage/package_entry.h>

#include <utility>

namespace pkgimage {

package_entry::package_entry(pkgimage::package_path package_path,
                             entry_type entry_type)
    : path(std::move(package_path)), type(entry_type)
{
}

} // namespace pkgimage
