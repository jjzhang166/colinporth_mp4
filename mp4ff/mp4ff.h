#pragma once
#include <stdint.h>
//{{{
#ifdef __cplusplus
extern "C" {
#endif
//}}}

//{{{  track defines
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
//{{{  struct  mp4ff_metadata_t
typedef struct {
  mp4ff_tag_t* tags;
  uint32_t count;
  } mp4ff_metadata_t;
//}}}
typedef void* mp4ff_t;

mp4ff_t* mp4ff_open_read (mp4ff_callback_t* f);
mp4ff_t* mp4ff_open_read_metaonly (mp4ff_callback_t* f);
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
