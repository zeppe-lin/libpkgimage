// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include <libpkgimage/error.h>

namespace pkgimage {

error::error(const std::string& message)
    : std::runtime_error(message)
{
}

} // namespace pkgimage
