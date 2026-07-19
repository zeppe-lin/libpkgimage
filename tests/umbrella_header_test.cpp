// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include "test.h"

#include <libpkgimage/libpkgimage.h>

int
main()
{
  const pkgimage::package_path path =
      pkgimage::package_path::parse("usr/bin/tool");
  pkgimage::package_entry entry(path, pkgimage::entry_type::regular);
  pkgimage::package_image image({entry});
  CHECK(image.size() == 1);
  return EXIT_SUCCESS;
}
