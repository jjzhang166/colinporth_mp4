// cMp4.cpp
//{{{  includes
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cMp4.h"
//}}}
//{{{  const
#define COPYRIGHT_SYMBOL 0xA9

//{{{
enum eAtomType {
  ATOM_MOOV, ATOM_TRAK,
  ATOM_EDTS, ATOM_MDIA, ATOM_MINF, ATOM_STBL, ATOM_UDTA,
  ATOM_ILST, ATOM_TITLE, ATOM_ARTIST, ATOM_WRITER, ATOM_ALBUM, ATOM_DATE, ATOM_TOOL,
  ATOM_COMMENT, ATOM_GENRE1, ATOM_TRACK, ATOM_DISC, ATOM_COMPILATION, ATOM_GENRE2,
  ATOM_TEMPO, ATOM_COVER, ATOM_DRMS, ATOM_SINF, ATOM_SCHI,
  ATOM_MOOF, ATOM_TRAF,

  SUBATOMIC,
  ATOM_FTYP, ATOM_MDAT,
  ATOM_MVHD, ATOM_TKHD, ATOM_TREF,
  ATOM_MDHD, ATOM_VMHD, ATOM_SMHD, ATOM_HMHD,
  ATOM_STSD, ATOM_STTS, ATOM_STSZ, ATOM_STZ2, ATOM_STCO, ATOM_STSC,
  ATOM_MP4A, ATOM_MP4V, ATOM_AVC1, ATOM_MP4S,
  ATOM_ESDS,
  ATOM_META, ATOM_NAME, ATOM_DATA, ATOM_CTTS, ATOM_FRMA, ATOM_IVIV, ATOM_PRIV, ATOM_USER, ATOM_KEY,
  ATOM_ALBUM_ARTIST, ATOM_CONTENTGROUP, ATOM_LYRICS, ATOM_DESCRIPTION, ATOM_NETWORK, ATOM_SHOW,
  ATOM_EPISODENAME, ATOM_SORTTITLE, ATOM_SORTALBUM, ATOM_SORTARTIST, ATOM_SORTALBUMARTIST, ATOM_SORTWRITER,
  ATOM_SORTSHOW, ATOM_SEASON, ATOM_EPISODE, ATOM_PODCAST,
  ATOM_TFHD, ATOM_TFHT, ATOM_MVEX, ATOM_TREX,

  ATOM_UNKNOWN };
//}}}
//{{{  struct atomLookup_t
typedef struct {
  const char* atomName;
  const eAtomType atomType;
  } atomLookup_t;
//}}}
//{{{
const atomLookup_t kAtomLookup[] = {
  "moof", ATOM_MOOF,
  "tfaf", ATOM_TRAF,
  "tfhd", ATOM_TFHD,
  "tfht", ATOM_TFHT,
  "mvex", ATOM_MVEX,
  "trex", ATOM_TREX,

  "moov", ATOM_MOOV,
  "minf", ATOM_MINF,
  "mdia", ATOM_MDIA,
  "mdat", ATOM_MDAT,
  "mdhd", ATOM_MDHD,
  "mvhd", ATOM_MVHD,
  "mp4s", ATOM_MP4S,
  "mp4a", ATOM_MP4A,
  "mp4v", ATOM_MP4V,
  "avc1", ATOM_AVC1,
  "meta", ATOM_META,
  "trak", ATOM_TRAK,
  "tkhd", ATOM_TKHD,
  "tref", ATOM_TREF,
  "trkn", ATOM_TRACK,
  "tmpo", ATOM_TEMPO,
  "tvnn", ATOM_NETWORK,
  "tvsh", ATOM_SHOW,
  "tven", ATOM_EPISODENAME,
  "tvsn", ATOM_SEASON,
  "tves", ATOM_EPISODE,
  "stbl", ATOM_STBL,
  "smhd", ATOM_SMHD,
  "stsd", ATOM_STSD,
  "stts", ATOM_STTS,
  "stco", ATOM_STCO,
  "stsc", ATOM_STSC,
  "stsz", ATOM_STSZ,
  "stz2", ATOM_STZ2,
  "sinf", ATOM_SINF,
  "schi", ATOM_SCHI,
  "sonm", ATOM_SORTTITLE,
  "soal", ATOM_SORTALBUM,
  "soar", ATOM_SORTARTIST,
  "soaa", ATOM_SORTALBUMARTIST,
  "soco", ATOM_SORTWRITER,
  "sosn", ATOM_SORTSHOW,
  "\xA9" "nam", ATOM_TITLE,
  "\xA9" "ART", ATOM_ARTIST,
  "\xA9" "wrt", ATOM_WRITER,
  "\xA9" "alb", ATOM_ALBUM,
  "\xA9" "day", ATOM_DATE,
  "\xA9" "too", ATOM_TOOL,
  "\xA9" "cmt", ATOM_COMMENT,
  "\xA9" "gen", ATOM_GENRE1,
  "\xA9" "grp", ATOM_CONTENTGROUP,
  "\xA9" "lyr", ATOM_LYRICS,
  "edts", ATOM_EDTS,
  "esds", ATOM_ESDS,
  "ftyp", ATOM_FTYP,
  "hmhd", ATOM_HMHD,
  "vmhd", ATOM_VMHD,
  "udta", ATOM_UDTA,
  "ilst", ATOM_ILST,
  "name", ATOM_NAME,
  "data", ATOM_DATA,
  "disk", ATOM_DISC,
  "gnre", ATOM_GENRE2,
  "covr", ATOM_COVER,
  "cpil", ATOM_COMPILATION,
  "ctts", ATOM_CTTS,
  "drms", ATOM_DRMS,
  "frma", ATOM_FRMA,
  "priv", ATOM_PRIV,
  "iviv", ATOM_IVIV,
  "user", ATOM_USER,
  "key ", ATOM_KEY,
  "aART", ATOM_ALBUM_ARTIST,
  "desc", ATOM_DESCRIPTION,
  "pcst", ATOM_PODCAST,
  "skip", ATOM_UNKNOWN,
  "free", ATOM_UNKNOWN,
  };
//}}}

//{{{  struct standardMetaItem
typedef struct {
  const char* atom;
  const char* name;
  } standardMetaItem_t;
//}}}
//{{{
const standardMetaItem_t kStandardMetaItems[] = {
  {"\xA9" "nam", "title"},
  {"\xA9" "ART", "artist"},
  {"\xA9" "wrt", "writer"},
  {"\xA9" "alb", "album"},
  {"\xA9" "day", "date"},
  {"\xA9" "too", "tool"},
  {"\xA9" "cmt", "comment"},
  {"cpil", "compilation"},
  {"covr", "cover"},
  {"aART", "album_artist"},
  };
//}}}

//{{{
const char* kTagNames[] = {
    "unknown", "title", "artist", "writer", "album",
    "date", "tool", "comment", "genre", "track",
    "disc", "compilation", "genre", "tempo", "cover",
    "album_artist", "contentgroup", "lyrics", "description",
    "network", "show", "episodename",
    "sorttitle", "sortalbum", "sortartist", "sortalbumartist",
    "sortwriter", "sortshow",
    "season", "episode", "podcast"
    };
