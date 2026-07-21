libpkgimage history
===================

Lineage
-------

`libpkgimage` is original Zeppe-Lin code.

It was created to replace archive inspection and payload access formerly
entangled with CRUX-derived package tooling. No CRUX `pkgutils` or
`libpkgcore` implementation was imported.

Initial model
-------------

The initial project established:

* canonical package-root paths;
* typed backend-neutral archive entries;
* ordered and validated package images;
* duplicate-path rejection;
* explicit empty-directory retention;
* libarchive tar normalization; and
* model and backend tests.

0.2 series
----------

The 0.2 series added:

* stable image-local entry identifiers;
* immutable regular-file selections;
* backend-neutral payload sinks;
* stable inspected archive objects;
* retained-descriptor payload replay;
* exact payload-size validation;
* source-change detection; and
* replay and source-stability tests.

Documentation reconstruction
----------------------------

The documentation reconstruction added:

* explicit model, replay, backend, and integration contracts;
* a durable archive-image versus footprint boundary;
* a testing doctrine;
* library, model, archive, replay, and backend manual pages;
* independent public-header tests;
* documentation consistency tests; and
* CI gates for scdoc and Doxygen.

0.3 series
----------

The 0.3 series seals archive truth:

* typed complete-archive, regular-content, image, and receipt identities;
* streaming SHA-256 through private libcrypto integration;
* canonical package-image identity version 1;
* exact retained-source hashing;
* immutable archive-inspection receipts;
* receipt-bearing inspection results;
* image-bound entry selections;
* replay-time regular-content verification; and
* explicit ABI generation 1.
