# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later

scdoc = find_program('scdoc', required: get_option('man_pages'))
if scdoc.found()
  foreach page : [
    'libpkgimage.3',
    'pkgimage_model.3',
    'pkgimage_archive.3',
    'pkgimage_replay.3',
    'pkgimage_libarchive_backend.3',
  ]
    custom_target(
      page,
      input: page + '.scdoc',
      output: page,
      command: [scdoc],
      feed: true,
      capture: true,
      install: true,
      install_dir: get_option('mandir') / 'man3',
    )
  endforeach
endif