//}}}
//{{{
const char* kID3v1GenreList[] = {
    "Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk",
    "Grunge", "Hip-Hop", "Jazz", "Metal", "New Age", "Oldies",
    "Other", "Pop", "R&B", "Rap", "Reggae", "Rock",
    "Techno", "Industrial", "Alternative", "Ska", "Death Metal", "Pranks",
    "Soundtrack", "Euro-Techno", "Ambient", "Trip-Hop", "Vocal", "Jazz+Funk",
    "Fusion", "Trance", "Classical", "Instrumental", "Acid", "House",
    "Game", "Sound Clip", "Gospel", "Noise", "AlternRock", "Bass",
    "Soul", "Punk", "Space", "Meditative", "Instrumental Pop", "Instrumental Rock",
    "Ethnic", "Gothic", "Darkwave", "Techno-Industrial", "Electronic", "Pop-Folk",
    "Eurodance", "Dream", "Southern Rock", "Comedy", "Cult", "Gangsta",
    "Top 40", "Christian Rap", "Pop/Funk", "Jungle", "Native American", "Cabaret",
    "New Wave", "Psychadelic", "Rave", "Showtunes", "Trailer", "Lo-Fi",
    "Tribal", "Acid Punk", "Acid Jazz", "Polka", "Retro", "Musical",
    "Rock & Roll", "Hard Rock", "Folk", "Folk/Rock", "National Folk", "Swing",
    "Fast-Fusion", "Bebob", "Latin", "Revival", "Celtic", "Bluegrass", "Avantgarde",
    "Gothic Rock", "Progressive Rock", "Psychedelic Rock", "Symphonic Rock", "Slow Rock", "Big Band",
    "Chorus", "Easy Listening", "Acoustic", "Humour", "Speech", "Chanson",
    "Opera", "Chamber Music", "Sonata", "Symphony", "Booty Bass", "Primus",
    "Porn Groove", "Satire", "Slow Jam", "Club", "Tango", "Samba",
    "Folklore", "Ballad", "Power Ballad", "Rhythmic Soul", "Freestyle", "Duet",
    "Punk Rock", "Drum Solo", "A capella", "Euro-House", "Dance Hall",
    "Goa", "Drum & Bass", "Club House", "Hardcore", "Terror",
    "Indie", "BritPop", "NegerPunk", "Polsk Punk", "Beat",
    "Christian Gangsta", "Heavy Metal", "Black Metal", "Crossover", "Contemporary C",
    "Christian Rock", "Merengue", "Salsa", "Thrash Metal", "Anime", "JPop",
    "SynthPop",
};
//}}}
//}}}

// utils
//{{{
uint32_t myatoi (const char* param) {
  return param ? atoi(param) : 0;
  }
//}}}
//{{{
uint16_t fixByteOrder16 (uint16_t src) {

  uint8_t data[2];
  memcpy (data, &src, sizeof(src));

  uint16_t a = data[0];
  uint16_t b = data[1];

  return (a<<8) | b;
  }
//}}}
//{{{
uint32_t fixByteOrder32 (uint32_t src) {

  uint8_t data[4];
  memcpy (data, &src, sizeof(src));

  uint32_t a = data[0];
  uint32_t b = data[1];
  uint32_t c = data[2];
  uint32_t d = data[3];

  return (a<<24) | (b<<16) | (c<<8) | d;
  }
//}}}

// cMp4
//{{{
cMp4::cMp4 (FILE* file, bool debug) {

  this->debug = debug;;
  this->file = file;

  parseAtoms();
  }
//}}}
//{{{
cMp4::~cMp4() {

  for (int i = 0; i < numTracks; i++)
    if (tracks[i]) {
      free (tracks[i]->stsz_table);
      free (tracks[i]->stts_sample_count);
      free (tracks[i]->stts_sample_delta);
      free (tracks[i]->stsc_first_chunk);
      free (tracks[i]->stsc_samples_per_chunk);
      free (tracks[i]->stsc_sample_desc_index);
      free (tracks[i]->stco_chunk_offset);
      free (tracks[i]->decoderConfig);
      free (tracks[i]->ctts_sample_count);
      free (tracks[i]->ctts_sample_offset);
      free (tracks[i]);
      }

  for (unsigned int i = 0; i < tags.count; i++) {
    free (tags.tags[i].item);
    free (tags.tags[i].value);
    }

  free (tags.tags);
  }
//}}}

//{{{
int32_t cMp4::getAudioTrack() {

  int32_t bestTrack = -1;
  for (auto track = 0; track < numTracks; track++)
    if (tracks[track]->type == eTrackAudio)
      bestTrack = track;

  return bestTrack;
  }
//}}}
//{{{
int32_t cMp4::getVideoTrack() {

  int32_t bestTrack = -1;
  for (auto track = 0; track < numTracks; track++)
    if (tracks[track]->type == eTrackVideo)
      bestTrack = track;

  return bestTrack;
  }
//}}}

// track gets
//{{{
int32_t cMp4::getNumSamples (int track) {

  int32_t total = 0;
  for (int i = 0; i < tracks[track]->stts_entry_count; i++)
    total += tracks[track]->stts_sample_count[i];

  return total;
  }
//}}}
//{{{
int32_t cMp4::getDecoderConfig (int track, uint8_t** buffer, uint32_t* bufferSize) {

  if (track >= numTracks) {
    *buffer = NULL;
    *bufferSize = 0;
    return 1;
    }

  if (tracks[track]->decoderConfig == NULL || tracks[track]->decoderConfigLen == 0) {
    *buffer = NULL;
    *bufferSize = 0;
    }

  else {
    *buffer = (uint8_t*)malloc (tracks[track]->decoderConfigLen);
    if (*buffer == NULL) {
      *bufferSize = 0;
      return 1;
      }

    memcpy (*buffer, tracks[track]->decoderConfig, tracks[track]->decoderConfigLen);
    *bufferSize = tracks[track]->decoderConfigLen;
    }

  return 0;
  }
//}}}

//{{{
int32_t cMp4::getSampleDuration (int track, int sample) {

  int32_t co = 0;

  for (int32_t i = 0; i < tracks[track]->stts_entry_count; i++) {
    int32_t delta = tracks[track]->stts_sample_count[i];
    if (sample < co + delta)
      return tracks[track]->stts_sample_delta[i];
    co += delta;
    }

  return (int32_t)(-1);
  }
//}}}
//{{{
int64_t cMp4::getSamplePosition (int track, int sample) {

  int32_t co = 0;
  int64_t acc = 0;

  for (int i = 0; i < tracks[track]->stts_entry_count; i++) {
    int32_t delta = tracks[track]->stts_sample_count[i];
    if (sample < co + delta) {
      acc += tracks[track]->stts_sample_delta[i] * (sample - co);
      return acc;
      }
    else
      acc += tracks[track]->stts_sample_delta[i] * delta;
    co += delta;
    }

  return (int64_t)(-1);
  }
//}}}
//{{{
int32_t cMp4::getSampleOffset (int track, int sample) {

  int32_t co = 0;
  for (int32_t i = 0; i < tracks[track]->ctts_entry_count; i++) {
    int32_t delta = tracks[track]->ctts_sample_count[i];
    if (sample < co + delta)
      return tracks[track]->ctts_sample_offset[i];
    co += delta;
    }

  return 0;
  }
//}}}
//{{{
int32_t cMp4::find_sample (int track, int64_t offset, int32_t* toskip) {

  int32_t co = 0;
  int64_t offset_total = 0;

  for (int32_t i = 0; i < tracks[track]->stts_entry_count; i++) {
    int32_t sample_count = tracks[track]->stts_sample_count[i];
    int32_t sample_delta = tracks[track]->stts_sample_delta[i];
    int64_t offset_delta = (int64_t)sample_delta * (int64_t)sample_count;
    if (offset < offset_total + offset_delta) {
      int64_t offset_fromstts = offset - offset_total;
      if (toskip)
        *toskip = (int32_t)(offset_fromstts % sample_delta);
      return co + (int32_t)(offset_fromstts / sample_delta);
      }
    else
      offset_total += offset_delta;
    co += sample_count;
    }

  return (int32_t)(-1);
  }
//}}}

//{{{
int64_t cMp4::getTrackDurationUseOffsets (int track) {

  int64_t duration = getTrackDuration (track);
  if (duration != -1) {
    int64_t offset = getSampleOffset (track, 0);
    if (offset > duration)
      duration = 0;
    else
      duration -= offset;
    }

  return duration;
  }
//}}}
//{{{
int32_t cMp4::getSampleDurationUseOffsets (int track, int sample) {

  int32_t duration = getSampleDuration (track, sample);

  if (duration != -1) {
    int32_t offset = getSampleOffset (track, sample);
    if (offset > duration)
      duration = 0;
    else
      duration -= offset;
    }

  return duration;
  }
//}}}
//{{{
int32_t cMp4::find_sample_use_offsets (int track, int64_t offset, int32_t* toskip) {
  return find_sample (track, offset + getSampleOffset (track, 0), toskip);
  }
//}}}

//{{{
uint32_t cMp4::getSampleSize (int track, int sample) {
  return tracks[track]->stsz_sample_size ? tracks[track]->stsz_sample_size : tracks[track]->stsz_table[sample];
  }
//}}}
//{{{
uint32_t cMp4::getSample (int track, int sample, uint8_t* buffer, uint32_t size) {

  setPosition (sample_to_offset (track, sample));
  return readData (buffer, size);
  }
//}}}

//{{{  meta gets
//{{{
int32_t cMp4::meta_get_title (char** value) {
  return meta_find_by_name ("title", value);
  }
