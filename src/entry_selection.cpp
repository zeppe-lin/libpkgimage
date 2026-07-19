// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include <libpkgimage/entry_selection.h>
#include <libpkgimage/error.h>

#include <algorithm>
#include <string>
#include <utility>

namespace pkgimage {

entry_selection::entry_selection(
    std::size_t image_size,
    std::vector<std::uint8_t> selected,
    std::vector<selected_entry> entries)
    : image_size_(image_size),
      selected_(std::move(selected)),
      entries_(std::move(entries))
{
}

entry_selection
entry_selection::all_regular(const package_image& image)
{
  std::vector<entry_id> ids;
  ids.reserve(image.size());

  for (const package_entry& entry : image.entries())
  {
    if (entry.type == entry_type::regular)
      ids.push_back(entry.id);
  }

  return from_ids(image, std::move(ids));
}

entry_selection
entry_selection::from_ids(const package_image& image,
                          std::vector<entry_id> ids)
{
  std::vector<std::uint8_t> selected(image.size(), 0);
  std::vector<selected_entry> entries;
  entries.reserve(ids.size());

  for (entry_id id : ids)
  {
    const package_entry* entry = image.entry(id);
    if (entry == nullptr)
    {
      throw selection_error(
          "package entry identifier is out of range: "
          + std::to_string(id));
    }

    if (entry->type != entry_type::regular)
    {
      throw selection_error(
          "package entry has no replayable payload: '"
          + entry->path.string() + "'");
    }

    if (selected[id] != 0)
    {
      throw selection_error(
          "package entry selected more than once: '"
          + entry->path.string() + "'");
    }

    selected[id] = 1;
    entries.push_back(selected_entry {id, entry->path});
  }

  std::sort(
      entries.begin(), entries.end(),
      [](const selected_entry& lhs, const selected_entry& rhs) {
        return lhs.id < rhs.id;
      });

  return entry_selection(image.size(), std::move(selected),
                         std::move(entries));
}

bool
entry_selection::contains(entry_id id) const noexcept
{
  return id < selected_.size() && selected_[id] != 0;
}

std::size_t
entry_selection::size() const noexcept
{
  return entries_.size();
}

void
entry_selection::validate(const package_image& image) const
{
  if (image.size() != image_size_)
    throw selection_error("entry selection belongs to a different image");

  for (const selected_entry& selected : entries_)
  {
    const package_entry* entry = image.entry(selected.id);
    if (entry == nullptr
        || entry->type != entry_type::regular
        || entry->path != selected.path)
    {
      throw selection_error("entry selection belongs to a different image");
    }
  }
}

} // namespace pkgimage
