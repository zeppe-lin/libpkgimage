if get_option('tests').allowed()
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
endif
