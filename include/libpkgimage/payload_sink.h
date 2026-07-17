/*
 * Copyright (C) 2026 Alexandr Savca
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*!
 * \file payload_sink.h
 * \brief Backend-neutral consumer of regular-file payload data.
 * \copyright See COPYING for license terms and COPYRIGHT for notices.
 */

#pragma once

#include <cstddef>

#include <libpkgimage/package_entry.h>

namespace pkgimage {

/*!
 * \brief Receives selected regular-file payloads in package order.
 *
 * A payload sink is called once for each selected regular entry.  begin()
 * precedes zero or more write() calls and end() completes the entry.  Empty
 * regular files therefore produce begin() and end() without a write().
 *
 * Exceptions raised by a sink propagate unchanged.  When begin() or write()
 * fails, end() is not called for that entry.
 */
class payload_sink {
public:
  /*!
   * \brief Destroy the payload sink.
   */
  virtual ~payload_sink() = default;

  /*!
   * \brief Begin one selected regular-file payload.
   * \param entry Entry whose payload follows.
   */
  virtual void begin(const package_entry& entry) = 0;

  /*!
   * \brief Consume one payload block.
   * \param entry Entry owning the payload block.
   * \param data Pointer to binary payload bytes.
   * \param size Number of bytes available at \p data.
   */
  virtual void write(const package_entry& entry,
                     const std::byte* data,
                     std::size_t size) = 0;

  /*!
   * \brief Complete one selected regular-file payload.
   * \param entry Entry whose payload is complete.
   */
  virtual void end(const package_entry& entry) = 0;
};

} // namespace pkgimage
