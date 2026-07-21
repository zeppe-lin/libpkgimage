#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later

set -eu

root=$1

if grep -R -E '^#include <(openssl|archive)(/|\.|>)' \
    "$root/include/libpkgimage" >/dev/null; then
  echo 'public headers expose an implementation dependency' >&2
  exit 1
fi

grep -F "requires_private: ['libarchive', 'libcrypto']" \
  "$root/src/meson.build" >/dev/null || {
  echo 'static pkg-config dependency closure is incomplete' >&2
  exit 1
}
