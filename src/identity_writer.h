// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "sha256.h"

#include <cstdint>
#include <string_view>

#include <libpkgimage/digest.h>

namespace pkgimage::detail {

class identity_writer final {
public:
  void u8(std::uint8_t value);
  void u16(std::uint16_t value);
  void u32(std::uint32_t value);
  void u64(std::uint64_t value);
  void i64(std::int64_t value);
  void bytes(const void* data, std::size_t size);
  void length_prefixed(std::string_view value);
  void digest(std::uint16_t representation_version,
              digest_algorithm algorithm,
              const digest_bytes& value);
  [[nodiscard]] sha256_digest_bytes finish();

private:
  sha256_context hash_;
};

} // namespace pkgimage::detail
