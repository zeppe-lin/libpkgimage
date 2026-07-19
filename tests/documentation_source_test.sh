#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later

set -eu

source_root=$1

fail()
{
  echo "documentation-source-test: $*" >&2
  exit 1
}

check_page()
{
  relative=$1
  heading=$2
  page=$source_root/$relative

  test -s "$page" || fail "missing or empty $relative"
  first=$(sed -n '1p' "$page")
  test "$first" = "$heading" ||
    fail "$relative heading is '$first', expected '$heading'"
}

check_page man/libpkgimage.3.scdoc LIBPKGIMAGE\(3\)
check_page man/pkgimage_model.3.scdoc PKGIMAGE_MODEL\(3\)
check_page man/pkgimage_archive.3.scdoc PKGIMAGE_ARCHIVE\(3\)
check_page man/pkgimage_replay.3.scdoc PKGIMAGE_REPLAY\(3\)
check_page man/pkgimage_libarchive_backend.3.scdoc \
  PKGIMAGE_LIBARCHIVE_BACKEND\(3\)

grep -F 'parse, write, compare, or update _.footprint_ files' \
  "$source_root/man/libpkgimage.3.scdoc" >/dev/null ||
  fail "library manual omits footprint boundary"

grep -F 'Replay is not rollback-capable.' \
  "$source_root/REPLAY.md" >/dev/null ||
  fail "replay contract omits partial-consumption warning"

grep -F 'begin(entry)' "$source_root/man/pkgimage_replay.3.scdoc" >/dev/null ||
  fail "replay manual omits begin event"
grep -F 'zero or more write(entry, bytes)' \
  "$source_root/man/pkgimage_replay.3.scdoc" >/dev/null ||
  fail "replay manual omits write events"
grep -F 'end(entry)' "$source_root/man/pkgimage_replay.3.scdoc" >/dev/null ||
  fail "replay manual omits end event"

grep -F 'Atomic pathname replacement does not redirect replay' \
  "$source_root/man/pkgimage_replay.3.scdoc" >/dev/null ||
  fail "replay manual omits descriptor-stability contract"

for filter in none gzip bzip2 xz lzip zstd
do
  grep -F "$filter" \
    "$source_root/man/pkgimage_libarchive_backend.3.scdoc" >/dev/null ||
    fail "libarchive backend manual omits $filter filter"
done

for type in \
  'regular files' \
  'explicit directories' \
  'symbolic links' \
  'hard links' \
  'FIFOs' \
  'character devices' \
  'block devices'
do
  grep -F "$type" \
    "$source_root/man/pkgimage_libarchive_backend.3.scdoc" >/dev/null ||
    fail "libarchive backend manual omits $type"
done

for document in \
  DESIGN.md \
  REPLAY.md \
  BACKENDS.md \
  INTEGRATION.md \
  TESTING.md \
  HISTORY.md
do
  test -s "$source_root/$document" || fail "missing or empty $document"
  grep -F "$document" "$source_root/Doxyfile" >/dev/null ||
    fail "Doxyfile omits $document"
done

for page in \
  'libpkgimage(3)' \
  'pkgimage_model(3)' \
  'pkgimage_archive(3)' \
  'pkgimage_replay(3)' \
  'pkgimage_libarchive_backend(3)'
do
  grep -F "$page" "$source_root/README.md" >/dev/null ||
    fail "README omits $page"
done

grep -F 'A `.footprint` is not a `package_image`.' \
  "$source_root/INTEGRATION.md" >/dev/null ||
  fail "integration document omits footprint distinction"

grep -F 'libpkgstate::installed_package' \
  "$source_root/INTEGRATION.md" >/dev/null ||
  fail "integration document omits installed-state boundary"
