// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file entry_selection.h
 * \brief Immutable selection of package payload entries.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include <libpkgimage/package_image.h>

namespace pkgimage {

/*!
 * \brief Immutable set of regular entries selected from a package image.
 *
 * A selection is bound to the package-image identity and retains the selected
 * entry identifiers, paths, and regular-content identities. Equivalent copies
 * of the same immutable image are accepted; every different image is rejected.
 */
class entry_selection final {
public:
  /*!
   * \brief Select every regular-file entry in an image.
   * \param image Image whose regular payloads should be selected.
   * \return Selection bound to \p image.
   */
  [[nodiscard]] static entry_selection
  all_regular(const package_image& image);

  /*!
   * \brief Select specific regular entries by stable image identifier.
   * \param image Image that defines the identifiers.
   * \param ids Entry identifiers to select.  Input order is irrelevant;
   *        replay always follows archive order.
   * \return Validated immutable selection.
   * \throws selection_error if an identifier is absent, duplicated, or
   *         refers to a non-regular entry.
   */
  [[nodiscard]] static entry_selection
  from_ids(const package_image& image, std::vector<entry_id> ids);

  /*!
   * \brief Return whether an entry identifier is selected.
   */
  [[nodiscard]] bool contains(entry_id id) const noexcept;

  /*!
   * \brief Return the number of selected entries.
   */
  [[nodiscard]] std::size_t size() const noexcept;

  /*!
   * \brief Validate this selection against an image.
   * \param image Image intended for replay.
   * \throws selection_error if the image does not match the selection.
   */
  void validate(const package_image& image) const;

private:
  struct selected_entry final {
    entry_id id;
    package_path path;
    regular_content_digest content_identity;
  };

  entry_selection(package_image_identity image_identity,
                  std::size_t image_size,
                  std::vector<std::uint8_t> selected,
                  std::vector<selected_entry> entries);

  package_image_identity image_identity_;
  std::size_t image_size_;
  std::vector<std::uint8_t> selected_;
  std::vector<selected_entry> entries_;
};

} // namespace pkgimage
