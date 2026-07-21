// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include "identity_writer.h"

#include <libpkgimage/error.h>

#include <array>
#include <limits>

namespace pkgimage::detail {

void
identity_writer::u8(std::uint8_t value)
{
  hash_.update(&value, sizeof(value));
}

void
identity_writer::u16(std::uint16_t value)
{
  const std::array<std::uint8_t, 2> bytes {
    static_cast<std::uint8_t>((value >> 8) & 0xff),
    static_cast<std::uint8_t>(value & 0xff),
  };
  hash_.update(bytes.data(), bytes.size());
}

void
identity_writer::u32(std::uint32_t value)
{
  const std::array<std::uint8_t, 4> bytes {
    static_cast<std::uint8_t>((value >> 24) & 0xff),
    static_cast<std::uint8_t>((value >> 16) & 0xff),
    static_cast<std::uint8_t>((value >> 8) & 0xff),
    static_cast<std::uint8_t>(value & 0xff),
  };
  hash_.update(bytes.data(), bytes.size());
}

void
identity_writer::u64(std::uint64_t value)
{
  const std::array<std::uint8_t, 8> bytes {
    static_cast<std::uint8_t>((value >> 56) & 0xff),
    static_cast<std::uint8_t>((value >> 48) & 0xff),
    static_cast<std::uint8_t>((value >> 40) & 0xff),
    static_cast<std::uint8_t>((value >> 32) & 0xff),
    static_cast<std::uint8_t>((value >> 24) & 0xff),
    static_cast<std::uint8_t>((value >> 16) & 0xff),
    static_cast<std::uint8_t>((value >> 8) & 0xff),
    static_cast<std::uint8_t>(value & 0xff),
  };
  hash_.update(bytes.data(), bytes.size());
}

void
identity_writer::i64(std::int64_t value)
{
  u64(static_cast<std::uint64_t>(value));
}

void
identity_writer::bytes(const void* data, std::size_t size)
{
  hash_.update(data, size);
}

void
identity_writer::length_prefixed(std::string_view value)
{
  if constexpr (sizeof(std::size_t) > sizeof(std::uint64_t))
  {
    if (value.size() > std::numeric_limits<std::uint64_t>::max())
      throw digest_error("identity field exceeds the v1 length limit");
  }
  u64(static_cast<std::uint64_t>(value.size()));
  bytes(value.data(), value.size());
}

void
identity_writer::digest(std::uint16_t representation_version,
                        digest_algorithm algorithm,
                        const digest_bytes& value)
{
  if (value.size() > std::numeric_limits<std::uint16_t>::max())
    throw digest_error("digest result exceeds the v1 length limit");
  u16(representation_version);
  u16(static_cast<std::uint16_t>(algorithm));
  u16(static_cast<std::uint16_t>(value.size()));
  bytes(value.data(), value.size());
}

sha256_digest_bytes
identity_writer::finish()
{
  return hash_.finish();
}

} // namespace pkgimage::detail
