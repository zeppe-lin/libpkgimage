# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later

public_inc = include_directories('../include')

libpkgimage = library(
  'pkgimage',
  [
    'error.cpp',
    'digest.cpp',
    'sha256.cpp',
    'identity_writer.cpp',
    'inspection_receipt.cpp',
    'package_path.cpp',
    'package_entry.cpp',
    'package_image.cpp',
    'entry_selection.cpp',
    'archive_backend.cpp',
    'libarchive_backend.cpp',
  ],
  cpp_args: ['-D_FILE_OFFSET_BITS=64'],
  include_directories: public_inc,
  dependencies: [libarchive_dep, libcrypto_dep],
  install: true,
  version: meson.project_version(),
  soversion: '0',
)

libpkgimage_dep = declare_dependency(
  link_with: libpkgimage,
  include_directories: public_inc,
  dependencies: [libarchive_dep, libcrypto_dep],
  compile_args: ['-D_FILE_OFFSET_BITS=64'],
)

meson.override_dependency('libpkgimage', libpkgimage_dep)

install_headers(
  '../include/libpkgimage/libpkgimage.h',
  '../include/libpkgimage/error.h',
  '../include/libpkgimage/digest.h',
  '../include/libpkgimage/inspection_receipt.h',
  '../include/libpkgimage/package_path.h',
  '../include/libpkgimage/package_entry.h',
  '../include/libpkgimage/package_image.h',
  '../include/libpkgimage/entry_selection.h',
  '../include/libpkgimage/payload_sink.h',
  '../include/libpkgimage/package_archive.h',
  '../include/libpkgimage/archive_backend.h',
  '../include/libpkgimage/libarchive_backend.h',
  subdir: 'libpkgimage',
)

pkgconf.generate(
  libpkgimage,
  name: 'libpkgimage',
  filebase: 'libpkgimage',
  description: 'Normalized package image and payload access library',
  subdirs: '.',
  requires_private: ['libarchive', 'libcrypto'],
)
