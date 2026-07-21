// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include <libpkgimage/error.h>

#include <utility>

namespace pkgimage {

error::error(const std::string& message)
    : std::runtime_error(message)
{
}

} // namespace pkgimage

namespace pkgimage {

complete_archive_digest_mismatch_error::
complete_archive_digest_mismatch_error(
    complete_archive_digest expected,
    complete_archive_digest actual)
    : archive_error(
          "complete archive digest mismatch: expected " + expected.string()
          + ", observed " + actual.string()),
      expected_(std::move(expected)),
      actual_(std::move(actual))
{
}

const complete_archive_digest&
complete_archive_digest_mismatch_error::expected() const noexcept
{
  return expected_;
}

const complete_archive_digest&
complete_archive_digest_mismatch_error::actual() const noexcept
{
  return actual_;
}

} // namespace pkgimage
