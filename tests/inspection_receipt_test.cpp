// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include "test.h"

#include <libpkgimage/error.h>
#include <libpkgimage/inspection_receipt.h>

using pkgimage::archive_backend_identity;
using pkgimage::archive_inspection_receipt;
using pkgimage::complete_archive_digest;
using pkgimage::invalid_receipt_error;
using pkgimage::package_image_identity;

int
main()
{
  const auto archive = complete_archive_digest::parse(
      "v1:sha256:00000000000000000000000000000000"
      "00000000000000000000000000000000");
  const auto image = package_image_identity::parse(
      "v1:sha256:11111111111111111111111111111111"
      "11111111111111111111111111111111");
  const archive_inspection_receipt receipt(
      archive_backend_identity::parse("backend.v1"), archive, image, 2);
  const archive_inspection_receipt same(
      archive_backend_identity::parse("backend.v1"), archive, image, 2);
  CHECK(receipt.identity() == same.identity());
  CHECK(receipt.identity().string()
        == "v1:sha256:5f1c92a5c4ee66026bbdf95c1f8c76ff"
           "b0810acc8a8c4ad91cbbf3df63e46453");

  const archive_inspection_receipt other_backend(
      archive_backend_identity::parse("backend.v2"), archive, image, 2);
  CHECK(receipt.identity() != other_backend.identity());

  const archive_inspection_receipt other_count(
      archive_backend_identity::parse("backend.v1"), archive, image, 3);
  CHECK(receipt.identity() != other_count.identity());

  const auto other_image = package_image_identity::parse(
      "v1:sha256:22222222222222222222222222222222"
      "22222222222222222222222222222222");
  const archive_inspection_receipt changed_image(
      archive_backend_identity::parse("backend.v1"), archive, other_image, 2);
  CHECK(receipt.identity() != changed_image.identity());

  check_throws<invalid_receipt_error>([&] {
    archive_inspection_receipt invalid(
        archive_backend_identity::parse("backend.v1"), archive, image, 0);
  });
  return EXIT_SUCCESS;
}
