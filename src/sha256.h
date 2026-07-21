// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>

#include <libpkgimage/digest.h>

#include <openssl/evp.h>

namespace pkgimage::detail {

class sha256_context final {
public:
  sha256_context();
  ~sha256_context();

  sha256_context(const sha256_context&) = delete;
  sha256_context& operator=(const sha256_context&) = delete;

  void update(const void* data, std::size_t size);
  [[nodiscard]] sha256_digest_bytes finish();

private:
  EVP_MD_CTX* context_;
  bool finished_ = false;
};

[[nodiscard]] sha256_digest_bytes
sha256_bytes(const void* data, std::size_t size);

} // namespace pkgimage::detail