//}}}
//{{{
int32_t cMp4::meta_get_artist (char** value) {
  return meta_find_by_name ("artist", value);
  }
//}}}
//{{{
int32_t cMp4::meta_get_writer (char** value) {
  return meta_find_by_name ("writer", value);
  }
//}}}
//{{{
int32_t cMp4::meta_get_album (char** value) {
  return meta_find_by_name ("album", value);
  }
//}}}
//{{{
int32_t cMp4::meta_get_date (char** value) {
  return meta_find_by_name ("date", value);
  }
//}}}
//{{{
int32_t cMp4::meta_get_tool (char** value) {
  return meta_find_by_name ("tool", value);
  }
//}}}
//{{{
int32_t cMp4::meta_get_comment (char** value) {
  return meta_find_by_name ("comment", value);
  }
//}}}
//{{{
int32_t cMp4::meta_get_genre (char** value) {
  return meta_find_by_name ("genre", value);
  }
//}}}
//{{{
int32_t cMp4::meta_get_track (char** value) {
  return meta_find_by_name ("track", value);
  }
//}}}
//{{{
int32_t cMp4::meta_get_totaltracks (char** value) {
  return meta_find_by_name ("totaltracks", value);
  }
//}}}
//{{{
int32_t cMp4::meta_get_disc (char** value) {
  return meta_find_by_name ("disc", value);
  }
//}}}
//{{{
int32_t cMp4::meta_get_totaldiscs (char** value) {
  return meta_find_by_name ("totaldiscs", value);
  }
//}}}
//{{{
int32_t cMp4::meta_get_compilation (char** value) {
  return meta_find_by_name ("compilation", value);
  }
//}}}
//{{{
int32_t cMp4::meta_get_tempo (char** value) {
  return meta_find_by_name ("tempo", value);
  }
//}}}
//{{{
int32_t cMp4::meta_get_coverart (char** value) {
  return meta_find_by_name ("cover", value);
  }
//}}}
//{{{
int32_t cMp4::meta_get_by_index (uint32_t index, char** item, char** value) {

  if (index >= tags.count) {
    *item = NULL;
    *value = NULL;
    return 0;
    }
  else {
    *item = _strdup (tags.tags[index].item);
    *value = _strdup (tags.tags[index].value);
    return 1;
    }
  }
//}}}

//{{{
int32_t cMp4::meta_get_num_items() {
  return tags.count;
  }
//}}}
//{{{
int32_t cMp4::meta_update (const metadata_t* data) {

  setPosition (0);
  parseAtoms();

  uint8_t* new_moov_data;
  uint32_t new_moov_size;
  if (!modify_moov (data, &new_moov_data, &new_moov_size))
    return 0;

  /* copy moov atom to end of the file */
  if (last_atom != ATOM_MOOV) {
    char* free_data = "free";

    /* rename old moov to free */
    setPosition (moov_offset + 4);
    writeData((uint8_t*)free_data, 4);

    setPosition (file_size);
    writeInt32 (new_moov_size + 8);
    writeData((uint8_t*)"moov",4);
    writeData(new_moov_data, new_moov_size);
    }

  else {
    setPosition (moov_offset);
    writeInt32 (new_moov_size + 8);
    writeData((uint8_t*)"moov",4);
    writeData(new_moov_data, new_moov_size);
    }

  truncate();

  return 1;
  }
//}}}
//}}}

// private
//{{{  seek, read, write
int64_t cMp4::getPosition() { return current_position; }
//{{{
int32_t cMp4::setPosition (const uint64_t position) {

  fseek (file, (long)position, SEEK_SET);
  current_position = position;
  return 0;
  }
//}}}

//{{{
uint32_t cMp4::readData (uint8_t* buffer, uint32_t size) {

  size_t result = fread (buffer, 1, size, file);
  current_position += size;
  return (uint32_t)result;
  }
//}}}
//{{{
uint8_t cMp4::readChar() {

  uint8_t output;
  readData (&output, 1);
  return output;
  }
//}}}
//{{{
uint16_t cMp4::readInt16() {

  uint8_t data[2];
  readData (data, 2);
  return (data[0]<<8) | data[1];
  }
//}}}
//{{{
uint32_t cMp4::readInt24() {

  uint8_t data[3];
  readData (data, 3);
  return (data[0]<<16) | (data[1]<<8) | data[2];
  }
//}}}
//{{{
uint32_t cMp4::readInt32() {

  uint8_t data[4];
  readData (data, 4);
  return (data[0]<<24) | (data[1]<<16) | (data[2]<<8) | data[3];
  }
//}}}
//{{{
uint64_t cMp4::readInt64() {

  uint8_t data[8];
  readData (data, 8);

  uint64_t result = 0;
  for (int i = 0; i < 8; i++)
    result |= ((uint64_t)data[i]) << ((7 - i) * 8);

  return result;
  }
//}}}
//{{{
uint32_t cMp4::readMp4DescrLength() {

  uint8_t b;
  uint8_t numBytes = 0;
  uint32_t length = 0;

  do {
    b = readChar();
    numBytes++;
    length = (length << 7) | (b & 0x7F);
    } while ((b & 0x80) && numBytes < 4);

  return length;
  }
//}}}
//{{{
char* cMp4::readString (uint32_t length) {

  auto str = (uint8_t*)malloc (length + 1);
  if ((uint32_t)readData (str, length) != length) {
    free (str);
    str = nullptr;
    }

  return (char*)str;
  }
//}}}
//{{{
uint64_t cMp4::readAtomHeader (uint8_t* atom_type, uint8_t* header_size, int indent)  {
// read atom header, return atom size, atom size is with header included

  if (debug)
    printf ("%8lld ", getPosition());

  uint8_t atom_header[9];
  int32_t ret = readData (atom_header, 8);
  if (ret != 8)
    return 0;

  // get atom size
  uint64_t size = (atom_header[0]) << 24;
  size |= (atom_header[1]) << 16;
  size |= (atom_header[2]) << 8;
  size |= atom_header[3];

  if (size == 1) {
    // 64 bit atom size
    *header_size = 16;
    size = readInt64();
    }
  else
    *header_size = 8;

  if (debug) {
    printf ("%8lld ", size);
    for (int i= 0; i < indent; i++)
      printf ("  ");
    printf ("%c%c%c%c\n", atom_header[4], atom_header[5], atom_header[6], atom_header[7]);
    }

  *atom_type = ATOM_UNKNOWN;
  atom_header[8] = 0;
  for (unsigned int n = 0; n < sizeof (kAtomLookup) / sizeof(atomLookup_t); n++)
    if (!_stricmp ((char*)&atom_header[4], kAtomLookup[n].atomName)) {
      *atom_type = kAtomLookup[n].atomType;
      break;
      }

  return size;
  }
//}}}

// write
//{{{
int32_t cMp4::truncate() {

  return 0;
  //return truncate (file);
  }
//}}}
//{{{
int32_t cMp4::writeData (uint8_t* data, uint32_t size) {

  int32_t result = 1;
  //result = write (file, data, size);
  result = 0;
  current_position += size;
  return result;
  }
//}}}
//{{{
int32_t cMp4::writeInt32 (const uint32_t data) {

  int8_t temp[4];
  *(uint32_t*)temp = data;
  uint32_t a = temp[0];
  uint32_t b = temp[1];
  uint32_t c = temp[2];
  uint32_t d = temp[3];

  uint32_t result = (a<<24) | (b<<16) | (c<<8) | d;
  return writeData ((uint8_t*)&result, sizeof (result));
  }
//}}}
//}}}
//{{{  membuffer
//{{{
cMp4::membuffer_t* cMp4::membuffer_create() {

  const unsigned initial_size = 256;
  membuffer_t* buf = (membuffer_t*)malloc(sizeof(membuffer_t));
  buf->data = malloc(initial_size);
  buf->written = 0;
  buf->allocated = initial_size;
  buf->error = buf->data == 0 ? 1 : 0;

  return buf;
  }
//}}}
//{{{
void cMp4::membuffer_free (membuffer_t* buf) {
  free (buf->data);
  free (buf);
  }
//}}}

//{{{
void* cMp4::membuffer_detach (membuffer_t* buf) {

  if (buf->error)
    return 0;

  void* ret = realloc (buf->data, buf->written);
  if (ret == 0)
    free (buf->data);

  buf->data = 0;
  buf->error = 1;

  return ret;
  }
