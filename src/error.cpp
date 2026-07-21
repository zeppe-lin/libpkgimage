// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include <libpkgimage/error.h>

#include <utility>

namespace pkgimage {

error::error(const std::string& message)
    : std::runtime_error(message)
{
}


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


regular_payload_digest_mismatch_error::
regular_payload_digest_mismatch_error(
    regular_content_digest expected,
    regular_content_digest actual,
    std::string path)
    : archive_error(
          "regular payload digest mismatch for '" + path + "': expected "
          + expected.string() + ", observed " + actual.string()),
      expected_(std::move(expected)),
      actual_(std::move(actual)),
      path_(std::move(path))
{
}

const regular_content_digest&
regular_payload_digest_mismatch_error::expected() const noexcept
{
  return expected_;
}

const regular_content_digest&
regular_payload_digest_mismatch_error::actual() const noexcept
{
  return actual_;
}

const std::string&
regular_payload_digest_mismatch_error::path() const noexcept
{
  return path_;
}

} // namespace pkgimage
