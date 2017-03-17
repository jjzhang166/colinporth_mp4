#pragma once
#include <stdint.h>
#include <stdbool.h>
//{{{
#ifdef __cplusplus
extern "C" {
#endif
//}}}

//{{{  atom defines
#define SUBATOMIC 128

/* atoms with subatoms */
#define ATOM_MOOV 1
#define ATOM_TRAK 2
#define ATOM_EDTS 3
#define ATOM_MDIA 4
#define ATOM_MINF 5
#define ATOM_STBL 6
#define ATOM_UDTA 7
#define ATOM_ILST 8 /* iTunes Metadata list */
#define ATOM_TITLE 9
#define ATOM_ARTIST 10
#define ATOM_WRITER 11
#define ATOM_ALBUM 12
#define ATOM_DATE 13
#define ATOM_TOOL 14
#define ATOM_COMMENT 15
#define ATOM_GENRE1 16
#define ATOM_TRACK 17
#define ATOM_DISC 18
#define ATOM_COMPILATION 19
#define ATOM_GENRE2 20
#define ATOM_TEMPO 21
#define ATOM_COVER 22
#define ATOM_DRMS 23
#define ATOM_SINF 24
#define ATOM_SCHI 25

/* atoms without subatoms */
#define ATOM_FTYP 129
#define ATOM_MDAT 130
#define ATOM_MVHD 131
#define ATOM_TKHD 132
#define ATOM_TREF 133
#define ATOM_MDHD 134
#define ATOM_VMHD 135
#define ATOM_SMHD 136
#define ATOM_HMHD 137
#define ATOM_STSD 138
#define ATOM_STTS 139
#define ATOM_STSZ 140
#define ATOM_STZ2 141
#define ATOM_STCO 142
#define ATOM_STSC 143
#define ATOM_MP4A 144
#define ATOM_MP4V 145
#define ATOM_MP4S 146
#define ATOM_ESDS 147
#define ATOM_META 148 /* iTunes Metadata box */
#define ATOM_NAME 149 /* iTunes Metadata name box */
#define ATOM_DATA 150 /* iTunes Metadata data box */
#define ATOM_CTTS 151
#define ATOM_FRMA 152
#define ATOM_IVIV 153
#define ATOM_PRIV 154
#define ATOM_USER 155
#define ATOM_KEY  156

#define ATOM_ALBUM_ARTIST 157
#define ATOM_CONTENTGROUP   158
#define ATOM_LYRICS         159
#define ATOM_DESCRIPTION    160
#define ATOM_NETWORK        161
#define ATOM_SHOW           162
#define ATOM_EPISODENAME    163
#define ATOM_SORTTITLE      164
#define ATOM_SORTALBUM      165
#define ATOM_SORTARTIST     166
#define ATOM_SORTALBUMARTIST    167
#define ATOM_SORTWRITER     168
#define ATOM_SORTSHOW       169
#define ATOM_SEASON         170
#define ATOM_EPISODE        171
#define ATOM_PODCAST        172

#define ATOM_UNKNOWN 255

#define ATOM_FREE ATOM_UNKNOWN
#define ATOM_SKIP ATOM_UNKNOWN
//}}}
//{{{  track defines
#define MAX_TRACKS 16

#define TRACK_UNKNOWN 0
#define TRACK_AUDIO   1
#define TRACK_VIDEO   2
#define TRACK_SYSTEM  3
//}}}
//{{{  struct mp4ff_callback_t
typedef struct {
  uint32_t (*read)(void* user_data, void* buffer, uint32_t length);
  uint32_t (*write)(void* udata, void* buffer, uint32_t length);
  uint32_t (*seek)(void* user_data, uint64_t position);
  uint32_t (*truncate)(void* user_data);
  void* user_data;
  } mp4ff_callback_t;
//}}}
//{{{  struct mp4ff_tag_t
typedef struct {
  char* item;
  char* value;
  } mp4ff_tag_t;
//}}}
//{{{  struct mp4ff_metadata_t
typedef struct {
  mp4ff_tag_t* tags;
  uint32_t count;
  } mp4ff_metadata_t;
//}}}
//{{{  struct mp4ff_track_t
typedef struct {
  int32_t type;
  int32_t channelCount;
  int32_t sampleSize;
  uint16_t sampleRate;
  int32_t audioType;

  /* stsd */
  int32_t stsd_entry_count;

  /* stsz */
  int32_t stsz_sample_size;
  int32_t stsz_sample_count;
  int32_t* stsz_table;

  /* stts */
  int32_t stts_entry_count;
  int32_t* stts_sample_count;
  int32_t* stts_sample_delta;

  /* stsc */
  int32_t stsc_entry_count;
  int32_t* stsc_first_chunk;
  int32_t* stsc_samples_per_chunk;
  int32_t* stsc_sample_desc_index;

  /* stsc */
  int32_t stco_entry_count;
  int32_t* stco_chunk_offset;

  /* ctts */
  int32_t ctts_entry_count;
  int32_t* ctts_sample_count;
  int32_t* ctts_sample_offset;

  /* esde */
  uint8_t* decoderConfig;
  int32_t decoderConfigLen;

  uint32_t maxBitrate;
  uint32_t avgBitrate;

  uint32_t timeScale;
  uint64_t duration;
  } mp4ff_track_t;
//}}}
//{{{  struct mp4ff_t
typedef struct {
  mp4ff_callback_t* stream;
  int64_t current_position;

  int32_t moov_read;
  uint64_t moov_offset;
  uint64_t moov_size;
  uint8_t last_atom;
  uint64_t file_size;

  /* mvhd */
  int32_t time_scale;
  int32_t duration;

  /* incremental track index while reading the file */
  int32_t total_tracks;

  /* track data */
  mp4ff_track_t* track[MAX_TRACKS];

  /* metadata */
  mp4ff_metadata_t tags;

  bool debug;
  } mp4ff_t;
//}}}

mp4ff_t* mp4ff_open (mp4ff_callback_t* f, bool debug);
void mp4ff_close (mp4ff_t* f);

int32_t mp4ff_get_sample_duration (const mp4ff_t* f, int track, int sample);
int32_t mp4ff_get_sample_duration_use_offsets (const mp4ff_t* f, int track, int sample);
int64_t mp4ff_get_sample_position (const mp4ff_t* f, int track, int sample);
int32_t mp4ff_get_sample_offset (const mp4ff_t* f, int track, int sample);

int32_t mp4ff_find_sample (const mp4ff_t* f, int track, int64_t offset, int32_t* toskip);
int32_t mp4ff_find_sample_use_offsets (const mp4ff_t* f, int track, int64_t offset, int32_t* toskip);

int32_t mp4ff_read_sample_size (mp4ff_t* f, int track, int sample);
int32_t mp4ff_read_sample (mp4ff_t* f, int track, int sample, uint8_t* buffer);

int32_t mp4ff_get_decoder_config (const mp4ff_t* f, int track, unsigned char** ppBuf, unsigned int* pBufSize);
int32_t mp4ff_get_track_type (const mp4ff_t* f, int track);
int32_t mp4ff_total_tracks (const mp4ff_t* f);

int32_t mp4ff_num_samples (const mp4ff_t* f, int track);
int32_t mp4ff_time_scale (const mp4ff_t* f, int track);

uint32_t mp4ff_get_avg_bitrate (const mp4ff_t* f, int track);
uint32_t mp4ff_get_max_bitrate (const mp4ff_t* f, int track);

int64_t mp4ff_get_track_duration (const mp4ff_t* f, int track); //returns (-1) if unknown
int64_t mp4ff_get_track_duration_use_offsets (const mp4ff_t* f, int track); //returns (-1) if unknown

uint32_t mp4ff_get_sample_rate (const mp4ff_t* f, int track);
uint32_t mp4ff_get_channel_count (const mp4ff_t*  f, int track);
uint32_t mp4ff_get_audio_type (const mp4ff_t*  f, int track);

// metadata
int mp4ff_meta_get_num_items (const mp4ff_t* f);
int mp4ff_meta_get_by_index (const mp4ff_t* f, unsigned int index, char** item, char** value);
int mp4ff_meta_get_title (const mp4ff_t* f, char** value);
int mp4ff_meta_get_artist (const mp4ff_t* f, char** value);
int mp4ff_meta_get_writer (const mp4ff_t* f, char** value);
int mp4ff_meta_get_album (const mp4ff_t* f, char** value);
int mp4ff_meta_get_date (const mp4ff_t* f, char** value);
int mp4ff_meta_get_tool (const mp4ff_t* f, char** value);
int mp4ff_meta_get_comment (const mp4ff_t* f, char** value);
int mp4ff_meta_get_genre (const mp4ff_t* f, char** value);
int mp4ff_meta_get_track (const mp4ff_t* f, char** value);
int mp4ff_meta_get_disc (const mp4ff_t* f, char** value);
int mp4ff_meta_get_totaltracks (const mp4ff_t* f, char** value);
int mp4ff_meta_get_totaldiscs (const mp4ff_t* f, char** value);
int mp4ff_meta_get_compilation (const mp4ff_t* f, char** value);
int mp4ff_meta_get_tempo (const mp4ff_t* f, char** value);
int32_t mp4ff_meta_get_coverart (const mp4ff_t* f, char** value);

int32_t mp4ff_meta_update (mp4ff_callback_t* f, const mp4ff_metadata_t* data);

//{{{
#ifdef __cplusplus
}
#endif
//}}}
