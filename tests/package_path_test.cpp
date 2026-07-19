// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include "test.h"

#include <libpkgimage/error.h>
#include <libpkgimage/package_path.h>

#include <string>

using pkgimage::package_path;
using pkgimage::path_error;

int
main()
{
  const package_path normalized =
      package_path::parse("./usr//bin/./pkgadd/");
  CHECK(normalized.string() == "usr/bin/pkgadd");
  CHECK(normalized.filename() == "pkgadd");
  CHECK(normalized.parent().has_value());
  CHECK(normalized.parent()->string() == "usr/bin");

  const package_path top = package_path::parse("etc");
  CHECK(!top.parent().has_value());
  CHECK(top.filename() == "etc");

  const package_path etc = package_path::parse("etc");
  const package_path config = package_path::parse("etc/pkg/pkgadd.conf");
  const package_path similar = package_path::parse("etcetera/file");
  CHECK(etc.is_ancestor_of(config));
  CHECK(!etc.is_ancestor_of(etc));
  CHECK(!etc.is_ancestor_of(similar));

  check_throws<path_error>([] { (void)package_path::parse(""); });
  check_throws<path_error>([] { (void)package_path::parse("/"); });
  check_throws<path_error>([] { (void)package_path::parse("/etc/passwd"); });
  check_throws<path_error>([] { (void)package_path::parse("."); });
  check_throws<path_error>([] { (void)package_path::parse("../etc/passwd"); });
  check_throws<path_error>([] { (void)package_path::parse("usr/../../etc/passwd"); });
  check_throws<path_error>([] { (void)package_path::parse("usr\nlib/lib.so"); });

  const std::string with_nul("usr\0bin", 7);
  check_throws<path_error>([&with_nul] { (void)package_path::parse(with_nul); });

  return EXIT_SUCCESS;
}
