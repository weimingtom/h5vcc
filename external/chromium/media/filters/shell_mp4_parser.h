/*
 * Copyright 2012 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MEDIA_FILTERS_SHELL_MP4_PARSER_H_
#define MEDIA_FILTERS_SHELL_MP4_PARSER_H_

#include "media/filters/shell_avc_parser.h"
#include "media/filters/shell_mp4_map.h"

// If true the parser will save every atom it parses to disk. Note that since
// the parser is lazy and only parses what it needs to build the map and know
// the required video config information it is likely to not build a complete
// atom table for a given file.
#define SHELL_MP4_PARSER_DUMP_ATOMS 0

namespace media {

// How many bytes to download from the start of the atom? Should be large
// enough that we can extract all the data we need from the atom without
// second download (typically), but no larger. This is currently set at 16
// bytes for the 8 byte header + optional 8 byte size extension plus 20 bytes
// for the needed values within an mvhd header. We leave this is the header so
// that ShellMP4Map can re-use,
static const int kAtomDownload = 36;

// mp4 atom fourCC codes as big-endian unsigned ints
static const uint32 kAtomType_avc1 = 0x61766331;  // skip in to subatom
static const uint32 kAtomType_avcC = 0x61766343;  // download and parse
static const uint32 kAtomType_co64 = 0x636f3634;  // cache in table
static const uint32 kAtomType_ctts = 0x63747473;  // cache in table
static const uint32 kAtomType_dinf = 0x64696e66;  // skip whole atom
static const uint32 kAtomType_dref = 0x64726566;  // skip whole atom
static const uint32 kAtomType_esds = 0x65736473;  // download and parse
static const uint32 kAtomType_ftyp = 0x66747970;  // top of the file only
static const uint32 kAtomType_hdlr = 0x68646c72;  // parse first 12 bytes
static const uint32 kAtomType_mdhd = 0x6d646864;  // parse first 20 bytes
static const uint32 kAtomType_mdia = 0x6d646961;  // container atom, no-op
static const uint32 kAtomType_minf = 0x6d696e66;  // container atom, no-op
static const uint32 kAtomType_moov = 0x6d6f6f76;  // container atom, no-op
static const uint32 kAtomType_mp4a = 0x6d703461;  // parse first 10 bytes
static const uint32 kAtomType_mvhd = 0x6d766864;  // parse first 20 bytes
static const uint32 kAtomType_smhd = 0x736d6862;  // skip whole atom
static const uint32 kAtomType_stbl = 0x7374626c;  // container atom, no-op
static const uint32 kAtomType_stco = 0x7374636f;  // cache in table
static const uint32 kAtomType_stts = 0x73747473;  // cache in table
static const uint32 kAtomType_stsc = 0x73747363;  // cache in table
static const uint32 kAtomType_stsd = 0x73747364;  // skip in to subatom
static const uint32 kAtomType_stss = 0x73747373;  // cache in table
static const uint32 kAtomType_stsz = 0x7374737a;  // cache in table
static const uint32 kAtomType_trak = 0x7472616b;  // container atom, no-op
static const uint32 kAtomType_tkhd = 0x746b6864;  // skip whole atom
static const uint32 kAtomType_vmhd = 0x766d6864;  // skip whole atom
// TODO: mp4v!!

class ShellMP4Parser : public ShellAVCParser {
 public:
  // attempts to make sense of the provided bytes of the top of a file as an
  // mp4, and if it does make sense returns a ShellMP4Parser initialized with
  // some basic state. If it doesn't make sense this returns NULL
  static scoped_refptr<ShellParser> Construct(
      scoped_refptr<ShellDataSourceReader> reader,
      const uint8* construction_header,
      const PipelineStatusCB& status_cb);
  ShellMP4Parser(scoped_refptr<ShellDataSourceReader> reader,
                 uint32 ftyp_atom_size);
  virtual ~ShellMP4Parser();

  // === ShellParser implementation
  virtual bool ParseConfig() OVERRIDE;
  virtual scoped_refptr<ShellAU> GetNextAU(DemuxerStream::Type type) OVERRIDE;
  virtual bool SeekTo(base::TimeDelta timestamp) OVERRIDE;

 private:
  bool ParseNextAtom();
  bool ParseMP4_esds(uint64 atom_data_size);
  bool ParseMP4_hdlr(uint64 atom_data_size, uint8* hdlr);
  bool ParseMP4_mdhd(uint64 atom_data_size, uint8* mdhd);
  bool ParseMP4_mp4a(uint64 atom_data_size, uint8* mp4a);
  bool ParseMP4_mvhd(uint64 atom_data_size, uint8* mvhd);
  base::TimeDelta TicksToTime(uint64 ticks, uint32 time_scale_hz);
  uint64 TimeToTicks(base::TimeDelta time, uint32 time_scale_hz);

#if SHELL_MP4_PARSER_DUMP_ATOMS
  void DumpAtomToDisk(uint32 four_cc, uint32 atom_size, uint64 atom_offset);
#endif

  uint64 atom_offset_;
  bool current_trak_is_video_;
  bool current_trak_is_audio_;
  uint32 current_trak_time_scale_;
  base::TimeDelta current_trak_duration_;
  uint32 video_time_scale_hz_;
  base::TimeDelta one_video_tick_;
  uint32 audio_time_scale_hz_;
  base::TimeDelta audio_track_duration_;
  base::TimeDelta video_track_duration_;
  scoped_refptr<ShellMP4Map> audio_map_;
  scoped_refptr<ShellMP4Map> video_map_;
  uint32 audio_sample_;
  uint32 video_sample_;
  // for keeping buffers continuous across time scales
  uint64 first_audio_hole_ticks_;
  base::TimeDelta first_audio_hole_;
};

}  // namespace media

#endif  // MEDIA_FILTERS_SHELL_MP4_PARSER_H_
