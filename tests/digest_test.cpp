// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include "test.h"

#include <libpkgimage/digest.h>
#include <libpkgimage/error.h>

#include <type_traits>

using pkgimage::complete_archive_digest;
using pkgimage::invalid_digest_error;
using pkgimage::regular_content_digest;
using pkgimage::unsupported_digest_algorithm_error;

int
main()
{
  static_assert(!std::is_same_v<complete_archive_digest,
                                regular_content_digest>);
  static_assert(!std::is_convertible_v<complete_archive_digest,
                                       regular_content_digest>);

  const auto empty = regular_content_digest::parse(
      "v1:sha256:e3b0c44298fc1c149afbf4c8996fb924"
      "27ae41e4649b934ca495991b7852b855");
  CHECK(empty.string()
        == "v1:sha256:e3b0c44298fc1c149afbf4c8996fb924"
           "27ae41e4649b934ca495991b7852b855");
  CHECK(empty.representation_version()
        == pkgimage::digest_representation_version);
  CHECK(empty.algorithm() == pkgimage::digest_algorithm::sha256);

  check_throws<invalid_digest_error>([] {
    (void)regular_content_digest::parse("v1:sha256:00");
  });
  check_throws<invalid_digest_error>([] {
    (void)regular_content_digest::parse(
        "v2:sha256:e3b0c44298fc1c149afbf4c8996fb924"
        "27ae41e4649b934ca495991b7852b855");
  });
  check_throws<invalid_digest_error>([] {
    (void)regular_content_digest::parse(
        "v1:sha256:E3b0c44298fc1c149afbf4c8996fb924"
        "27ae41e4649b934ca495991b7852b855");
  });
  check_throws<invalid_digest_error>([] {
    (void)regular_content_digest::parse(
        "v1:sha256:z3b0c44298fc1c149afbf4c8996fb924"
        "27ae41e4649b934ca495991b7852b855");
  });
  check_throws<unsupported_digest_algorithm_error>([] {
    (void)regular_content_digest::parse(
        "v1:sha512:e3b0c44298fc1c149afbf4c8996fb924"
        "27ae41e4649b934ca495991b7852b855");
  });

  return EXIT_SUCCESS;
}
