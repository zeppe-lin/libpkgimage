libpkgimage payload replay
==========================

Overview
--------

Payload replay transports selected regular-file bytes from one inspected
archive to a caller-provided sink.

The flow is:

```text
package_image
      |
      v
entry_selection
      |
      v
package_archive::replay()
      |
      v
payload_sink
```

Replay does not extract into a live filesystem and does not provide a package
transaction.

Selection contract
------------------

An `entry_selection` is immutable after construction.

It may contain regular entries only.

Selections are created by:

```cpp
entry_selection::all_regular(image)
entry_selection::from_ids(image, ids)
```

`from_ids()` rejects:

* identifiers absent from the image;
* duplicate identifiers; and
* identifiers naming non-regular entries.

A selection records the package-image identity plus the selected entry
identifiers, paths, and regular-content identities. Before replay, `validate()`
requires the same sealed image identity. Equivalent immutable copies remain
valid; a same-width image with matching paths but different content is rejected.

Selection input order does not determine replay order. Replay always follows
archive order.

Sink contract
-------------

For each selected regular entry, a sink receives:

```text
begin(entry)
zero or more write(entry, bytes)
end(entry)
```

An empty regular file produces `begin()` and `end()` with no `write()` call.

Payload bytes are binary. The library performs no text decoding,
normalization, decompression beyond archive transport, or content
transformation.

Sink exceptions propagate unchanged.

If `begin()` or `write()` throws, `end()` is not called for that entry.

Partial consumption
-------------------

Replay is not rollback-capable.

A sink may have:

* begun one or more entries;
* consumed complete earlier entries; or
* consumed part of the current entry

before a sink exception, archive error, or source-change error is reported.

Consumers that require all-or-nothing deployment must write into private
staging storage and publish through their own transaction layer.

Archive replay validation
-------------------------

The libarchive backend reopens the retained descriptor through a fresh archive
reader for each replay.

During replay it:

1. validates the selection against the inspected image;
2. verifies the retained source stamp;
3. decodes entries in archive order;
4. normalizes each current header;
5. compares it with the corresponding inspected entry;
6. hashes and emits selected payload bytes;
7. verifies declared payload sizes;
8. verifies each selected payload against its inspection-time content digest
   before `end(entry)`;
9. verifies image and selected-entry counts;
10. closes the archive reader; and
11. verifies the retained source stamp and complete archive digest again.

A changed header, entry count, payload size, or retained source stamp is
reported rather than accepted as the inspected image.

Source stability
----------------

The libarchive backend retains an open descriptor to the inspected regular
file.

Its source stamp includes:

```text
device
inode
size
modification time
modification-time nanoseconds
```

Atomic pathname replacement after `open()` does not redirect replay because
the retained descriptor continues to name the inspected inode.

Unlinking the original pathname does not invalidate replay while the retained
inode remains available.

Pathname-only or other metadata-only changes that do not alter the source stamp
do not invalidate an otherwise unchanged source.

In-place changes affecting the retained source stamp are reported as
`source_changed_error`.

The backend also compares every normalized archive entry during replay, so
manifest changes detectable through archive decoding are rejected even when a
filesystem stamp alone would be insufficient.

Concurrency
-----------

The abstract `package_archive` interface does not promise concurrent replay.

A concrete backend may support concurrent calls, but consumers must not rely
on that behavior unless the backend documents it.

Sinks remain responsible for their own thread safety.
