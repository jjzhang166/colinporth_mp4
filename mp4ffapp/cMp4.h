// cMp4.h
#pragma once
#include <stdint.h>
//{{{  track defines
#define TRACK_UNKNOWN 0

#define TRACK_AUDIO   1
#define TRACK_VIDEO   2
#define TRACK_SYSTEM  3
//}}}

class cMp4 {
public:
  cMp4 (FILE* file, bool debug);
  ~cMp4();

  int32_t getNumTracks() { return numTracks; }

  int32_t get_track_type (int track) { return tracks[track]->type; }
  int32_t get_time_scale (int track) { return tracks[track]->timeScale; }

  uint32_t get_avg_bitrate (int track) { return tracks[track]->avgBitrate; }
  uint32_t get_max_bitrate (int track) { return tracks[track]->maxBitrate; }

  int64_t get_track_duration (int track) { return tracks[track]->duration; }
  uint32_t get_audio_type (int track) { return tracks[track]->audioType; }
  uint32_t get_sample_rate (int track) { return tracks[track]->sampleRate; }
  uint32_t get_channel_count (int track) { return tracks[track]->channelCount; }

  int32_t get_num_samples (int track);
  int64_t get_track_duration_use_offsets (int track);
  int32_t get_decoder_config (int track, unsigned char** ppBuf, unsigned int* pBufSize);

  int32_t get_sample_duration (int track, int sample);
  int32_t get_sample_duration_use_offsets (int track, int sample);
  int64_t get_sample_position (int track, int sample);
  int32_t get_sample_offset (int track, int sample);

  int32_t find_sample (int track, int64_t offset, int32_t* toskip);
  int32_t find_sample_use_offsets (int track, int64_t offset, int32_t* toskip);

  int32_t read_sample_size (int track, int sample);
  int32_t read_sample (int track, int sample, uint8_t* buffer);

  //{{{  get metadata
  //{{{  struct tag_t
  typedef struct {
    char* item;
    char* value;
    } tag_t;
  //}}}
  //{{{  struct metadata_t
  typedef struct {
    tag_t* tags;
    uint32_t count;
    } metadata_t;
  //}}}

  int meta_get_num_items();
  int meta_get_by_index (unsigned int index, char** item, char** value);
  int meta_get_title (char** value);
  int meta_get_artist (char** value);
  int meta_get_writer (char** value);
  int meta_get_album (char** value);
  int meta_get_date (char** value);
  int meta_get_tool (char** value);
  int meta_get_comment (char** value);
  int meta_get_genre (char** value);
  int meta_get_track (char** value);
  int meta_get_disc (char** value);
  int meta_get_totaltracks (char** value);
  int meta_get_totaldiscs (char** value);
  int meta_get_compilation (char** value);
  int meta_get_tempo (char** value);
  int32_t meta_get_coverart (char** value);