//}}}
unsigned cMp4::membuffer_error (const membuffer_t* buf) { return buf->error; }
void cMp4::membuffer_set_error (membuffer_t* buf) { buf->error = 1; }

void* cMp4::membuffer_get_ptr (const membuffer_t* buf) { return buf->data; }
unsigned cMp4::membuffer_get_size (const membuffer_t* buf) { return buf->written; }

//{{{
unsigned cMp4::membufferWrite (membuffer_t * buf,const void * ptr,unsigned bytes) {

  if (buf->error)
    return 0;

  unsigned dest_size = buf->written + bytes;
  if (dest_size > buf->allocated) {
    do {
      buf->allocated <<= 1;
      } while (dest_size > buf->allocated);

    {
      void* newptr = realloc (buf->data, buf->allocated);
      if (newptr == 0) {
        free (buf->data);
        buf->data = 0;
        buf->error = 1;
        return 0;
        }
      buf->data = newptr;
      }
    }

  if (ptr)
    memcpy ((char*)buf->data + buf->written,ptr,bytes);
  buf->written += bytes;

  return bytes;
  }
//}}}
//{{{
unsigned cMp4::membufferWriteInt32 (membuffer_t* buf, uint32_t data) {

  uint8_t temp[4] = {(uint8_t)(data>>24),(uint8_t)(data>>16),(uint8_t)(data>>8),(uint8_t)data};
  return membufferWrite (buf, temp, 4);
  }
//}}}
//{{{
unsigned cMp4::membufferWriteInt24 (membuffer_t* buf, uint32_t data) {

  uint8_t temp[3] = {(uint8_t)(data>>16),(uint8_t)(data>>8),(uint8_t)data};
  return membufferWrite (buf, temp, 3);
  }
//}}}
//{{{
unsigned cMp4::membufferWriteInt16 (membuffer_t* buf, uint16_t data) {

  uint8_t temp[2] = {(uint8_t)(data>>8),(uint8_t)data};
  return membufferWrite (buf, temp, 2);
  }
//}}}
//{{{
unsigned cMp4::membufferWriteInt8 (membuffer_t* buf, uint8_t data) {
  return membufferWrite (buf, &data, 1);
  }
//}}}
//{{{
unsigned cMp4::membufferWrite_string (membuffer_t* buf, const char* data) {
  return membufferWrite (buf, data, (uint32_t)strlen(data));
  }
//}}}

//{{{
unsigned cMp4::membufferWrite_atom_name (membuffer_t* buf, const char * data) {
  return membufferWrite (buf, data, 4) == 4 ? 1 : 0;
  }
//}}}
//{{{
void cMp4::membufferWrite_atom (membuffer_t* buf, const char* name, unsigned size, const void* data) {

  membufferWriteInt32 (buf, size + 8);
  membufferWrite_atom_name (buf, name);
  membufferWrite (buf, data, size);
  }
//}}}

//{{{
void cMp4::membufferWrite_track_tag (membuffer_t* buf, const char* name, uint32_t index, uint32_t total) {

  membufferWriteInt32 (buf,8 /*atom header*/ + 8 /*data atom header*/ + 8 /*flags + reserved*/ + 8 /*actual data*/ );
  membufferWrite_atom_name (buf,name);
  membufferWriteInt32 (buf,8 /*data atom header*/ + 8 /*flags + reserved*/ + 8 /*actual data*/ );
  membufferWrite_atom_name (buf,"data");
  membufferWriteInt32 (buf,0);//flags
  membufferWriteInt32 (buf,0);//reserved
  membufferWriteInt16 (buf,0);
  membufferWriteInt16 (buf,(uint16_t)index);//track number
  membufferWriteInt16 (buf,(uint16_t)total);//total tracks
  membufferWriteInt16 (buf,0);
  }
//}}}
//{{{
void cMp4::membufferWriteInt16_tag (membuffer_t* buf, const char* name, uint16_t value) {

  membufferWriteInt32 (buf, 8 /*atom header*/ + 8 /*data atom header*/ + 8 /*flags + reserved*/ + 2 /*actual data*/ );
  membufferWrite_atom_name (buf,name);
  membufferWriteInt32 (buf, 8 /*data atom header*/ + 8 /*flags + reserved*/ + 2 /*actual data*/ );
  membufferWrite_atom_name (buf, "data");
  membufferWriteInt32 (buf, 0);//flags
  membufferWriteInt32 (buf, 0);//reserved
  membufferWriteInt16 (buf, value);//value
  }
//}}}
//{{{
void cMp4::membufferWrite_std_tag (membuffer_t* buf, const char* name, const char* value) {
/* added by AJS */

  uint32_t flags = 1;
  /* special check for compilation flag */
  if ( strcmp(name, "cpil") == 0)
    flags = 21;

  membufferWriteInt32 (buf, 8 /*atom header*/ + 8 /*data atom header*/ + 8 /*flags + reserved*/ + (uint32_t)strlen(value) );
  membufferWrite_atom_name (buf,name);
  membufferWriteInt32 (buf, 8 /*data atom header*/ + 8 /*flags + reserved*/ + (uint32_t)strlen(value));
  membufferWrite_atom_name (buf,"data");
  membufferWriteInt32 (buf, flags);//flags
  membufferWriteInt32 (buf, 0);//reserved
  membufferWrite (buf, value, (uint32_t)strlen(value));
  }
//}}}

//{{{
unsigned cMp4::membuffer_transfer_from_file (membuffer_t* buf, unsigned bytes) {

  unsigned oldsize = membuffer_get_size (buf);
  if (membufferWrite (buf, 0, bytes) != bytes)
    return 0;

  void* bufptr = membuffer_get_ptr (buf);
  if (bufptr == 0)
    return 0;

  if ((unsigned)readData ((uint8_t*)bufptr + oldsize, bytes) != bytes) {
    membuffer_set_error (buf);
    return 0;
    }

  return bytes;
  }
//}}}
//}}}
//{{{  meta
//{{{
int32_t cMp4::tag_add_field (metadata_t* tags, const char* item, const char* value) {

  void* backup = (void *)tags->tags;

  if (!item || (item && !*item) || !value) return 0;

  tags->tags = (tag_t*)realloc(tags->tags, (tags->count+1) * sizeof(tag_t));
  if (!tags->tags) {
    free(backup);
    return 0;
    }
  else {
    tags->tags[tags->count].item = _strdup (item);
    tags->tags[tags->count].value = _strdup (value);

    if (!tags->tags[tags->count].item || !tags->tags[tags->count].value) {
      free (tags->tags[tags->count].item);
      free (tags->tags[tags->count].value);
      tags->tags[tags->count].item = NULL;
      tags->tags[tags->count].value = NULL;
      return 0;
      }

    tags->count++;
    return 1;
    }
  }
//}}}
//{{{
int32_t cMp4::tag_set_field (metadata_t* tags, const char* item, const char* value) {

  if (!item || (item && !*item) || !value)
    return 0;

  for (unsigned int i = 0; i < tags->count; i++) {
    if (!_stricmp (tags->tags[i].item, item)) {
      free(tags->tags[i].value);
      tags->tags[i].value = _strdup (value);
      return 1;
      }
    }

  return tag_add_field (tags, item, value);
  }
//}}}

//{{{
int32_t cMp4::set_metadata_name (uint8_t atom_type, char* *name) {

    uint8_t tag_idx = 0;

    switch (atom_type) {
    case ATOM_TITLE: tag_idx = 1; break;
    case ATOM_ARTIST: tag_idx = 2; break;
    case ATOM_WRITER: tag_idx = 3; break;
    case ATOM_ALBUM: tag_idx = 4; break;
    case ATOM_DATE: tag_idx = 5; break;
    case ATOM_TOOL: tag_idx = 6; break;
    case ATOM_COMMENT: tag_idx = 7; break;
    case ATOM_GENRE1: tag_idx = 8; break;
    case ATOM_TRACK: tag_idx = 9; break;
    case ATOM_DISC: tag_idx = 10; break;
    case ATOM_COMPILATION: tag_idx = 11; break;
    case ATOM_GENRE2: tag_idx = 12; break;
    case ATOM_TEMPO: tag_idx = 13; break;
    case ATOM_COVER: tag_idx = 14; break;
    case ATOM_ALBUM_ARTIST: tag_idx = 15; break;
    case ATOM_CONTENTGROUP: tag_idx = 16; break;
    case ATOM_LYRICS: tag_idx = 17; break;
    case ATOM_DESCRIPTION: tag_idx = 18; break;
    case ATOM_NETWORK: tag_idx = 19; break;
    case ATOM_SHOW: tag_idx = 20; break;
    case ATOM_EPISODENAME: tag_idx = 21; break;
    case ATOM_SORTTITLE: tag_idx = 22; break;
    case ATOM_SORTALBUM: tag_idx = 23; break;
    case ATOM_SORTARTIST: tag_idx = 24; break;
    case ATOM_SORTALBUMARTIST: tag_idx = 25; break;
    case ATOM_SORTWRITER: tag_idx = 26; break;
    case ATOM_SORTSHOW: tag_idx = 27; break;
    case ATOM_SEASON: tag_idx = 28; break;
    case ATOM_EPISODE: tag_idx = 29; break;
    case ATOM_PODCAST: tag_idx = 30; break;
    default: tag_idx = 0; break;
    }

  *name = _strdup (kTagNames[tag_idx]);

  return 0;
  }
