// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include "test.h"

#include <libpkgimage/package_image.h>

#include <utility>
#include <vector>

using pkgimage::device_number;
using pkgimage::entry_type;
using pkgimage::package_entry;
using pkgimage::package_image;
using pkgimage::package_path;
using pkgimage::regular_content_digest;

namespace {

constexpr const char* empty_digest =
    "v1:sha256:e3b0c44298fc1c149afbf4c8996fb924"
    "27ae41e4649b934ca495991b7852b855";
constexpr const char* a_digest =
    "v1:sha256:ca978112ca1bbdcafac231b39a23dc4d"
    "a786eff8147c4e72b9807785afee48bb";
constexpr const char* b_digest =
    "v1:sha256:3e23e8160039594a33894f6564e1b134"
    "8bbd7a0088d42c4acb73eeaed59c009d";

package_entry
entry(const char* path, entry_type type)
{
  package_entry result(package_path::parse(path), type);
  result.mode = 0644;
  result.uid = 11;
  result.gid = 22;
  result.mtime = -33;
  result.mtime_nanoseconds = 44;
  return result;
}

package_entry
regular(const char* path, const char* digest, std::uint64_t size = 1)
{
  package_entry result = entry(path, entry_type::regular);
  result.size = size;
  result.regular_content = regular_content_digest::parse(digest);
  return result;
}

std::vector<package_entry>
base_entries()
{
  package_entry directory = entry("dir", entry_type::directory);
  directory.mode = 0755;

  package_entry symlink = entry("link", entry_type::symlink);
  symlink.mode = 0777;
  symlink.symlink_target = "a";

  package_entry hardlink = entry("hard", entry_type::hardlink);
  hardlink.hardlink_target = package_path::parse("a");

  package_entry fifo = entry("pipe", entry_type::fifo);
  fifo.mode = 0600;

  package_entry character = entry("dev/char", entry_type::character_device);
  character.device = device_number {1, 3};

  package_entry block = entry("dev/block", entry_type::block_device);
  block.device = device_number {8, 0};

  return {
    regular("a", a_digest),
    regular("b", b_digest),
    regular("empty", empty_digest, 0),
    std::move(directory),
    std::move(symlink),
    std::move(hardlink),
    std::move(fifo),
    std::move(character),
    std::move(block),
  };
}

void
check_changed(std::vector<package_entry> entries,
              const pkgimage::package_image_identity& expected)
{
  CHECK(package_image(std::move(entries)).identity() != expected);
}

} // namespace

int
main()
{
  const package_image image(base_entries());
  const package_image copy = image;
  CHECK(image.identity() == copy.identity());

  auto changed = base_entries();
  changed[3].path = package_path::parse("other-dir");
  check_changed(std::move(changed), image.identity());

  changed = base_entries();
  changed[6] = entry("pipe", entry_type::directory);
  check_changed(std::move(changed), image.identity());

  changed = base_entries();
  changed[0].mode = 0600;
  check_changed(std::move(changed), image.identity());

  changed = base_entries();
  changed[0].uid = 12;
  check_changed(std::move(changed), image.identity());

  changed = base_entries();
  changed[0].gid = 23;
  check_changed(std::move(changed), image.identity());

  changed = base_entries();
  changed[0].size = 2;
  check_changed(std::move(changed), image.identity());

  changed = base_entries();
  changed[0].mtime = -34;
  check_changed(std::move(changed), image.identity());

  changed = base_entries();
  changed[0].mtime_nanoseconds = 45;
  check_changed(std::move(changed), image.identity());

  changed = base_entries();
  changed[4].symlink_target = "b";
  check_changed(std::move(changed), image.identity());

  changed = base_entries();
  changed[5].hardlink_target = package_path::parse("b");
  check_changed(std::move(changed), image.identity());

  changed = base_entries();
  changed[7].device = device_number {1, 5};
  check_changed(std::move(changed), image.identity());

  changed = base_entries();
  changed[0].regular_content = regular_content_digest::parse(b_digest);
  check_changed(std::move(changed), image.identity());

  changed = base_entries();
  std::swap(changed[0], changed[1]);
  check_changed(std::move(changed), image.identity());

  CHECK(image.identity().string().rfind("v1:sha256:", 0) == 0);

  /* Filled from the canonical v1 record after the schema is frozen. */
  constexpr const char* golden =
      "v1:sha256:941d61ab28770097e015b2dd1ae62ce13"
      "139586708f1828f99a652dc60e7af5a";
  CHECK(image.identity().string() == golden);
  return EXIT_SUCCESS;
}
