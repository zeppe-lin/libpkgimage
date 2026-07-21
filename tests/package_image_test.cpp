// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

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
using pkgimage::regular_content_digest;

namespace {

package_entry
entry(const char* path, entry_type type)
{
  package_entry result(package_path::parse(path), type);
  if (type == entry_type::regular)
    result.regular_content = regular_content_digest::parse(
        "v1:sha256:e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
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
    package_entry file(package_path::parse("usr/share/missing"),
                       entry_type::regular);
    package_image missing_content({std::move(file)});
  });

  check_throws<manifest_error>([] {
    package_entry directory = entry("usr/share/data", entry_type::directory);
    directory.regular_content = regular_content_digest::parse(
        "v1:sha256:e3b0c44298fc1c149afbf4c8996fb924"
        "27ae41e4649b934ca495991b7852b855");
    package_image unexpected_content({std::move(directory)});
  });

  check_throws<manifest_error>([] {
    package_entry file = entry("usr/share/data", entry_type::regular);
    package_entry link = entry("usr/share/data.link", entry_type::hardlink);
    link.hardlink_target = file.path;
    link.regular_content = regular_content_digest::parse(
        "v1:sha256:e3b0c44298fc1c149afbf4c8996fb924"
        "27ae41e4649b934ca495991b7852b855");
    package_image hardlink_content({std::move(file), std::move(link)});
  });

  check_throws<manifest_error>([] {
    package_entry invalid = entry("usr/share/invalid", entry_type::fifo);
    invalid.type = static_cast<entry_type>(255);
    package_image unsupported_type({std::move(invalid)});
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
