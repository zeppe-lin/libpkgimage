libpkgimage archive identity and inspection evidence
====================================================

Authority
---------

`libpkgimage` is authoritative for the normalized meaning of exact package
archive bytes. It owns path and entry normalization, decoded regular-file
content identity, package-image identity, and the inspection receipt that binds
one exact input archive to one normalized image.

It does not assign package release identity, select candidates, interpret
artifact manifests, establish provider provenance, verify signatures, decide
trust, choose installation policy, describe installed ownership, observe a
target filesystem, or apply a package.

Identity domains
----------------

The public model keeps these domains distinct:

```text
complete_archive_digest
regular_content_digest
package_image_identity
archive_inspection_receipt_identity
```

A complete-archive digest identifies every exact input byte, including archive
headers, ordering, compression, and trailing transport bytes.

A regular-content digest identifies the decoded bytes of one regular archive
entry. Empty regular files identify the empty byte sequence. Hard-link entries
have no independent payload digest; they retain their target-path relation to a
regular entry in the same image.

A package-image identity identifies the complete ordered normalized semantics
of one `package_image`, including every regular-content digest. It is not an
artifact identity, package release identity, footprint digest, installed
ownership identity, or future payload-equivalence digest.

Canonical package-image record
------------------------------

Package-image identity version 1 hashes a deterministic record beginning with:

```text
libpkgimage.package-image.v1
```

The record uses explicit big-endian integer encodings and length-prefixed byte
strings. It records the entry count and, for each archive-order entry, its
image-local ordinal, canonical path, type, mode, uid, gid, semantically
applicable size, mtime seconds and nanoseconds, applicable link target or device
number, and applicable regular-content digest.

It never includes an archive pathname, source descriptor, source stat facts,
backend handle, diagnostic text, package identity, provider facts, artifact
manifest, installed state, or target observations.

Inspection receipt
------------------

A successful archive-inspection receipt binds:

```text
complete archive digest
archive backend semantic identity
inspection schema version
package-image identity
entry count
```

Its identity is derived from a separate canonical record beginning with:

```text
libpkgimage.archive-inspection-receipt.v1
```

The backend identity describes semantic inspection behavior, not compiler,
linkage mode, or the linked archive library patch release.

The source pathname may be retained for diagnostics but participates in no
identity. A receipt states what bytes were inspected and what image was
produced. It does not certify package naming, provenance, signatures, trust,
candidate acceptance, or installation acceptance.

Inspection transition
---------------------

Inspection retains one stable source according to the documented source
contract, hashes the complete retained source, decodes every entry, streams and
hashes every regular payload, validates decoded byte counts, constructs the
immutable image, derives its identity, constructs the receipt, and validates
source stability before publishing either result.

The complete-archive digest and archive interpretation are derived from the
same retained source. Reopening the pathname to establish the binding is
forbidden.

Replay
------

Replay transports selected regular bytes. It validates the selection against
the package-image identity, compares current normalized headers with the sealed
image, validates declared sizes, and checks each replayed regular payload
against its inspection-time content digest before calling `payload_sink::end()`.

A sink may already have consumed bytes when a late failure is reported. Replay
is not transactional and performs no target filesystem mutation.