//}}}
//{{{
const char* cMp4::meta_index_to_genre (uint32_t idx) {

  if (idx>0 && idx <= sizeof (kID3v1GenreList) / sizeof (kID3v1GenreList[0]))
    return kID3v1GenreList[idx-1];
  else
    return 0;
  }
//}}}

//{{{
int32_t cMp4::parse_tag (uint8_t parent_atom_type, int32_t size) {

  uint8_t atom_type;
  uint8_t header_size = 0;
  uint64_t subsize, sumsize = 0;
  char*  name = NULL;
  char*  data = NULL;
  uint32_t done = 0;

  while (sumsize < size) {
    uint64_t destpos;
    subsize = readAtomHeader (&atom_type, &header_size, 0);
    destpos = getPosition() + subsize-header_size;
    if (!done) {
      if (atom_type == ATOM_DATA) {
        readChar(); /* version */
        readInt24(); /* flags */
        readInt32(); /* reserved */

        /* some need special attention */
        if (parent_atom_type == ATOM_GENRE2 || parent_atom_type == ATOM_TEMPO) {
          if (subsize - header_size >= 8 + 2) {
            uint16_t val = readInt16();

            if (parent_atom_type == ATOM_TEMPO) {
              char temp[16];
              sprintf(temp, "%.5u BPM", val);
              tag_add_field(&(tags), "tempo", temp);
            }
            else {
              const char * temp = meta_index_to_genre (val);
              if (temp)
                tag_add_field (&(tags), "genre", temp);
              }
            done = 1;
            }
          }
        else if (parent_atom_type == ATOM_TRACK || parent_atom_type == ATOM_DISC) {
          /* if (!done && subsize - header_size >= 8 + 8) */
          /* modified by AJS */
          if ( !done && (subsize - header_size) >=
            (sizeof(char) + sizeof(uint8_t)*3 + sizeof(uint32_t) + /* version + flags + reserved */
            + sizeof(uint16_t) /* leading uint16_t */
            + sizeof(uint16_t) /* track / disc */
            + sizeof(uint16_t)) /* totaltracks / totaldiscs */
            )
          {
            uint16_t index,total;
            char temp[32];
            readInt16();
            index = readInt16();
            total = readInt16();
              /* modified by AJS */
            /* readInt16(); */

            sprintf (temp, "%d",index);
            tag_add_field (&(tags), parent_atom_type == ATOM_TRACK ? "track" : "disc", temp);
            if (total>0)
            {
              sprintf (temp, "%d",total);
              tag_add_field (&(tags), parent_atom_type == ATOM_TRACK ? "totaltracks" : "totaldiscs", temp);
            }
            done = 1;
            }
          }
        else {
          free (data);
          data = readString ((uint32_t)(subsize-(header_size+8)));
          }
        }
      else if (atom_type == ATOM_NAME) {
        if (!done) {
          readChar(); /* version */
          readInt24(); /* flags */
          if (name) free(name);
          name = readString ((uint32_t)(subsize-(header_size+4)));
          }
        }
      setPosition(destpos);
      sumsize += subsize;
      }
    }

  if (data) {
    if (!done) {
      if (name == NULL)
        set_metadata_name(parent_atom_type, &name);
      if (name)
        tag_add_field(&(tags), name, data);
      }

    free(data);
    }

  if (name)
    free(name);

  return 1;
  }
//}}}
//{{{
int32_t cMp4::parse_metadata (int32_t size, int indent) {

  uint64_t subsize, sumsize = 0;
  uint8_t atom_type;
  uint8_t header_size = 0;

  while (sumsize < size) {
    subsize = readAtomHeader(&atom_type, &header_size, indent);
    if (subsize == 0)
      break;
    parse_tag(atom_type, (uint32_t)(subsize-header_size));
    sumsize += subsize;
    }

  return 0;
  }
//}}}

//{{{
/* find a metadata item by name */
/* returns 0 if item found, 1 if no such item */
int32_t cMp4::meta_find_by_name (const char* item, char** value) {

  uint32_t i;

  for (i = 0; i < tags.count; i++) {
    if (!_stricmp(tags.tags[i].item, item)) {
      *value = _strdup(tags.tags[i].value);
      return 1;
      }
    }

  *value = NULL;

  /* not found */
  return 0;
  }
//}}}
//{{{
uint32_t cMp4::meta_genre_to_index (const char* genrestr) {

  unsigned n;
  for (n = 0; n < sizeof (kID3v1GenreList)/sizeof(kID3v1GenreList[0]); n++)
    if (!_stricmp (genrestr, kID3v1GenreList[n]))
      return n+1;
  return 0;
  }
//}}}

//{{{
uint32_t cMp4::find_atom (uint64_t base, uint32_t size, const char* name) {

  uint32_t remaining = size;
  uint64_t atom_offset = base;

  while (true) {
    uint8_t atom_name[4];
    uint32_t atom_size;

    setPosition (atom_offset);

    if (remaining < 8) break;
    atom_size = readInt32();
    if (atom_size > remaining || atom_size < 8) break;
    readData (atom_name,4);

    if (!memcmp(atom_name,name,4)) {
      setPosition (atom_offset);
      return 1;
      }

    remaining -= atom_size;
    atom_offset += atom_size;
    }

  return 0;
  }
//}}}
//{{{
uint32_t cMp4::find_atom_v2 (uint64_t base, uint32_t size, const char* name,
                              uint32_t extraheaders, const char* name_inside) {

  uint64_t first_base = (uint64_t)(-1);
  while (find_atom (base,size,name)) { //try to find atom <name> with atom <name_inside> in it
    uint64_t mybase = getPosition();
    uint32_t mysize = readInt32();

    if (first_base == (uint64_t)(-1))
      first_base = mybase;

    if (mysize < 8 + extraheaders)
      break;

    if (find_atom (mybase + (8 + extraheaders), mysize - (8 + extraheaders), name_inside)) {
      setPosition (mybase);
      return 2;
      }
    base += mysize;
    if (size <= mysize) {
      size = 0;
      break;
      }
    size -= mysize;
    }

  if (first_base != (uint64_t)(-1)) { //wanted atom inside not found
    setPosition (first_base);
    return 1;
    }
  else
    return 0;
  }
//}}}
//{{{
const char* cMp4::findStandardMetaAtom (const char* name) {
// returns atom name if found, 0 if not

  for (unsigned int n = 0; n < sizeof (kStandardMetaItems) / sizeof(standardMetaItem_t); n++)
    if (!_stricmp (name, kStandardMetaItems[n].name))
      return kStandardMetaItems[n].atom;
    return 0;
  }
//}}}

