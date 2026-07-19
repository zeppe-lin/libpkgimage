// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file package_archive.h
 * \brief Inspected package archive with replayable payload access.
 */

#pragma once

#include <libpkgimage/entry_selection.h>
#include <libpkgimage/payload_sink.h>

namespace pkgimage {

/*!
 * \brief Stable inspected package archive.
 *
 * A package_archive owns a stable source handle and an immutable normalized
 * image.  Payload replay addresses entries through that image and never
 * extracts files or interprets installation policy.
 */
class package_archive {
public:
  /*!
   * \brief Destroy the package archive.
   */
  virtual ~package_archive() = default;

  /*!
   * \brief Return the immutable normalized package image.
   */
  [[nodiscard]] virtual const package_image& image() const noexcept = 0;

  /*!
   * \brief Replay selected regular-file payloads in archive order.
   * \param selection Selection created from this image or an equivalent copy.
   * \param sink Destination for payload events.
   * \throws selection_error if \p selection does not match the image.
   * \throws source_changed_error if the retained archive source changed.
   * \throws archive_error for archive decoding, format, or I/O failures.
   *
   * The method performs no filesystem mutation.  Sink exceptions propagate
   * unchanged.  A backend may support concurrent replay calls, but consumers
   * must not rely on concurrency unless the concrete backend documents it.
   */
  virtual void replay(const entry_selection& selection,
                      payload_sink& sink) const = 0;
};

} // namespace pkgimage
