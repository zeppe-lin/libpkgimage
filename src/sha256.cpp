// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sha256.h"

#include <libpkgimage/error.h>

#include <limits>

namespace pkgimage::detail {
namespace {

[[noreturn]] void
hash_failure(const char* operation)
{
  throw digest_error(std::string("SHA-256 ") + operation + " failed");
}

} // namespace

sha256_context::sha256_context()
    : context_(EVP_MD_CTX_new())
{
  if (context_ == nullptr)
    hash_failure("context allocation");
  if (EVP_DigestInit_ex(context_, EVP_sha256(), nullptr) != 1)
  {
    EVP_MD_CTX_free(context_);
    context_ = nullptr;
    hash_failure("initialization");
  }
}

sha256_context::~sha256_context()
{
  EVP_MD_CTX_free(context_);
}

void
sha256_context::update(const void* data, std::size_t size)
{
  if (finished_)
    throw digest_error("SHA-256 context already finalized");
  if (size != 0 && data == nullptr)
    throw digest_error("SHA-256 update received a null buffer");
  if (EVP_DigestUpdate(context_, data, size) != 1)
    hash_failure("update");
}

sha256_digest_bytes
sha256_context::finish()
{
  if (finished_)
    throw digest_error("SHA-256 context already finalized");

  sha256_digest_bytes result {};
  unsigned int size = 0;
  if (EVP_DigestFinal_ex(context_, result.data(), &size) != 1)
    hash_failure("finalization");
  if (size != result.size())
    throw digest_error("SHA-256 returned an invalid digest length");

  finished_ = true;
  return result;
}

sha256_digest_bytes
sha256_bytes(const void* data, std::size_t size)
{
  sha256_context context;
  context.update(data, size);
  return context.finish();
}

} // namespace pkgimage::detail
