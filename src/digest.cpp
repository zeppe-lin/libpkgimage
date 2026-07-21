// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include <libpkgimage/digest.h>
#include <libpkgimage/error.h>

#include <utility>

namespace pkgimage {
namespace {

int
hex_value(char ch)
{
  if (ch >= '0' && ch <= '9')
    return ch - '0';
  if (ch >= 'a' && ch <= 'f')
    return ch - 'a' + 10;
  return -1;
}

struct parsed_digest final {
  std::uint16_t representation_version;
  digest_algorithm algorithm;
  digest_bytes bytes;
};

parsed_digest
parse_digest(std::string_view input)
{
  const std::size_t version_separator = input.find(':');
  if (version_separator == std::string_view::npos)
    throw invalid_digest_error("digest representation has no version tag");

  const std::string_view version = input.substr(0, version_separator);
  if (version != "v1")
  {
    throw invalid_digest_error(
        "unsupported digest representation version '" + std::string(version)
        + "'");
  }

  input.remove_prefix(version_separator + 1);
  const std::size_t algorithm_separator = input.find(':');
  if (algorithm_separator == std::string_view::npos)
    throw invalid_digest_error("digest representation has no algorithm tag");

  const std::string_view algorithm_name =
      input.substr(0, algorithm_separator);
  if (algorithm_name != "sha256")
  {
    throw unsupported_digest_algorithm_error(
        "unsupported digest algorithm '" + std::string(algorithm_name) + "'");
  }

  const std::string_view hex = input.substr(algorithm_separator + 1);
  if (hex.size() != sha256_digest_size * 2)
    throw invalid_digest_error("invalid SHA-256 digest length");

  digest_bytes bytes(sha256_digest_size);
  for (std::size_t i = 0; i < bytes.size(); ++i)
  {
    const int high = hex_value(hex[i * 2]);
    const int low = hex_value(hex[i * 2 + 1]);
    if (high < 0 || low < 0)
      throw invalid_digest_error("invalid lowercase hexadecimal digest byte");
    bytes[i] = static_cast<std::uint8_t>((high << 4) | low);
  }

  return parsed_digest {
    digest_representation_version,
    digest_algorithm::sha256,
    std::move(bytes),
  };
}

std::string
format_digest(std::uint16_t representation_version,
              digest_algorithm algorithm,
              const digest_bytes& bytes)
{
  if (representation_version != digest_representation_version
      || algorithm != digest_algorithm::sha256
      || bytes.size() != sha256_digest_size)
  {
    throw digest_error("cannot serialize unsupported digest representation");
  }

  static constexpr char hex[] = "0123456789abcdef";
  std::string result("v1:sha256:");
  result.reserve(result.size() + bytes.size() * 2);
  for (const std::uint8_t byte : bytes)
  {
    result.push_back(hex[(byte >> 4) & 0x0f]);
    result.push_back(hex[byte & 0x0f]);
  }
  return result;
}

} // namespace

#define PKGIMAGE_DEFINE_TYPED_DIGEST(type_name)                               \
  type_name::type_name(std::uint16_t representation_version,                  \
                       digest_algorithm algorithm,                            \
                       digest_bytes bytes)                                    \
      : representation_version_(representation_version),                      \
        algorithm_(algorithm),                                                \
        bytes_(std::move(bytes))                                              \
  {                                                                           \
  }                                                                           \
                                                                               \
  type_name                                                                   \
  type_name::from_sha256(sha256_digest_bytes bytes)                           \
  {                                                                           \
    return type_name(                                                         \
        digest_representation_version, digest_algorithm::sha256,              \
        digest_bytes(bytes.begin(), bytes.end()));                            \
  }                                                                           \
                                                                               \
  type_name                                                                   \
  type_name::parse(std::string_view input)                                    \
  {                                                                           \
    parsed_digest parsed = parse_digest(input);                               \
    return type_name(parsed.representation_version, parsed.algorithm,         \
                     std::move(parsed.bytes));                                \
  }                                                                           \
                                                                               \
  std::uint16_t                                                               \
  type_name::representation_version() const noexcept                          \
  {                                                                           \
    return representation_version_;                                           \
  }                                                                           \
                                                                               \
  digest_algorithm                                                            \
  type_name::algorithm() const noexcept                                       \
  {                                                                           \
    return algorithm_;                                                        \
  }                                                                           \
                                                                               \
  const digest_bytes&                                                         \
  type_name::bytes() const noexcept                                           \
  {                                                                           \
    return bytes_;                                                            \
  }                                                                           \
                                                                               \
  std::string                                                                 \
  type_name::string() const                                                   \
  {                                                                           \
    return format_digest(representation_version_, algorithm_, bytes_);        \
  }                                                                           \
                                                                               \
  bool                                                                        \
  operator==(const type_name& lhs, const type_name& rhs) noexcept            \
  {                                                                           \
    return lhs.representation_version_ == rhs.representation_version_         \
        && lhs.algorithm_ == rhs.algorithm_                                   \
        && lhs.bytes_ == rhs.bytes_;                                          \
  }                                                                           \
                                                                               \
  bool                                                                        \
  operator!=(const type_name& lhs, const type_name& rhs) noexcept            \
  {                                                                           \
    return !(lhs == rhs);                                                     \
  }

PKGIMAGE_DEFINE_TYPED_DIGEST(complete_archive_digest)
PKGIMAGE_DEFINE_TYPED_DIGEST(regular_content_digest)
PKGIMAGE_DEFINE_TYPED_DIGEST(package_image_identity)
PKGIMAGE_DEFINE_TYPED_DIGEST(archive_inspection_receipt_identity)

#undef PKGIMAGE_DEFINE_TYPED_DIGEST

} // namespace pkgimage
