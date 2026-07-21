// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include "test.h"

#include <libpkgimage/entry_selection.h>
#include <libpkgimage/error.h>

#include <utility>
#include <vector>

using pkgimage::entry_selection;
using pkgimage::entry_type;
using pkgimage::package_entry;
using pkgimage::package_image;
using pkgimage::package_path;
using pkgimage::regular_content_digest;
using pkgimage::selection_error;

namespace {

package_entry
entry(const char* path, entry_type type)
{
  package_entry result(package_path::parse(path), type);
  if (type == entry_type::regular)
    result.regular_content = regular_content_digest::parse(
        "v1:sha256:e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
  return result;
}

} // namespace

int
main()
{
  package_image image({
    entry("usr/share", entry_type::directory),
    entry("usr/share/first", entry_type::regular),
    entry("usr/share/second", entry_type::regular),
  });

  CHECK(image.entries()[0].id == 0);
  CHECK(image.entries()[1].id == 1);
  CHECK(image.entries()[2].id == 2);

  const entry_selection all = entry_selection::all_regular(image);
  CHECK(all.size() == 2);
  CHECK(!all.contains(0));
  CHECK(all.contains(1));
  CHECK(all.contains(2));
  all.validate(image);

  const entry_selection reversed =
      entry_selection::from_ids(image, {2, 1});
  CHECK(reversed.size() == 2);
  CHECK(reversed.contains(1));
  CHECK(reversed.contains(2));

  const package_image copied = image;
  reversed.validate(copied);

  package_image different({
    entry("usr/share", entry_type::directory),
    entry("usr/share/other", entry_type::regular),
    entry("usr/share/second", entry_type::regular),
  });
  check_throws<selection_error>([&] { reversed.validate(different); });

  package_entry same_path_changed =
      entry("usr/share/first", entry_type::regular);
  same_path_changed.regular_content = regular_content_digest::parse(
      "v1:sha256:ca978112ca1bbdcafac231b39a23dc4d"
      "a786eff8147c4e72b9807785afee48bb");
  package_image changed_content({
    entry("usr/share", entry_type::directory),
    same_path_changed,
    entry("usr/share/second", entry_type::regular),
  });
  check_throws<selection_error>([&] { reversed.validate(changed_content); });

  check_throws<selection_error>(
      [&] { (void)entry_selection::from_ids(image, {1, 1}); });
  check_throws<selection_error>(
      [&] { (void)entry_selection::from_ids(image, {3}); });
  check_throws<selection_error>(
      [&] { (void)entry_selection::from_ids(image, {0}); });

  const entry_selection empty = entry_selection::from_ids(image, {});
  CHECK(empty.size() == 0);
  CHECK(!empty.contains(1));
  empty.validate(image);

  return EXIT_SUCCESS;
}
