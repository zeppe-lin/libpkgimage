// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include <libpkgimage/error.h>
#include <libpkgimage/package_image.h>

#include "identity_writer.h"

#include <algorithm>
#include <limits>
#include <unordered_map>
#include <utility>

namespace pkgimage {
namespace {

void
validate_link_text(const std::string& target, const package_path& path)
{
  if (target.empty())
    throw manifest_error("empty symlink target for '" + path.string() + "'");

  if (target.find('\0') != std::string::npos
      || target.find('\n') != std::string::npos
      || target.find('\r') != std::string::npos)
  {
    throw manifest_error("invalid symlink target for '" + path.string() + "'");
  }
}

void
validate_entry(const package_entry& entry)
{
  if (entry.type == entry_type::symlink)
  {
    if (!entry.symlink_target)
    {
      throw manifest_error(
          "missing symlink target for '" + entry.path.string() + "'");
    }
    validate_link_text(*entry.symlink_target, entry.path);
  }
  else if (entry.symlink_target)
  {
    throw manifest_error(
        "unexpected symlink target for '" + entry.path.string() + "'");
  }

  if (entry.type == entry_type::hardlink)
  {
    if (!entry.hardlink_target)
    {
      throw manifest_error(
          "missing hardlink target for '" + entry.path.string() + "'");
    }
    if (*entry.hardlink_target == entry.path)
    {
      throw manifest_error(
          "self-referential hardlink '" + entry.path.string() + "'");
    }
  }
  else if (entry.hardlink_target)
  {
    throw manifest_error(
        "unexpected hardlink target for '" + entry.path.string() + "'");
  }

  if (entry.type != entry_type::regular && entry.size != 0)
  {
    throw manifest_error(
        "non-regular entry has payload size: '" + entry.path.string() + "'");
  }

  if (entry.type == entry_type::regular && !entry.regular_content)
  {
    throw manifest_error(
        "regular entry has no content identity: '" + entry.path.string() + "'");
  }
  if (entry.type != entry_type::regular && entry.regular_content)
  {
    throw manifest_error(
        "non-regular entry has a content identity: '"
        + entry.path.string() + "'");
  }

  const bool is_device = entry.type == entry_type::character_device
                      || entry.type == entry_type::block_device;
  if (is_device && !entry.device)
  {
    throw manifest_error(
        "missing device number for '" + entry.path.string() + "'");
  }
  if (!is_device && entry.device)
  {
    throw manifest_error(
        "unexpected device number for '" + entry.path.string() + "'");
  }

  if (entry.mtime_nanoseconds > 999999999U)
  {
    throw manifest_error(
        "invalid mtime nanoseconds for '" + entry.path.string() + "'");
  }
}

std::uint8_t
entry_type_code(entry_type type)
{
  switch (type)
  {
    case entry_type::regular: return 1;
    case entry_type::directory: return 2;
    case entry_type::symlink: return 3;
    case entry_type::hardlink: return 4;
    case entry_type::fifo: return 5;
    case entry_type::character_device: return 6;
    case entry_type::block_device: return 7;
  }
  throw manifest_error("invalid package entry type");
}

void
write_optional_string(detail::identity_writer& writer,
                      const std::optional<std::string>& value)
{
  writer.u8(value ? 1 : 0);
  if (value)
    writer.length_prefixed(*value);
}

void
write_optional_path(detail::identity_writer& writer,
                    const std::optional<package_path>& value)
{
  writer.u8(value ? 1 : 0);
  if (value)
    writer.length_prefixed(value->string());
}

std::uint64_t
to_v1_u64(std::size_t value, const char* field)
{
  if constexpr (sizeof(std::size_t) > sizeof(std::uint64_t))
  {
    if (value > std::numeric_limits<std::uint64_t>::max())
      throw manifest_error(std::string(field) + " exceeds the v1 limit");
  }
  return static_cast<std::uint64_t>(value);
}

package_image_identity
identify(const std::vector<package_entry>& entries)
{
  detail::identity_writer writer;
  writer.length_prefixed("libpkgimage.package-image.v1");
  writer.u64(to_v1_u64(entries.size(), "package-image entry count"));

  for (const package_entry& entry : entries)
  {
    writer.u64(to_v1_u64(entry.id, "package-image entry ordinal"));
    writer.length_prefixed(entry.path.string());
    writer.u8(entry_type_code(entry.type));
    writer.u32(entry.mode);
    writer.u64(entry.uid);
    writer.u64(entry.gid);

    writer.u8(entry.type == entry_type::regular ? 1 : 0);
    if (entry.type == entry_type::regular)
      writer.u64(entry.size);

    writer.i64(entry.mtime);
    writer.u32(entry.mtime_nanoseconds);
    write_optional_string(writer, entry.symlink_target);
    write_optional_path(writer, entry.hardlink_target);

    writer.u8(entry.device ? 1 : 0);
    if (entry.device)
    {
      writer.u64(entry.device->major);
      writer.u64(entry.device->minor);
    }

    writer.u8(entry.regular_content ? 1 : 0);
    if (entry.regular_content)
    {
      writer.digest(entry.regular_content->representation_version(),
                    entry.regular_content->algorithm(),
                    entry.regular_content->bytes());
    }
  }

  return package_image_identity::from_sha256(writer.finish());
}

} // namespace

struct package_image::construction_result final {
  std::vector<package_entry> entries;
  package_image_identity identity;
};

package_image::package_image(std::vector<package_entry> entries)
    : package_image(construct(std::move(entries)))
{
}

package_image::package_image(construction_result result)
    : entries_(std::move(result.entries)),
      identity_(std::move(result.identity))
{
}

package_image::construction_result
package_image::construct(std::vector<package_entry> entries)
{
  if (entries.empty())
    throw manifest_error("package image is empty");

  std::unordered_map<std::string, std::size_t> by_path;
  by_path.reserve(entries.size());

  for (std::size_t i = 0; i < entries.size(); ++i)
  {
    package_entry& entry = entries[i];
    entry.id = i;
    validate_entry(entry);

    const auto inserted = by_path.emplace(entry.path.string(), i);
    if (!inserted.second)
    {
      throw manifest_error(
          "duplicate package path '" + entry.path.string() + "'");
    }
  }

  for (const package_entry& entry : entries)
  {
    if (entry.type != entry_type::hardlink)
      continue;

    const auto target = by_path.find(entry.hardlink_target->string());
    if (target == by_path.end())
    {
      throw manifest_error(
          "hardlink target is absent: '" + entry.path.string() + "' -> '"
          + entry.hardlink_target->string() + "'");
    }

    if (entries[target->second].type != entry_type::regular)
    {
      throw manifest_error(
          "hardlink target is not a regular file: '" + entry.path.string()
          + "' -> '" + entry.hardlink_target->string() + "'");
    }
  }

  package_image_identity identity = identify(entries);
  return construction_result {std::move(entries), std::move(identity)};
}

const std::vector<package_entry>&
package_image::entries() const noexcept
{
  return entries_;
}

std::size_t
package_image::size() const noexcept
{
  return entries_.size();
}

const package_image_identity&
package_image::identity() const noexcept
{
  return identity_;
}

const package_entry*
package_image::entry(entry_id id) const noexcept
{
  return id < entries_.size() ? &entries_[id] : nullptr;
}

const package_entry*
package_image::find(const package_path& path) const noexcept
{
  const auto found = std::find_if(
      entries_.begin(), entries_.end(),
      [&path](const package_entry& entry) { return entry.path == path; });
  return found == entries_.end() ? nullptr : &*found;
}

} // namespace pkgimage