//{{{
uint32_t cMp4::create_ilst (const metadata_t* data, void** out_buffer, uint32_t* out_size) {

  membuffer_t* buf = membuffer_create();
  unsigned metaptr;
  char* mask = (char*)malloc (data->count);
  memset (mask,0,data->count);
  {
    const char * tracknumber_ptr = 0, * totaltracks_ptr = 0;
    const char * discnumber_ptr = 0, * totaldiscs_ptr = 0;
    const char * genre_ptr = 0, * tempo_ptr = 0;
    for (metaptr = 0; metaptr < data->count; metaptr++) {
      tag_t * tag = &data->tags[metaptr];
      if (!_stricmp(tag->item,"tracknumber") || !_stricmp(tag->item,"track")) {
        if (tracknumber_ptr==0)
          tracknumber_ptr = tag->value;
        mask[metaptr] = 1;
        }
      else if (!_stricmp(tag->item,"totaltracks")) {
        if (totaltracks_ptr==0)
          totaltracks_ptr = tag->value;
        mask[metaptr] = 1;
        }
      else if (!_stricmp(tag->item,"discnumber") || !_stricmp(tag->item,"disc")) {
        if (discnumber_ptr==0)
          discnumber_ptr = tag->value;
        mask[metaptr] = 1;
        }
      else if (!_stricmp(tag->item,"totaldiscs")) {
        if (totaldiscs_ptr==0)
          totaldiscs_ptr = tag->value;
        mask[metaptr] = 1;
        }
      else if (!_stricmp(tag->item,"genre")) {
        if (genre_ptr==0)
          genre_ptr = tag->value;
        mask[metaptr] = 1;
        }
      else if (!_stricmp(tag->item,"tempo")) {
        if (tempo_ptr==0)
          tempo_ptr = tag->value;
        mask[metaptr] = 1;
        }
      }

    if (tracknumber_ptr)
      membufferWrite_track_tag (buf, "trkn", myatoi (tracknumber_ptr), myatoi (totaltracks_ptr));
    if (discnumber_ptr)
      membufferWrite_track_tag (buf, "disk", myatoi (discnumber_ptr), myatoi (totaldiscs_ptr));
    if (tempo_ptr)
      membufferWriteInt16_tag (buf, "tmpo", (uint16_t)myatoi (tempo_ptr));

    if (genre_ptr) {
      uint32_t index = meta_genre_to_index (genre_ptr);
      if (index == 0)
        membufferWrite_std_tag (buf, "©gen", genre_ptr);
      else
        membufferWriteInt16_tag (buf, "gnre", (uint16_t)index);
      }
    }

  for(metaptr = 0; metaptr < data->count; metaptr++) {
    if (!mask[metaptr]) {
      tag_t* tag = &data->tags[metaptr];
      const char* standardMetaAtom = findStandardMetaAtom (tag->item);
      if (standardMetaAtom)
        membufferWrite_std_tag (buf, standardMetaAtom, tag->value);
      }
    }

  free (mask);

  if (membuffer_error (buf)) {
    membuffer_free (buf);
    return 0;
    }

  *out_size = membuffer_get_size (buf);
  *out_buffer = membuffer_detach (buf);
  membuffer_free (buf);

  return 1;
  }
//}}}
//{{{
uint32_t cMp4::create_meta (const metadata_t* data, void** out_buffer, uint32_t* out_size) {

  uint32_t ilst_size;
  void* ilst_buffer;
  if (!create_ilst (data, &ilst_buffer, &ilst_size))
    return 0;

  membuffer_t* buf = membuffer_create();

  membufferWriteInt32 (buf, 0);
  membufferWrite_atom (buf, "ilst", ilst_size, ilst_buffer);
  free (ilst_buffer);

  *out_size = membuffer_get_size(buf);
  *out_buffer = membuffer_detach(buf);
  membuffer_free(buf);

  return 1;
  }
//}}}
//{{{
uint32_t cMp4::create_udta (const metadata_t* data, void** out_buffer, uint32_t* out_size) {

  membuffer_t* buf;
  uint32_t meta_size;
  void* meta_buffer;

  if (!create_meta (data, &meta_buffer, &meta_size))
    return 0;

  buf = membuffer_create();

  membufferWrite_atom (buf, "meta", meta_size,meta_buffer);

  free (meta_buffer);

  *out_size = membuffer_get_size (buf);
  *out_buffer = membuffer_detach (buf);
  membuffer_free (buf);

  return 1;
  }
//}}}
//{{{
uint32_t cMp4::modify_moov (const metadata_t* data, uint8_t** out_buffer, uint32_t* out_size) {

  uint64_t total_base = moov_offset + 8;
  uint32_t total_size = (uint32_t)(moov_size - 8);

  uint64_t udta_offset, meta_offset, ilst_offset;
  uint32_t udta_size, meta_size, ilst_size;

  uint32_t new_ilst_size;
  void* new_ilst_buffer;

  uint8_t* p_out;
  int32_t size_delta;
  if (!find_atom_v2 (total_base, total_size, "udta", 0, "meta")) {
    void* new_udta_buffer;
    uint32_t new_udta_size;
    if (!create_udta (data, &new_udta_buffer, &new_udta_size))
      return 0;

    membuffer_t* buf = membuffer_create();
    setPosition (total_base);
    membuffer_transfer_from_file (buf, total_size);

    membufferWrite_atom (buf, "udta", new_udta_size,  new_udta_buffer);

    free (new_udta_buffer);

    *out_size = membuffer_get_size (buf);
    *out_buffer = (uint8_t*)membuffer_detach (buf);
    membuffer_free(buf);
    return 1;
    }

  else {
    udta_offset = getPosition();
    udta_size = readInt32();
    if (!find_atom_v2 (udta_offset + 8, udta_size - 8, "meta", 4, "ilst")) {
      void* new_meta_buffer;
      uint32_t new_meta_size;
      if (!create_meta (data, &new_meta_buffer, &new_meta_size))
        return 0;

      membuffer_t* buf = membuffer_create();
      setPosition (total_base);
      membuffer_transfer_from_file (buf, (uint32_t)(udta_offset - total_base));

      membufferWriteInt32(buf,udta_size + 8 + new_meta_size);
      membufferWrite_atom_name(buf,"udta");
      membuffer_transfer_from_file (buf,udta_size);

      membufferWrite_atom (buf, "meta", new_meta_size, new_meta_buffer);
      free (new_meta_buffer);

      *out_size = membuffer_get_size(buf);
      *out_buffer = (uint8_t*)membuffer_detach(buf);
      membuffer_free(buf);
      return 1;
      }

    meta_offset = getPosition();
    meta_size = readInt32();
    if (!find_atom (meta_offset + 12, meta_size - 12, "ilst"))
      return 0;//shouldn't happen, find_atom_v2 above takes care of it
    ilst_offset = getPosition();
    ilst_size = readInt32();

    if (!create_ilst (data, &new_ilst_buffer, &new_ilst_size))
      return 0;

    size_delta = new_ilst_size - (ilst_size - 8);

    *out_size = total_size + size_delta;
    *out_buffer = (uint8_t*)malloc(*out_size);
    if (*out_buffer == 0) {
      free (new_ilst_buffer);
      return 0;
      }

    p_out = (uint8_t*)*out_buffer;

    setPosition (total_base);
    readData (p_out,(uint32_t)(udta_offset - total_base ));
    p_out += (uint32_t)(udta_offset - total_base );
    *(uint32_t*)p_out = fixByteOrder32 (readInt32() + size_delta);
    p_out += 4;

    readData(p_out,4); p_out += 4;
    readData(p_out,(uint32_t)(meta_offset - udta_offset - 8));
    p_out += (uint32_t)(meta_offset - udta_offset - 8);
    *(uint32_t*)p_out = fixByteOrder32 (readInt32() + size_delta);
    p_out += 4;

    readData(p_out,4); p_out += 4;
    readData(p_out,(uint32_t)(ilst_offset - meta_offset - 8));
    p_out += (uint32_t)(ilst_offset - meta_offset - 8);
    *(uint32_t*)p_out = fixByteOrder32 (readInt32() + size_delta);
    p_out += 4;
    readData(p_out,4); p_out += 4;

    memcpy(p_out,new_ilst_buffer,new_ilst_size);
    p_out += new_ilst_size;

    setPosition(ilst_offset + ilst_size);
    readData(p_out,(uint32_t)(total_size - (ilst_offset - total_base) - ilst_size));

    free(new_ilst_buffer);
    }

  return 1;
  }
//}}}
//}}}
//{{{  sample
//{{{
bool cMp4::chunk_of_sample (int track, int sample, int32_t* chunk_sample, int32_t* chunk) {

  if (tracks[track] == NULL) {
    //{{{  return false, and zeros
    *chunk = 0;
    *chunk_sample = 0;
    return false;
    }
    //}}}

  int32_t total_entries = tracks[track]->stsc_entry_count;
  int32_t chunk1 = 1;
  int32_t chunk1samples = 0;
  int32_t chunk2entry = 0;
  int32_t total = 0;
  do {
    int32_t chunk2 = tracks[track]->stsc_first_chunk[chunk2entry];
    *chunk = chunk2 - chunk1;
    int32_t range_samples = *chunk * chunk1samples;

    if (sample < total + range_samples)
      break;

    chunk1samples = tracks[track]->stsc_samples_per_chunk[chunk2entry];
    chunk1 = chunk2;

    if (chunk2entry < total_entries) {
      chunk2entry++;
      total += range_samples;
      }
    } while (chunk2entry < total_entries);

  if (chunk1samples)
    *chunk = (sample - total) / chunk1samples + chunk1;
  else
    *chunk = 1;

  *chunk_sample = total + (*chunk - chunk1) * chunk1samples;

  return true;
  }
