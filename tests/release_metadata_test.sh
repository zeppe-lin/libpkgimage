#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later

set -eu
root=$1

grep -F "version: '0.3.0'" "$root/meson.build" >/dev/null
grep -F "soversion: '1'" "$root/src/meson.build" >/dev/null
grep -F 'PROJECT_NUMBER         = 0.3.0' "$root/Doxyfile" >/dev/null
grep -F '`libpkgimage 0.3.0` changes the shared-library ABI generation' \
  "$root/ABI.md" >/dev/null
