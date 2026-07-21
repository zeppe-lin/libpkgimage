# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later

if get_option('tests').allowed()
  sh = find_program('sh')

  digest_test = executable(
    'digest-test',
    'digest_test.cpp',
    dependencies: libpkgimage_dep,
  )
  test('validate typed digests', digest_test)

  package_path_test = executable(
    'package-path-test',
    'package_path_test.cpp',
    dependencies: libpkgimage_dep,
  )
  test('normalize package paths', package_path_test)

  package_image_test = executable(
    'package-image-test',
    'package_image_test.cpp',
    dependencies: libpkgimage_dep,
  )
  test('validate package images', package_image_test)

  entry_selection_test = executable(
    'entry-selection-test',
    'entry_selection_test.cpp',
    dependencies: libpkgimage_dep,
  )
  test('validate payload selections', entry_selection_test)

  libarchive_backend_test = executable(
    'libarchive-backend-test',
    'libarchive_backend_test.cpp',
    dependencies: libpkgimage_dep,
  )
  test('normalize libarchive images', libarchive_backend_test)

  payload_replay_test = executable(
    'payload-replay-test',
    'payload_replay_test.cpp',
    dependencies: libpkgimage_dep,
  )
  test('replay selected package payloads', payload_replay_test)

  umbrella_header_test = executable(
    'umbrella-header-test',
    'umbrella_header_test.cpp',
    dependencies: libpkgimage_dep,
  )
  test('compile umbrella header', umbrella_header_test)

  public_headers = [
    ['libpkgimage', 'libpkgimage/libpkgimage.h'],
    ['error', 'libpkgimage/error.h'],
    ['digest', 'libpkgimage/digest.h'],
    ['package-path', 'libpkgimage/package_path.h'],
    ['package-entry', 'libpkgimage/package_entry.h'],
    ['package-image', 'libpkgimage/package_image.h'],
    ['entry-selection', 'libpkgimage/entry_selection.h'],
    ['payload-sink', 'libpkgimage/payload_sink.h'],
    ['package-archive', 'libpkgimage/package_archive.h'],
    ['archive-backend', 'libpkgimage/archive_backend.h'],
    ['libarchive-backend', 'libpkgimage/libarchive_backend.h'],
  ]

  foreach header : public_headers
    public_header_test = executable(
      header[0] + '-public-header-test',
      'public_header_test.cpp',
      cpp_args: [
        '-DLIBPKGIMAGE_TEST_HEADER="' + header[1] + '"',
      ],
      dependencies: libpkgimage_dep,
    )
    test('compile public header ' + header[1], public_header_test)
  endforeach

  test(
    'verify documentation source contracts',
    sh,
    args: [
      join_paths(
        meson.current_source_dir(),
        'documentation_source_test.sh',
      ),
      meson.project_source_root(),
    ],
  )
endif
