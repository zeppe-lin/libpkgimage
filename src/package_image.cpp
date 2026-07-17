/*
 * Copyright (C) 2026 Alexandr Savca
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <libpkgimage/error.h>
#include <libpkgimage/package_image.h>

#include <algorithm>
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

} // namespace

package_image::package_image(std::vector<package_entry> entries)
    : entries_(std::move(entries))
{
  if (entries_.empty())
    throw manifest_error("package image is empty");

  std::unordered_map<std::string, std::size_t> by_path;
  by_path.reserve(entries_.size());

  for (std::size_t i = 0; i < entries_.size(); ++i)
  {
    const package_entry& entry = entries_[i];
    validate_entry(entry);

    const auto inserted = by_path.emplace(entry.path.string(), i);
    if (!inserted.second)
    {
      throw manifest_error(
          "duplicate package path '" + entry.path.string() + "'");
    }
  }

  for (const package_entry& entry : entries_)
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

    if (entries_[target->second].type != entry_type::regular)
    {
      throw manifest_error(
          "hardlink target is not a regular file: '" + entry.path.string()
          + "' -> '" + entry.hardlink_target->string() + "'");
    }
  }
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

const package_entry*
package_image::find(const package_path& path) const noexcept
{
  const auto found = std::find_if(
      entries_.begin(), entries_.end(),
      [&path](const package_entry& entry) { return entry.path == path; });
  return found == entries_.end() ? nullptr : &*found;
}

} // namespace pkgimage
