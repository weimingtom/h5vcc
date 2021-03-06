// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// DataPack represents a read-only view onto an on-disk file that contains
// (key, value) pairs of data.  It's used to store static resources like
// translation strings and images.

#ifndef UI_BASE_RESOURCE_DATA_PACK_H_
#define UI_BASE_RESOURCE_DATA_PACK_H_

#include <map>

#include "base/basictypes.h"
#include "base/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "base/platform_file.h"
#include "base/string_piece.h"
#include "ui/base/layout.h"
#include "ui/base/resource/resource_handle.h"
#include "ui/base/ui_export.h"

class FilePath;

namespace base {
class RefCountedStaticMemory;
}

namespace file_util {
class MemoryMappedFile;
}

namespace ui {

class UI_EXPORT DataPack : public ResourceHandle {
 public:
  DataPack(ui::ScaleFactor scale_factor);
  virtual ~DataPack();

  // Load a pack file from |path|, returning false on error.
  bool LoadFromPath(const FilePath& path);

  // Loads a pack file from |file|, returning false on error.
  bool LoadFromFile(base::PlatformFile file);

#if !defined(__LB_SHELL__)
  // Writes a pack file containing |resources| to |path|. If there are any
  // text resources to be written, their encoding must already agree to the
  // |textEncodingType| specified. If no text resources are present, please
  // indicate BINARY.
  static bool WritePack(const FilePath& path,
                        const std::map<uint16, base::StringPiece>& resources,
                        TextEncodingType textEncodingType);
#endif

  // ResourceHandle implementation:
  virtual bool HasResource(uint16 resource_id) const OVERRIDE;
  virtual bool GetStringPiece(uint16 resource_id,
                              base::StringPiece* data) const OVERRIDE;
  virtual base::RefCountedStaticMemory* GetStaticMemory(
      uint16 resource_id) const OVERRIDE;
  virtual TextEncodingType GetTextEncodingType() const OVERRIDE;
  virtual ui::ScaleFactor GetScaleFactor() const OVERRIDE;

 private:
  // Does the actual loading of a pack file. Called by Load and LoadFromFile.
  bool LoadImpl();

#if defined(__LB_SHELL__)

#if defined(COMPILER_MSVC)
# define PACK(decl) __pragma(pack(push, 1)) decl __pragma(pack(pop))
#else
# define PACK(decl) decl __attribute__((__packed__))
#endif

  // These have to be packed to match the structure from the data file.
  // The performance hit is probably okay since this is not accessed much.
  PACK(struct DataPackHeader {
    uint32 version;
    uint32 resource_count;
    uint8 encoding;
  });
  PACK(struct DataPackEntry {
    uint16 resource_id;
    uint32 file_offset;
  });
  COMPILE_ASSERT(sizeof(DataPackHeader) == 9,
                 size_of_header_must_be_nine);
  COMPILE_ASSERT(sizeof(DataPackEntry) == 6,
                 size_of_entry_must_be_six);

  FilePath pack_file_;
  scoped_array<DataPackEntry> metadata_;
  std::map<uint16, int> resource_id_to_metadata_index_;
  mutable std::map<uint16, std::string> static_cache_;
#else
  // The memory-mapped data.
  scoped_ptr<file_util::MemoryMappedFile> mmap_;
#endif

  // Number of resources in the data.
  size_t resource_count_;

  // Type of encoding for text resources.
  TextEncodingType text_encoding_type_;

  // The scale of the image in this resource pack relative to images in the 1x
  // resource pak.
  ui::ScaleFactor scale_factor_;

  DISALLOW_COPY_AND_ASSIGN(DataPack);
};

}  // namespace ui

#endif  // UI_BASE_RESOURCE_DATA_PACK_H_