//}}}
//{{{
int32_t cMp4::chunk_to_offset (int track, int32_t chunk) {

  if (tracks[track]->stco_entry_count && (chunk > tracks[track]->stco_entry_count))
    return tracks[track]->stco_chunk_offset[tracks[track]->stco_entry_count - 1];
  else if (tracks[track]->stco_entry_count)
    return tracks[track]->stco_chunk_offset[chunk - 1];
  else
    return 8;

  return 0;
  }
//}}}

//{{{
int32_t cMp4::sample_range_size (int track, int32_t chunk_sample, int sample) {

  if (tracks[track]->stsz_sample_size)
    return (sample - chunk_sample) * tracks[track]->stsz_sample_size;

  else {
    if (sample >= tracks[track]->stsz_sample_count)
      return 0; //error

    int32_t total = 0;
    for (int32_t i = chunk_sample; i < sample; i++)
      total += tracks[track]->stsz_table[i];
    return total;
    }
  }
//}}}
//{{{
int32_t cMp4::sample_to_offset (int track, int sample) {

  int32_t chunk;
  int32_t chunk_sample;
  chunk_of_sample (track, sample, &chunk_sample, &chunk);
  return chunk_to_offset (track, chunk) + sample_range_size (track, chunk_sample, sample);
  }
//}}}
//}}}
//{{{  atom
//{{{
int32_t cMp4::readStsz() {

  readChar(); /* version */
  readInt24(); /* flags */

  tracks[numTracks - 1]->stsz_sample_size = readInt32();
  tracks[numTracks - 1]->stsz_sample_count = readInt32();

  if (tracks[numTracks - 1]->stsz_sample_size == 0) {
    tracks[numTracks - 1]->stsz_table = (int32_t*)malloc (tracks[numTracks - 1]->stsz_sample_count * sizeof(int32_t));
    for (auto i = 0; i < tracks[numTracks - 1]->stsz_sample_count; i++)
      tracks[numTracks - 1]->stsz_table[i] = readInt32();
    }

  return 0;
  }
//}}}
//{{{
int32_t cMp4::readEsds() {

  readChar(); /* version */
  readInt24(); /* flags */

  /* get and verify ES_DescrTag */
  auto tag = readChar();
  if (tag == 0x03) {
    /* read length */
    if (readMp4DescrLength() < 5 + 15)
      return 1;
    /* skip 3 bytes */
    readInt24();
    }
  else
    /* skip 2 bytes */
    readInt16();

  // get and verify DecoderConfigDescrTab
  if (readChar() != 0x04)
    return 1;

  // read length
  auto temp = readMp4DescrLength();
  if (temp < 13)
    return 1;

  tracks[numTracks - 1]->audioType = readChar();
  readInt32();//0x15000414 ????
  tracks[numTracks - 1]->maxBitrate = readInt32();
  tracks[numTracks - 1]->avgBitrate = readInt32();

  // get and verify DecSpecificInfoTag
  if (readChar() != 0x05)
    return 1;

  // read length
  tracks[numTracks - 1]->decoderConfigLen = readMp4DescrLength();

  if (tracks[numTracks - 1]->decoderConfig)
    free(tracks[numTracks - 1]->decoderConfig);
  tracks[numTracks - 1]->decoderConfig = (uint8_t*)malloc (tracks[numTracks - 1]->decoderConfigLen);

  if (tracks[numTracks - 1]->decoderConfig)
    readData (tracks[numTracks - 1]->decoderConfig, tracks[numTracks - 1]->decoderConfigLen);
  else
    tracks[numTracks - 1]->decoderConfigLen = 0;

  // skip remainder of atom
  return 0;
  }
//}}}
//{{{
int32_t cMp4::readStsc() {

  readChar(); /* version */
  readInt24(); /* flags */

  tracks[numTracks-1]->stsc_entry_count = readInt32();

  tracks[numTracks-1]->stsc_first_chunk = (int32_t*)malloc (tracks[numTracks-1]->stsc_entry_count * sizeof (int32_t));
  tracks[numTracks-1]->stsc_samples_per_chunk = (int32_t*)malloc (tracks[numTracks-1]->stsc_entry_count * sizeof (int32_t));
  tracks[numTracks-1]->stsc_sample_desc_index = (int32_t*)malloc (tracks[numTracks-1]->stsc_entry_count * sizeof (int32_t));

  for (auto i = 0; i < tracks[numTracks-1]->stsc_entry_count; i++) {
    tracks[numTracks-1]->stsc_first_chunk[i] = readInt32();
    tracks[numTracks-1]->stsc_samples_per_chunk[i] = readInt32();
    tracks[numTracks-1]->stsc_sample_desc_index[i] = readInt32();
    }

  return 0;
  }
//}}}
//{{{
int32_t cMp4::readStco() {

  readChar(); /* version */
  readInt24(); /* flags */
  tracks[numTracks-1]->stco_entry_count = readInt32();

  tracks[numTracks-1]->stco_chunk_offset = (int32_t*)malloc (tracks[numTracks-1]->stco_entry_count * sizeof (int32_t));

  for (auto i = 0; i < tracks[numTracks-1]->stco_entry_count; i++)
    tracks[numTracks - 1]->stco_chunk_offset[i] = readInt32();

  return 0;
  }
//}}}
//{{{
int32_t cMp4::readCtts() {

  if (tracks[numTracks - 1]->ctts_entry_count)
    return 0;

  readChar(); /* version */
  readInt24(); /* flags */
  tracks[numTracks - 1]->ctts_entry_count = readInt32();

  tracks[numTracks - 1]->ctts_sample_count = (int32_t*)malloc (tracks[numTracks - 1]->ctts_entry_count * sizeof (int32_t));
  tracks[numTracks - 1]->ctts_sample_offset = (int32_t*)malloc (tracks[numTracks - 1]->ctts_entry_count * sizeof (int32_t));

  if (tracks[numTracks - 1]->ctts_sample_count == 0 || tracks[numTracks - 1]->ctts_sample_offset == 0) {
    if (tracks[numTracks - 1]->ctts_sample_count) {
      free (tracks[numTracks - 1]->ctts_sample_count);
      tracks[numTracks - 1]->ctts_sample_count = 0;
      }
    if (tracks[numTracks - 1]->ctts_sample_offset) {
      free (tracks[numTracks - 1]->ctts_sample_offset);
      tracks[numTracks - 1]->ctts_sample_offset = 0;
      }
    tracks[numTracks - 1]->ctts_entry_count = 0;
    return 0;
    }
  else {
    for (auto i = 0; i < tracks[numTracks - 1]->ctts_entry_count; i++) {
      tracks[numTracks - 1]->ctts_sample_count[i] = readInt32();
      tracks[numTracks - 1]->ctts_sample_offset[i] = readInt32();
      }
    return 1;
    }
  }
//}}}
//{{{
int32_t cMp4::readStts() {

  if (tracks[numTracks - 1]->stts_entry_count)
    return 0;

  readChar(); /* version */
  readInt24(); /* flags */
  tracks[numTracks - 1]->stts_entry_count = readInt32();
  tracks[numTracks - 1]->stts_sample_count = (int32_t*)malloc (tracks[numTracks - 1]->stts_entry_count * sizeof (int32_t));
  tracks[numTracks - 1]->stts_sample_delta = (int32_t*)malloc (tracks[numTracks - 1]->stts_entry_count * sizeof (int32_t));

  if ((tracks[numTracks - 1]->stts_sample_count == 0) || (tracks[numTracks - 1]->stts_sample_delta == 0)) {
    if (tracks[numTracks - 1]->stts_sample_count) {
      free (tracks[numTracks - 1]->stts_sample_count);
      tracks[numTracks - 1]->stts_sample_count = 0;
      }
    if (tracks[numTracks - 1]->stts_sample_delta) {
      free (tracks[numTracks - 1]->stts_sample_delta);
      tracks[numTracks - 1]->stts_sample_delta = 0;
      }
    tracks[numTracks - 1]->stts_entry_count = 0;
    return 0;
    }
  else {
    for (auto i = 0; i < tracks[numTracks - 1]->stts_entry_count; i++) {
      tracks[numTracks - 1]->stts_sample_count[i] = readInt32();
      tracks[numTracks - 1]->stts_sample_delta[i] = readInt32();
      }
    return 1;
    }
  }