  int32_t meta_update (const metadata_t* data);
  //}}}

private:
  #define MAX_TRACKS 16
  //{{{  struct track_t
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
    } track_t;
  //}}}
  //{{{  struct membuffer
  typedef struct {
    void* data;
    unsigned written;
    unsigned allocated;
    unsigned error;
    } membuffer;
  //}}}

  //{{{  read, write
  int64_t getPosition();
  int32_t setPosition (const uint64_t position);

  int32_t read_data (uint8_t* data, uint32_t size);

  uint64_t read_int64();
  uint32_t read_int32();
  uint32_t read_int24();
  uint16_t read_int16();

  uint8_t read_char();
  char* read_string (uint32_t length);

  uint32_t read_mp4_descr_length();

  uint64_t readAtomHeader (uint8_t* atom_type, uint8_t* header_size, int indent);

  // write
  int32_t truncate();
  int32_t write_data (uint8_t* data, uint32_t size);
  int32_t write_int32 (const uint32_t data);
  //}}}
  //{{{  meta
  int32_t tag_add_field (metadata_t* tags, const char* item, const char* value);
  int32_t tag_set_field (metadata_t* tags, const char* item, const char* value);

  int32_t set_metadata_name (uint8_t atom_type, char** name);
  const char* meta_index_to_genre (uint32_t idx);

  int32_t parse_tag (uint8_t parent_atom_type, int32_t size);
  int32_t parse_metadata (int32_t size, int indent);
  int32_t meta_find_by_name (const char* item, char** value);

  // membuffer
  unsigned membuffer_write (membuffer* buf,const void* ptr,unsigned bytes);
  unsigned membuffer_write_int32 (membuffer* buf, uint32_t data);
  unsigned membuffer_write_int24 (membuffer* buf, uint32_t data);
  unsigned membuffer_write_int16 (membuffer* buf, uint16_t data);

  unsigned membuffer_write_atom_name (membuffer* buf, const char* data);
  void membuffer_write_atom (membuffer* buf, const char* name, unsigned size, const void* data);

  unsigned membuffer_write_string (membuffer* buf, const char* data);
  unsigned membuffer_write_int8 (membuffer* buf, uint8_t data);

  void* membuffer_get_ptr (const membuffer* buf);
  unsigned membuffer_get_size (const membuffer* buf);

  unsigned membuffer_error (const membuffer* buf);
  void membuffer_set_error (membuffer* buf);

  unsigned membuffer_transfer_from_file (membuffer* buf, unsigned bytes);

  membuffer* membuffer_create();
  void membuffer_free (membuffer* buf);
  void* membuffer_detach (membuffer* buf);

  const char* find_standard_meta (const char* name);

  void membuffer_write_track_tag (membuffer* buf, const char* name, uint32_t index, uint32_t total);
  void membuffer_write_int16_tag (membuffer* buf, const char* name, uint16_t value);
  void membuffer_write_std_tag (membuffer* buf, const char* name, const char* value);
  void membuffer_write_custom_tag (membuffer* buf, const char* name, const char* value);

  // more meta
  uint32_t meta_genre_to_index (const char* genrestr);
  uint32_t create_ilst (const metadata_t* data,void ** out_buffer, uint32_t* out_size);

  uint32_t find_atom (uint64_t base, uint32_t size, const char* name);
  uint32_t find_atom_v2 (uint64_t base, uint32_t size, const char* name,
                                uint32_t extraheaders, const char* name_inside);

  uint32_t create_meta (const metadata_t * data, void** out_buffer, uint32_t* out_size);
  uint32_t create_udta (const metadata_t * data, void** out_buffer, uint32_t* out_size);
  uint32_t modify_moov ( const metadata_t* data, uint8_t** out_buffer, uint32_t* out_size);
  //}}}
  //{{{  sample
  int32_t chunk_of_sample (int track, int sample, int32_t* chunk_sample, int32_t *chunk);
  int32_t chunk_to_offset (int track, int32_t chunk);
  int32_t sample_range_size (int track, int32_t chunk_sample, int sample);
  int32_t sample_to_offset (int track, int sample);
  int32_t audio_frame_size (int track, int sample);
  int32_t set_sample_position (int track, int sample);
  //}}}
  //{{{  atom
  int32_t read_stsz();
  int32_t read_esds();
  int32_t read_stsc();
  int32_t read_stco();
  int32_t read_ctts();
  int32_t read_stts();
  int32_t read_mvhd();
  int32_t read_tkhd();
  int32_t read_mdhd();

  int32_t read_mp4a (int indent);
  int32_t read_meta (uint64_t size, int indent);
  int32_t read_stsd (int indent);

  int32_t atom_read (int32_t size, uint8_t atom_type, int indent);

  int32_t parseSubAtoms (const uint64_t total_size, int indent);
  int32_t parseAtoms();
  //}}}

  // vars
  bool debug = false;
  FILE* file = nullptr;
  uint64_t file_size = 0;
  int64_t current_position = 0;

  int32_t moov_read = 0;
  uint64_t moov_offset = 0;
  uint64_t moov_size = 0;
  uint8_t last_atom = 0;

  int32_t duration = 0;
  int32_t time_scale = 0;

  int32_t numTracks = 0;
  track_t* tracks[MAX_TRACKS];

  metadata_t tags = { 0,0 };
  };
