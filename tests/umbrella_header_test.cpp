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
  entry.regular_content = pkgimage::regular_content_digest::parse(
      "v1:sha256:e3b0c44298fc1c149afbf4c8996fb924"
      "27ae41e4649b934ca495991b7852b855");
  pkgimage::package_image image({entry});
  CHECK(image.size() == 1);
  return EXIT_SUCCESS;
}
