// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file digest.h
 * \brief Typed archive, content, image, and receipt identities.
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace pkgimage {

/*! \brief Version of the public algorithm-qualified digest representation. */
inline constexpr std::uint16_t digest_representation_version = 1;

/*! \brief Algorithm identifier carried by digest-domain values. */
enum class digest_algorithm : std::uint16_t {
  sha256 = 1, //!< SHA-256 with a 32-byte result.
};

/*! \brief Number of bytes in a SHA-256 result. */
inline constexpr std::size_t sha256_digest_size = 32;

/*! \brief Algorithm-neutral storage for a digest result. */
using digest_bytes = std::vector<std::uint8_t>;

/*! \brief Fixed-size SHA-256 result accepted by construction factories. */
using sha256_digest_bytes = std::array<std::uint8_t, sha256_digest_size>;

/*! \brief Exact-byte digest of one complete archive source. */
class complete_archive_digest final {
public:
  /*!
   * \brief Construct a typed SHA-256 value from exact digest bytes.
   * \param bytes SHA-256 result bytes in network-independent order.
   */
  [[nodiscard]] static complete_archive_digest
  from_sha256(sha256_digest_bytes bytes);

  /*!
   * \brief Parse the canonical versioned representation.
   * \param input Text in v1:sha256:<lowercase-hex> form.
   */
  [[nodiscard]] static complete_archive_digest parse(std::string_view input);

  /*! \brief Return the digest representation version. */
  [[nodiscard]] std::uint16_t representation_version() const noexcept;

  /*! \brief Return the represented digest algorithm. */
  [[nodiscard]] digest_algorithm algorithm() const noexcept;

  /*! \brief Return the exact digest bytes. */
  [[nodiscard]] const digest_bytes& bytes() const noexcept;

  /*! \brief Return canonical versioned algorithm-qualified text. */
  [[nodiscard]] std::string string() const;

  /*! \brief Compare two values in this semantic digest domain. */
  friend bool operator==(const complete_archive_digest& lhs,
                         const complete_archive_digest& rhs) noexcept;

  /*! \brief Compare two values in this semantic digest domain. */
  friend bool operator!=(const complete_archive_digest& lhs,
                         const complete_archive_digest& rhs) noexcept;

private:
  complete_archive_digest(std::uint16_t representation_version,
                          digest_algorithm algorithm,
                          digest_bytes bytes);

  std::uint16_t representation_version_;
  digest_algorithm algorithm_;
  digest_bytes bytes_;
};

/*! \brief Digest of the decoded bytes of one regular archive entry. */
class regular_content_digest final {
public:
  /*!
   * \brief Construct a typed SHA-256 value from exact digest bytes.
   * \param bytes SHA-256 result bytes in network-independent order.
   */
  [[nodiscard]] static regular_content_digest
  from_sha256(sha256_digest_bytes bytes);

  /*!
   * \brief Parse the canonical versioned representation.
   * \param input Text in v1:sha256:<lowercase-hex> form.
   */
  [[nodiscard]] static regular_content_digest parse(std::string_view input);

  /*! \brief Return the digest representation version. */
  [[nodiscard]] std::uint16_t representation_version() const noexcept;

  /*! \brief Return the represented digest algorithm. */
  [[nodiscard]] digest_algorithm algorithm() const noexcept;

  /*! \brief Return the exact digest bytes. */
  [[nodiscard]] const digest_bytes& bytes() const noexcept;

  /*! \brief Return canonical versioned algorithm-qualified text. */
  [[nodiscard]] std::string string() const;

  /*! \brief Compare two values in this semantic digest domain. */
  friend bool operator==(const regular_content_digest& lhs,
                         const regular_content_digest& rhs) noexcept;

  /*! \brief Compare two values in this semantic digest domain. */
  friend bool operator!=(const regular_content_digest& lhs,
                         const regular_content_digest& rhs) noexcept;

private:
  regular_content_digest(std::uint16_t representation_version,
                         digest_algorithm algorithm,
                         digest_bytes bytes);

  std::uint16_t representation_version_;
  digest_algorithm algorithm_;
  digest_bytes bytes_;
};

/*! \brief Identity of one complete ordered normalized package image. */
class package_image_identity final {
public:
  /*!
   * \brief Construct a typed SHA-256 value from exact digest bytes.
   * \param bytes SHA-256 result bytes in network-independent order.
   */
  [[nodiscard]] static package_image_identity
  from_sha256(sha256_digest_bytes bytes);

  /*!
   * \brief Parse the canonical versioned representation.
   * \param input Text in v1:sha256:<lowercase-hex> form.
   */
  [[nodiscard]] static package_image_identity parse(std::string_view input);

  /*! \brief Return the digest representation version. */
  [[nodiscard]] std::uint16_t representation_version() const noexcept;

  /*! \brief Return the represented digest algorithm. */
  [[nodiscard]] digest_algorithm algorithm() const noexcept;

  /*! \brief Return the exact digest bytes. */
  [[nodiscard]] const digest_bytes& bytes() const noexcept;

  /*! \brief Return canonical versioned algorithm-qualified text. */
  [[nodiscard]] std::string string() const;

  /*! \brief Compare two values in this semantic digest domain. */
  friend bool operator==(const package_image_identity& lhs,
                         const package_image_identity& rhs) noexcept;

  /*! \brief Compare two values in this semantic digest domain. */
  friend bool operator!=(const package_image_identity& lhs,
                         const package_image_identity& rhs) noexcept;

private:
  package_image_identity(std::uint16_t representation_version,
                         digest_algorithm algorithm,
                         digest_bytes bytes);

  std::uint16_t representation_version_;
  digest_algorithm algorithm_;
  digest_bytes bytes_;
};

/*! \brief Identity of one archive-inspection receipt. */
class archive_inspection_receipt_identity final {
public:
  /*!
   * \brief Construct a typed SHA-256 value from exact digest bytes.
   * \param bytes SHA-256 result bytes in network-independent order.
   */
  [[nodiscard]] static archive_inspection_receipt_identity
  from_sha256(sha256_digest_bytes bytes);

  /*!
   * \brief Parse the canonical versioned representation.
   * \param input Text in v1:sha256:<lowercase-hex> form.
   */
  [[nodiscard]] static archive_inspection_receipt_identity
  parse(std::string_view input);

  /*! \brief Return the digest representation version. */
  [[nodiscard]] std::uint16_t representation_version() const noexcept;

  /*! \brief Return the represented digest algorithm. */
  [[nodiscard]] digest_algorithm algorithm() const noexcept;

  /*! \brief Return the exact digest bytes. */
  [[nodiscard]] const digest_bytes& bytes() const noexcept;

  /*! \brief Return canonical versioned algorithm-qualified text. */
  [[nodiscard]] std::string string() const;

  /*! \brief Compare two values in this semantic digest domain. */
  friend bool operator==(const archive_inspection_receipt_identity& lhs,
                         const archive_inspection_receipt_identity& rhs) noexcept;

  /*! \brief Compare two values in this semantic digest domain. */
  friend bool operator!=(const archive_inspection_receipt_identity& lhs,
                         const archive_inspection_receipt_identity& rhs) noexcept;

private:
  archive_inspection_receipt_identity(
      std::uint16_t representation_version,
      digest_algorithm algorithm,
      digest_bytes bytes);

  std::uint16_t representation_version_;
  digest_algorithm algorithm_;
  digest_bytes bytes_;
};

} // namespace pkgimage