//}}}
//{{{
int32_t cMp4::readMvhd() {

  readChar(); /* version */
  readInt24(); /* flags */

  /* creation_time */ readInt32();
  /* modification_time */ readInt32();

  time_scale = readInt32();
  duration = readInt32();

  /* preferred_rate */ readInt32(); /*read_fixed32();*/
  /* preferred_volume */ readInt16(); /*read_fixed16();*/

  for (auto i = 0; i < 10; i++)
    /* reserved */ readChar();

  for (auto i = 0; i < 9; i++)
    readInt32(); /* matrix */

  /* preview_time */ readInt32();
  /* preview_duration */ readInt32();
  /* poster_time */ readInt32();
  /* selection_time */ readInt32();
  /* selection_duration */ readInt32();
  /* current_time */ readInt32();
  /* next_track_id */ readInt32();

  return 0;
  }
//}}}
//{{{
int32_t cMp4::readTkhd() {

  auto version = readChar(); /* version */
  auto flags = readInt24(); /* flags */

  if (version == 1) {
    readInt64(); // creation-time
    readInt64(); // modification-time
    readInt32(); // track-id
    readInt32(); // reserved
    tracks[numTracks - 1]->duration = readInt64();//duration
    }
  else { //version == 0
    readInt32(); // creation-time
    readInt32(); // modification-time
    readInt32(); // track-id
    readInt32(); // reserved
    tracks[numTracks - 1]->duration = readInt32();//duration
    if (tracks[numTracks - 1]->duration == 0xFFFFFFFF)
      tracks[numTracks - 1]->duration = 0xFFFFFFFFFFFFFFFF;
    }

  readInt32(); // reserved
  readInt32(); // reserved
  readInt16(); // layer
  readInt16(); // pre-defined
  readInt16(); // volume
  readInt16(); // reserved

  // matrix
  readInt32();
  readInt32();
  readInt32();
  readInt32();
  readInt32();
  readInt32();
  readInt32();
  readInt32();
  readInt32();

  readInt32(); // width
  readInt32(); // height
  return 1;
  }
//}}}
//{{{
int32_t cMp4::readMdhd() {

  auto version = readInt32();
  if (version == 1) {
    readInt64();//creation-time
    readInt64();//modification-time
    tracks[numTracks - 1]->timeScale = readInt32();//timescale
    tracks[numTracks - 1]->duration = readInt64();//duration
    }
  else { //version == 0
    readInt32();//creation-time
    readInt32();//modification-time
    tracks[numTracks - 1]->timeScale = readInt32();//timescale
    auto temp = readInt32();
    tracks[numTracks - 1]->duration = (temp == (uint32_t)(-1)) ? (uint64_t)(-1) : (uint64_t)(temp);
    }

  readInt16();
  readInt16();
  return 1;
  }
//}}}

//{{{
int32_t cMp4::readMeta (uint64_t size, int indent) {

  readChar(); /* version */
  readInt24(); /* flags */

  uint64_t sumsize = 0;
  uint8_t header_size = 0;
  while (sumsize < (size-(header_size + 4))) {
    uint8_t atom_type;
    uint64_t subsize = readAtomHeader (&atom_type, &header_size, indent);
    if (subsize <= header_size + 4)
      return 1;
    if (atom_type == ATOM_ILST)
      parse_metadata ((uint32_t)(subsize - (header_size + 4)), indent);
    else
      setPosition (getPosition() + subsize - header_size);
    sumsize += subsize;
    }

  return 0;
  }
//}}}
//{{{
int32_t cMp4::readMp4a (int indent) {

  for (auto i = 0; i < 6; i++)
    readChar(); /* reserved */
  /* data_reference_index */ readInt16();

  readInt32(); /* reserved */
  readInt32(); /* reserved */

  tracks[numTracks - 1]->channelCount = readInt16();
  tracks[numTracks - 1]->sampleSize = readInt16();

  readInt16();
  readInt16();

  tracks[numTracks - 1]->sampleRate = readInt16();

  readInt16();

  uint8_t atom_type = 0;
  uint8_t header_size = 0;
  auto size = readAtomHeader (&atom_type, &header_size, indent);
  if (atom_type == ATOM_ESDS)
    readEsds();

  return 0;
  }
//}}}
//{{{
int32_t cMp4::readStsd (int indent) {

  readChar(); /* version */
  readInt24(); /* flags */

  tracks[numTracks - 1]->stsd_entry_count = readInt32();

  for (auto i = 0; i < tracks[numTracks - 1]->stsd_entry_count; i++) {
    uint64_t skip = getPosition();
    uint8_t atom_type = 0;
    uint8_t header_size = 0;
    auto size = readAtomHeader (&atom_type, &header_size, indent);
    skip += size;

    if (atom_type == ATOM_MP4A) {
      tracks[numTracks - 1]->type = eTrackAudio;
      readMp4a (indent);
      }
    else if (atom_type == ATOM_MP4V)
      tracks[numTracks - 1]->type = eTrackVideo;
    else if (atom_type == ATOM_AVC1)
      tracks[numTracks - 1]->type = eTrackVideo;
    else if (atom_type == ATOM_MP4S)
      tracks[numTracks - 1]->type = eTrackSystem;
    else
      tracks[numTracks - 1]->type = eTrackUnknown;

    setPosition (skip);
    }

  return 0;
  }
//}}}

//{{{
int32_t cMp4::parseAtom (int32_t size, uint8_t atom_type, int indent) {

  auto dest_position = getPosition() + size - 8;

  if (atom_type == ATOM_STSZ)      // sample size box
    readStsz();
  else if (atom_type == ATOM_STTS) // time to sample box
    readStts();
  else if (atom_type == ATOM_CTTS) // composition offset box
    readCtts();
  else if (atom_type == ATOM_STSC) // sample to chunk box
    readStsc();
  else if (atom_type == ATOM_STCO) // chunk offset box
    readStco();
  else if (atom_type == ATOM_MVHD) // movie header box
    readMvhd();
  else if (atom_type == ATOM_MDHD) // track header
    readMdhd();
  else if (atom_type == ATOM_STSD) // sample description box
    readStsd (indent);
  else if (atom_type == ATOM_META) // iTunes Metadata box
    readMeta (size, indent);

  setPosition (dest_position);
  return 0;
  }
//}}}
//{{{
int32_t cMp4::parseSubAtoms (const uint64_t total_size, int indent) {
// parse atoms that are sub atoms of other atoms

  uint64_t count = 0;
  while (count < total_size) {
    uint8_t atom_type = 0;
    uint8_t header_size = 0;
    auto size = readAtomHeader (&atom_type, &header_size, indent);
    if (!size)
      break;

    if (atom_type == ATOM_TRAK) {
      // add new track
      tracks[numTracks] = (track_t*)malloc (sizeof (track_t));
      memset (tracks[numTracks], 0, sizeof (track_t));
      numTracks++;
      }

    if (atom_type < SUBATOMIC)
      parseSubAtoms (size - header_size, indent + 1);
    else
      parseAtom ((uint32_t)size, atom_type, indent);

    count += size;
    }

  return 0;
  }
//}}}
//{{{
int32_t cMp4::parseAtoms() {
// parse root atoms

  file_size = 0;

  while (true) {
    uint8_t atom_type = 0;
    uint8_t header_size = 0;
    auto size = readAtomHeader (&atom_type, &header_size, 0);
    if (size == 0)
      break;
    file_size += size;
    last_atom = atom_type;

    if ((atom_type == ATOM_MDAT) && moov_read) {
      // moov atom is before mdat, we can stop reading when mdat is encountered
      // file position will stay at beginning of mdat data
      //break;
      }

    if ((atom_type == ATOM_MOOV) && (size > header_size)) {
      moov_read = 1;
      moov_offset = getPosition() - header_size;
      moov_size = size;
      }

    // parse subatoms
    if (atom_type < SUBATOMIC)
      parseSubAtoms (size - header_size, 1);
    else // skip this atom
      setPosition (getPosition() + size - header_size);
    }

  return 0;
  }
//}}}
//}}}
