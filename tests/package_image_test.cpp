/*
 * Copyright (C) 2026 Alexandr Savca
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "test.h"

#include <libpkgimage/error.h>
#include <libpkgimage/package_image.h>

#include <utility>
#include <vector>

using pkgimage::entry_type;
using pkgimage::manifest_error;
using pkgimage::package_entry;
using pkgimage::package_image;
using pkgimage::package_path;

namespace {

package_entry
entry(const char* path, entry_type type)
{
  package_entry result(package_path::parse(path), type);
  result.mode = 0644;
  return result;
}

} // namespace

int
main()
{
  package_entry empty_directory =
      entry("var/lib/example-empty", entry_type::directory);
  empty_directory.mode = 0755;

  package_entry regular =
      entry("etc/example/file.conf", entry_type::regular);
  regular.size = 18;

  package_entry hardlink =
      entry("etc/example/file.link", entry_type::hardlink);
  hardlink.hardlink_target = package_path::parse("etc/example/file.conf");

  package_image image({empty_directory, regular, hardlink});
  CHECK(image.size() == 3);
  CHECK(image.entries()[0].type == entry_type::directory);
  CHECK(image.find(package_path::parse("var/lib/example-empty")) != nullptr);
  CHECK(image.find(package_path::parse("var/lib")) == nullptr);
  CHECK(image.find(package_path::parse("missing")) == nullptr);

  check_throws<manifest_error>([] {
    package_image empty(std::vector<package_entry> {});
  });

  check_throws<manifest_error>([] {
    package_image duplicate({
      entry("./usr/bin/tool", entry_type::regular),
      entry("usr//bin/tool", entry_type::regular),
    });
  });

  check_throws<manifest_error>([] {
    package_entry link = entry("usr/bin/tool", entry_type::symlink);
    package_image missing_target({std::move(link)});
  });

  check_throws<manifest_error>([] {
    package_entry link = entry("usr/bin/tool", entry_type::hardlink);
    link.hardlink_target = package_path::parse("usr/libexec/tool");
    package_image absent_target({std::move(link)});
  });

  check_throws<manifest_error>([] {
    package_entry directory =
        entry("usr/share/data", entry_type::directory);
    package_entry link =
        entry("usr/share/data.link", entry_type::hardlink);
    link.hardlink_target = directory.path;
    package_image wrong_target({std::move(directory), std::move(link)});
  });

  check_throws<manifest_error>([] {
    package_entry device =
        entry("dev/example", entry_type::character_device);
    package_image missing_number({std::move(device)});
  });

  check_throws<manifest_error>([] {
    package_entry file = entry("usr/share/data", entry_type::regular);
    file.device = pkgimage::device_number {1, 3};
    package_image unexpected_number({std::move(file)});
  });

  return EXIT_SUCCESS;
}
