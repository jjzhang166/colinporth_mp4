// mp4ff.c
//{{{  includes
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "mp4ff.h"

#define stricmp _stricmp
#define strdup _strdup
//}}}
//{{{  types
//{{{  struct stdmeta_entry
typedef struct {
  const char * atom;
  const char * name;
  } stdmeta_entry;
//}}}
//{{{  struct membuffer
typedef struct {
  void* data;
  unsigned written;
  unsigned allocated;
  unsigned error;
  } membuffer;
//}}}
//}}}
//{{{  const
#define COPYRIGHT_SYMBOL ((int8_t)0xA9)
//{{{
static const char* tag_names[] = {
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
static const char* ID3v1GenreList[] = {
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

//{{{
static stdmeta_entry stdmetas[] =
{
  {"\xA9" "nam", "title"},
  {"\xA9" "ART", "artist"},
  {"\xA9" "wrt", "writer"},
  {"\xA9" "alb", "album"},
  {"\xA9" "day", "date"},
  {"\xA9" "too", "tool"},
  {"\xA9" "cmt", "comment"},
//  {"\xA9" "gen","genre"},
  {"cpil", "compilation"},
//  {"trkn","track"},
//  {"disk","disc"},
//  {"gnre","genre"},
  {"covr", "cover"},
  /* added by AJS */
  {"aART", "album_artist"},
};
//}}}
//}}}

//{{{  read utils
//{{{
static int64_t mp4ff_position (const mp4ff_t* f) {
  return f->current_position;
  }
//}}}
//{{{
static int32_t mp4ff_set_position (mp4ff_t* f, const uint64_t position) {

  f->stream->seek (f->stream->user_data, position);
  f->current_position = position;
  return 0;
  }
//}}}

//{{{
static int32_t mp4ff_read_data (mp4ff_t* f, uint8_t* data, uint32_t size) {

  int32_t result = f->stream->read (f->stream->user_data, data, size);
  f->current_position += size;
  return result;
  }
//}}}

//{{{
static uint64_t mp4ff_read_int64 (mp4ff_t* f) {

  uint8_t data[8];
  mp4ff_read_data (f, data, 8);

  uint64_t result = 0;
  for (int i = 0; i < 8; i++)
    result |= ((uint64_t)data[i]) << ((7 - i) * 8);

  return result;
  }
//}}}
//{{{
static uint32_t mp4ff_read_int32 (mp4ff_t* f) {

  uint8_t data[4];
  mp4ff_read_data (f, data, 4);

  uint32_t a = data[0];
  uint32_t b = data[1];
  uint32_t c = data[2];
  uint32_t d = data[3];

  return (a<<24) | (b<<16) | (c<<8) | d;
  }
//}}}
//{{{
static uint32_t mp4ff_read_int24 (mp4ff_t* f) {

  uint8_t data[4];
  mp4ff_read_data (f, data, 3);

  uint32_t a = data[0];
  uint32_t b = data[1];
  uint32_t c = data[2];

  return (a<<16) | (b<<8) | c;
  }
//}}}
//{{{
static uint16_t mp4ff_read_int16 (mp4ff_t* f) {

  uint8_t data[2];
  mp4ff_read_data (f, data, 2);

  return (data[0]<<8) | data[1];
  }
//}}}

//{{{
static uint8_t mp4ff_read_char (mp4ff_t* f) {

  uint8_t output;
  mp4ff_read_data (f, &output, 1);
  return output;
  }
//}}}
//{{{
static char* mp4ff_read_string (mp4ff_t* f, uint32_t length) {

  uint8_t* str = (uint8_t*)malloc (length + 1);
  if (str != 0) {
    if ((uint32_t)mp4ff_read_data (f, str, length) != length) {
      free (str);
      str = 0;
      }
    else
      str[length] = 0;
    }

  return (char*)str;
  }
//}}}

//{{{
static uint32_t mp4ff_read_mp4_descr_length (mp4ff_t* f) {

  uint8_t b;
  uint8_t numBytes = 0;
  uint32_t length = 0;

  do {
    b = mp4ff_read_char (f);
    numBytes++;
    length = (length << 7) | (b & 0x7F);
    } while ((b & 0x80) && numBytes < 4);

  return length;
  }
//}}}
//}}}
//{{{  write utils
//{{{
static int32_t mp4ff_truncate (mp4ff_t* f) {

  return f->stream->truncate (f->stream->user_data);
  }
//}}}
//{{{
static int32_t mp4ff_write_data (mp4ff_t* f, uint8_t* data, uint32_t size) {

  int32_t result = 1;
  result = f->stream->write (f->stream->user_data, data, size);
  f->current_position += size;
  return result;
  }
//}}}
//{{{
static int32_t mp4ff_write_int32 (mp4ff_t* f, const uint32_t data) {

  uint32_t result;
  uint32_t a, b, c, d;
  int8_t temp[4];

  *(uint32_t*)temp = data;
  a = temp[0];
  b = temp[1];
  c = temp[2];
  d = temp[3];

  result = (a<<24) | (b<<16) | (c<<8) | d;

  return mp4ff_write_data (f, (uint8_t*)&result, sizeof (result));
  }
//}}}
//}}}
//{{{  parse utils
//{{{
static bool mp4ff_atom_compare (uint8_t* ch1, char* ch2) {
// compare 2 atom names, returns 1 for equal, 0 for unequal

  for (int i = 0; i < 4; i++)
    if (ch1[i] != ch2[i])
      return false;

  return true;
  }
//}}}
//{{{
static uint8_t mp4ff_atom_name_to_type (uint8_t* fourChars) {

  if (mp4ff_atom_compare (fourChars, "moov")) return ATOM_MOOV;
  else if (mp4ff_atom_compare (fourChars, "minf")) return ATOM_MINF;
  else if (mp4ff_atom_compare (fourChars, "mdia")) return ATOM_MDIA;
  else if (mp4ff_atom_compare (fourChars, "mdat")) return ATOM_MDAT;
  else if (mp4ff_atom_compare (fourChars, "mdhd")) return ATOM_MDHD;
  else if (mp4ff_atom_compare (fourChars, "mvhd")) return ATOM_MVHD;
  else if (mp4ff_atom_compare (fourChars, "mp4a")) return ATOM_MP4A;
  else if (mp4ff_atom_compare (fourChars, "mp4v")) return ATOM_MP4V;
  else if (mp4ff_atom_compare (fourChars, "mp4s")) return ATOM_MP4S;
  else if (mp4ff_atom_compare (fourChars, "meta")) return ATOM_META;
  else if (mp4ff_atom_compare (fourChars, "trak")) return ATOM_TRAK;
  else if (mp4ff_atom_compare (fourChars, "tkhd")) return ATOM_TKHD;
  else if (mp4ff_atom_compare (fourChars, "tref")) return ATOM_TREF;
  else if (mp4ff_atom_compare (fourChars, "trkn")) return ATOM_TRACK;
  else if (mp4ff_atom_compare (fourChars, "tmpo")) return ATOM_TEMPO;
  else if (mp4ff_atom_compare (fourChars, "tvnn")) return ATOM_NETWORK;
  else if (mp4ff_atom_compare (fourChars, "tvsh")) return ATOM_SHOW;
  else if (mp4ff_atom_compare (fourChars, "tven")) return ATOM_EPISODENAME;
  else if (mp4ff_atom_compare (fourChars, "tvsn")) return ATOM_SEASON;
  else if (mp4ff_atom_compare (fourChars, "tves")) return ATOM_EPISODE;
  else if (mp4ff_atom_compare (fourChars, "stbl")) return ATOM_STBL;
  else if (mp4ff_atom_compare (fourChars, "smhd")) return ATOM_SMHD;
  else if (mp4ff_atom_compare (fourChars, "stsd")) return ATOM_STSD;
  else if (mp4ff_atom_compare (fourChars, "stts")) return ATOM_STTS;
  else if (mp4ff_atom_compare (fourChars, "stco")) return ATOM_STCO;
  else if (mp4ff_atom_compare (fourChars, "stsc")) return ATOM_STSC;
  else if (mp4ff_atom_compare (fourChars, "stsz")) return ATOM_STSZ;
  else if (mp4ff_atom_compare (fourChars, "stz2")) return ATOM_STZ2;
  else if (mp4ff_atom_compare (fourChars, "skip")) return ATOM_SKIP;
  else if (mp4ff_atom_compare (fourChars, "sinf")) return ATOM_SINF;
  else if (mp4ff_atom_compare (fourChars, "schi")) return ATOM_SCHI;
  else if (mp4ff_atom_compare (fourChars, "sonm")) return ATOM_SORTTITLE;
  else if (mp4ff_atom_compare (fourChars, "soal")) return ATOM_SORTALBUM;
  else if (mp4ff_atom_compare (fourChars, "soar")) return ATOM_SORTARTIST;
  else if (mp4ff_atom_compare (fourChars, "soaa")) return ATOM_SORTALBUMARTIST;
  else if (mp4ff_atom_compare (fourChars, "soco")) return ATOM_SORTWRITER;
  else if (mp4ff_atom_compare (fourChars, "sosn")) return ATOM_SORTSHOW;
  else if (mp4ff_atom_compare (fourChars, "\251A9nam")) return ATOM_TITLE;
  else if (mp4ff_atom_compare (fourChars, "\251ART")) return ATOM_ARTIST;
  else if (mp4ff_atom_compare (fourChars, "\251wrt")) return ATOM_WRITER;
  else if (mp4ff_atom_compare (fourChars, "\251alb")) return ATOM_ALBUM;
  else if (mp4ff_atom_compare (fourChars, "\251day")) return ATOM_DATE;
  else if (mp4ff_atom_compare (fourChars, "\251too")) return ATOM_TOOL;
  else if (mp4ff_atom_compare (fourChars, "\251cmt")) return ATOM_COMMENT;
  else if (mp4ff_atom_compare (fourChars, "\251gen")) return ATOM_GENRE1;
  else if (mp4ff_atom_compare (fourChars, "\251grp")) return ATOM_CONTENTGROUP;
  else if (mp4ff_atom_compare (fourChars, "\251lyr")) return ATOM_LYRICS;
  else if (mp4ff_atom_compare (fourChars, "edts")) return ATOM_EDTS;
  else if (mp4ff_atom_compare (fourChars, "esds")) return ATOM_ESDS;
  else if (mp4ff_atom_compare (fourChars, "ftyp")) return ATOM_FTYP;
  else if (mp4ff_atom_compare (fourChars, "free")) return ATOM_FREE;
  else if (mp4ff_atom_compare (fourChars, "hmhd")) return ATOM_HMHD;
  else if (mp4ff_atom_compare (fourChars, "vmhd")) return ATOM_VMHD;
  else if (mp4ff_atom_compare (fourChars, "udta")) return ATOM_UDTA;
  else if (mp4ff_atom_compare (fourChars, "ilst")) return ATOM_ILST;
  else if (mp4ff_atom_compare (fourChars, "name")) return ATOM_NAME;
  else if (mp4ff_atom_compare (fourChars, "data")) return ATOM_DATA;
  else if (mp4ff_atom_compare (fourChars, "disk")) return ATOM_DISC;
  else if (mp4ff_atom_compare (fourChars, "gnre")) return ATOM_GENRE2;
  else if (mp4ff_atom_compare (fourChars, "covr")) return ATOM_COVER;
  else if (mp4ff_atom_compare (fourChars, "cpil")) return ATOM_COMPILATION;
  else if (mp4ff_atom_compare (fourChars, "ctts")) return ATOM_CTTS;
  else if (mp4ff_atom_compare (fourChars, "drms")) return ATOM_DRMS;
  else if (mp4ff_atom_compare (fourChars, "frma")) return ATOM_FRMA;
  else if (mp4ff_atom_compare (fourChars, "priv")) return ATOM_PRIV;
  else if (mp4ff_atom_compare (fourChars, "iviv")) return ATOM_IVIV;
  else if (mp4ff_atom_compare (fourChars, "user")) return ATOM_USER;
  else if (mp4ff_atom_compare (fourChars, "key ")) return ATOM_KEY;
  else if (mp4ff_atom_compare (fourChars, "aART")) return ATOM_ALBUM_ARTIST;
  else if (mp4ff_atom_compare (fourChars, "desc")) return ATOM_DESCRIPTION;
  else if (mp4ff_atom_compare (fourChars, "pcst")) return ATOM_PODCAST;
  else
    return ATOM_UNKNOWN;
  }
//}}}

//{{{
static uint64_t mp4ff_atom_read_header (mp4ff_t* f, uint8_t* atom_type, uint8_t* header_size, int indent)  {
// read atom header, return atom size, atom size is with header included

  if (f->debug)
    printf ("%8lld ", mp4ff_position (f));

  uint8_t atom_header[8];
  int32_t ret = mp4ff_read_data (f, atom_header, 8);
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
    size = mp4ff_read_int64 (f);
    }
  else
    *header_size = 8;

  if (f->debug) {
    printf ("%8lld ", size);
    for (int i= 0; i < indent; i++)
      printf ("  ");
    printf ("%c%c%c%c\n", atom_header[4], atom_header[5], atom_header[6], atom_header[7]);
    }

  *atom_type = mp4ff_atom_name_to_type (&atom_header[4]);

  return size;
  }
//}}}
//}}}
//{{{  meta utils
//{{{
static int32_t mp4ff_tag_add_field (mp4ff_metadata_t* tags, const char* item, const char* value)
{
    void* backup = (void *)tags->tags;

    if (!item || (item && !*item) || !value) return 0;

    tags->tags = (mp4ff_tag_t*)realloc(tags->tags, (tags->count+1) * sizeof(mp4ff_tag_t));
    if (!tags->tags)
    {
        if (backup) free(backup);
        return 0;
    } else {
        tags->tags[tags->count].item = strdup(item);
        tags->tags[tags->count].value = strdup(value);

        if (!tags->tags[tags->count].item || !tags->tags[tags->count].value)
        {
            if (!tags->tags[tags->count].item) free (tags->tags[tags->count].item);
            if (!tags->tags[tags->count].value) free (tags->tags[tags->count].value);
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
static int32_t mp4ff_tag_set_field (mp4ff_metadata_t* tags, const char* item, const char* value)
{
    unsigned int i;

    if (!item || (item && !*item) || !value) return 0;

    for (i = 0; i < tags->count; i++)
    {
        if (!stricmp(tags->tags[i].item, item))
        {
      free(tags->tags[i].value);
      tags->tags[i].value = strdup(value);
            return 1;
        }
    }

    return mp4ff_tag_add_field(tags, item, value);
}
//}}}

//{{{
static int32_t mp4ff_set_metadata_name (mp4ff_t* f, uint8_t atom_type, char* *name) {

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

  *name = strdup(tag_names[tag_idx]);

  return 0;
  }
//}}}
//{{{
static const char* mp4ff_meta_index_to_genre (uint32_t idx) {

  if (idx>0 && idx<=sizeof(ID3v1GenreList)/sizeof(ID3v1GenreList[0]))
    return ID3v1GenreList[idx-1];
  else
    return 0;
  }
//}}}

//{{{
static int32_t mp4ff_parse_tag (mp4ff_t* f, uint8_t parent_atom_type, int32_t size) {

  uint8_t atom_type;
  uint8_t header_size = 0;
  uint64_t subsize, sumsize = 0;
  char*  name = NULL;
  char*  data = NULL;
  uint32_t done = 0;

  while (sumsize < size) {
    uint64_t destpos;
    subsize = mp4ff_atom_read_header (f, &atom_type, &header_size, 0);
    destpos = mp4ff_position(f)+subsize-header_size;
    if (!done) {
      if (atom_type == ATOM_DATA) {
        mp4ff_read_char(f); /* version */
        mp4ff_read_int24(f); /* flags */
        mp4ff_read_int32(f); /* reserved */

        /* some need special attention */
        if (parent_atom_type == ATOM_GENRE2 || parent_atom_type == ATOM_TEMPO) {
          if (subsize - header_size >= 8 + 2) {
            uint16_t val = mp4ff_read_int16(f);

            if (parent_atom_type == ATOM_TEMPO) {
              char temp[16];
              sprintf(temp, "%.5u BPM", val);
              mp4ff_tag_add_field(&(f->tags), "tempo", temp);
            }
            else {
              const char * temp = mp4ff_meta_index_to_genre (val);
              if (temp)
                mp4ff_tag_add_field (&(f->tags), "genre", temp);
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
            mp4ff_read_int16(f);
            index = mp4ff_read_int16(f);
            total = mp4ff_read_int16(f);
              /* modified by AJS */
            /* mp4ff_read_int16(f); */

            sprintf(temp,"%d",index);
            mp4ff_tag_add_field(&(f->tags), parent_atom_type == ATOM_TRACK ? "track" : "disc", temp);
            if (total>0)
            {
              sprintf(temp,"%d",total);
              mp4ff_tag_add_field(&(f->tags), parent_atom_type == ATOM_TRACK ? "totaltracks" : "totaldiscs", temp);
            }
            done = 1;
            }
          }
        else {
          if (data) {
            free(data);
            data = NULL;
            }
          data = mp4ff_read_string(f,(uint32_t)(subsize-(header_size+8)));
          }
        }
      else if (atom_type == ATOM_NAME) {
        if (!done) {
          mp4ff_read_char(f); /* version */
          mp4ff_read_int24(f); /* flags */
          if (name) free(name);
          name = mp4ff_read_string(f,(uint32_t)(subsize-(header_size+4)));
          }
        }
      mp4ff_set_position(f, destpos);
      sumsize += subsize;
      }
    }

  if (data) {
    if (!done) {
      if (name == NULL)
        mp4ff_set_metadata_name(f, parent_atom_type, &name);
      if (name)
        mp4ff_tag_add_field(&(f->tags), name, data);
      }

    free(data);
    }

  if (name)
    free(name);

  return 1;
  }
//}}}
//{{{
static int32_t mp4ff_parse_metadata (mp4ff_t* f, int32_t size, int indent) {

  uint64_t subsize, sumsize = 0;
  uint8_t atom_type;
  uint8_t header_size = 0;

  while (sumsize < size) {
    subsize = mp4ff_atom_read_header (f, &atom_type, &header_size, indent);
    if (subsize == 0)
      break;
    mp4ff_parse_tag(f, atom_type, (uint32_t)(subsize-header_size));
    sumsize += subsize;
    }

  return 0;
  }
//}}}

//{{{
/* find a metadata item by name */
/* returns 0 if item found, 1 if no such item */
static int32_t mp4ff_meta_find_by_name (const mp4ff_t* f, const char* item, char** value) {

  uint32_t i;

  for (i = 0; i < f->tags.count; i++) {
    if (!stricmp(f->tags.tags[i].item, item)) {
      *value = strdup(f->tags.tags[i].value);
      return 1;
      }
    }

  *value = NULL;

  /* not found */
  return 0;
  }
//}}}

//{{{
static uint32_t fix_byte_order_32 (uint32_t src)
{
    uint32_t result;
    uint32_t a, b, c, d;
    uint8_t data[4];

    memcpy(data,&src,sizeof(src));
    a = data[0];
    b = data[1];
    c = data[2];
    d = data[3];

    result = (a<<24) | (b<<16) | (c<<8) | d;
    return (uint32_t)result;
}
//}}}
//{{{
static uint16_t fix_byte_order_16 (uint16_t src)
{
    uint16_t result;
    uint16_t a, b;
    uint8_t data[2];

    memcpy(data,&src,sizeof(src));
    a = data[0];
    b = data[1];

    result = (a<<8) | b;
    return (uint16_t)result;
}
//}}}

//{{{
static unsigned membuffer_write (membuffer * buf,const void * ptr,unsigned bytes)
{
  unsigned dest_size = buf->written + bytes;

  if (buf->error) return 0;
  if (dest_size > buf->allocated)
  {
    do
    {
      buf->allocated <<= 1;
    } while(dest_size > buf->allocated);

    {
      void * newptr = realloc(buf->data,buf->allocated);
      if (newptr==0)
      {
        free(buf->data);
        buf->data = 0;
        buf->error = 1;
        return 0;
      }
      buf->data = newptr;
    }
  }

  if (ptr) memcpy((char*)buf->data + buf->written,ptr,bytes);
  buf->written += bytes;
  return bytes;
}
//}}}
//{{{
static unsigned membuffer_write_int32 (membuffer* buf, uint32_t data)
{
  uint8_t temp[4] = {(uint8_t)(data>>24),(uint8_t)(data>>16),(uint8_t)(data>>8),(uint8_t)data};
  return membuffer_write(buf,temp,4);
}
//}}}
//{{{
static unsigned membuffer_write_int24 (membuffer* buf, uint32_t data)
{
  uint8_t temp[3] = {(uint8_t)(data>>16),(uint8_t)(data>>8),(uint8_t)data};
  return membuffer_write(buf,temp,3);
}
//}}}
//{{{
static unsigned membuffer_write_int16 (membuffer* buf, uint16_t data)
{
  uint8_t temp[2] = {(uint8_t)(data>>8),(uint8_t)data};
  return membuffer_write(buf,temp,2);
}
//}}}
//{{{
static unsigned membuffer_write_atom_name (membuffer* buf, const char * data)
{
  return membuffer_write(buf,data,4)==4 ? 1 : 0;
}
//}}}
//{{{
static void membuffer_write_atom (membuffer* buf, const char* name, unsigned size, const void* data)
{
  membuffer_write_int32(buf,size + 8);
  membuffer_write_atom_name(buf,name);
  membuffer_write(buf,data,size);
}
//}}}
//{{{
static unsigned membuffer_write_string (membuffer* buf, const char* data)
{
  return membuffer_write(buf,data,strlen(data));
}
//}}}
//{{{
static unsigned membuffer_write_int8 (membuffer* buf, uint8_t data)
{
  return membuffer_write(buf,&data,1);
}
//}}}

//{{{
static void* membuffer_get_ptr (const membuffer* buf)
{
  return buf->data;
}
//}}}
//{{{
static unsigned membuffer_get_size (const membuffer* buf)
{
  return buf->written;
}
//}}}

//{{{
static unsigned membuffer_error (const membuffer* buf)
{
  return buf->error;
}
//}}}
//{{{
static void membuffer_set_error(membuffer* buf) {
  buf->error = 1;
  }
//}}}
//{{{
static unsigned membuffer_transfer_from_file (membuffer* buf, mp4ff_t* src, unsigned bytes)
{
  unsigned oldsize;
  void * bufptr;

  oldsize = membuffer_get_size(buf);
  if (membuffer_write(buf,0,bytes) != bytes) 
    return 0;

  bufptr = membuffer_get_ptr(buf);
  if (bufptr==0) 
    return 0;

  if ((unsigned)mp4ff_read_data (src, (uint8_t*)bufptr + oldsize,bytes) != bytes) {
    membuffer_set_error(buf);
    return 0;
    }

  return bytes;
}
//}}}
//{{{
static membuffer * membuffer_create()
{
  const unsigned initial_size = 256;

  membuffer * buf = (membuffer *) malloc(sizeof(membuffer));
  buf->data = malloc(initial_size);
  buf->written = 0;
  buf->allocated = initial_size;
  buf->error = buf->data == 0 ? 1 : 0;

  return buf;
}
//}}}
//{{{
static void membuffer_free (membuffer* buf)
{
  if (buf->data) free(buf->data);
  free(buf);
}
//}}}
//{{{
static void* membuffer_detach (membuffer* buf)
{
  void * ret;

  if (buf->error) return 0;

  ret = realloc(buf->data,buf->written);

  if (ret == 0) free(buf->data);

  buf->data = 0;
  buf->error = 1;

  return ret;
}
//}}}

//{{{
static const char* find_standard_meta (const char* name) // returns atom name if found, 0 if not
{
  unsigned n;
  for(n=0;n<sizeof(stdmetas)/sizeof(stdmetas[0]);n++)
  {
    if (!stricmp(name,stdmetas[n].name)) return stdmetas[n].atom;
  }
    return 0;
}
//}}}
//{{{
static void membuffer_write_track_tag (membuffer* buf, const char* name, uint32_t index, uint32_t total)
{
  membuffer_write_int32(buf,8 /*atom header*/ + 8 /*data atom header*/ + 8 /*flags + reserved*/ + 8 /*actual data*/ );
  membuffer_write_atom_name(buf,name);
  membuffer_write_int32(buf,8 /*data atom header*/ + 8 /*flags + reserved*/ + 8 /*actual data*/ );
  membuffer_write_atom_name(buf,"data");
  membuffer_write_int32(buf,0);//flags
  membuffer_write_int32(buf,0);//reserved
  membuffer_write_int16(buf,0);
  membuffer_write_int16(buf,(uint16_t)index);//track number
  membuffer_write_int16(buf,(uint16_t)total);//total tracks
  membuffer_write_int16(buf,0);
}
//}}}
//{{{
static void membuffer_write_int16_tag (membuffer* buf, const char* name, uint16_t value)
{
  membuffer_write_int32(buf,8 /*atom header*/ + 8 /*data atom header*/ + 8 /*flags + reserved*/ + 2 /*actual data*/ );
  membuffer_write_atom_name(buf,name);
  membuffer_write_int32(buf,8 /*data atom header*/ + 8 /*flags + reserved*/ + 2 /*actual data*/ );
  membuffer_write_atom_name(buf,"data");
  membuffer_write_int32(buf,0);//flags
  membuffer_write_int32(buf,0);//reserved
  membuffer_write_int16(buf,value);//value
}
//}}}
//{{{
static void membuffer_write_std_tag (membuffer* buf, const char* name, const char* value)
{
  /* added by AJS */
  uint32_t flags = 1;

  /* special check for compilation flag */
  if ( strcmp(name, "cpil") == 0)
  {
    flags = 21;
  }

  membuffer_write_int32(buf,8 /*atom header*/ + 8 /*data atom header*/ + 8 /*flags + reserved*/ + strlen(value) );
  membuffer_write_atom_name(buf,name);
  membuffer_write_int32(buf,8 /*data atom header*/ + 8 /*flags + reserved*/ + strlen(value));
  membuffer_write_atom_name(buf,"data");
  membuffer_write_int32(buf,flags);//flags
  membuffer_write_int32(buf,0);//reserved
  membuffer_write(buf,value,strlen(value));
}
//}}}
//{{{
static void membuffer_write_custom_tag (membuffer* buf, const char* name, const char* value)
{
  membuffer_write_int32(buf,8 /*atom header*/ + 0x1C /*weirdo itunes atom*/ + 12 /*name atom header*/ + strlen(name) + 16 /*data atom header + flags*/ + strlen(value) );
  membuffer_write_atom_name(buf,"----");
  membuffer_write_int32(buf,0x1C);//weirdo itunes atom
  membuffer_write_atom_name(buf,"mean");
  membuffer_write_int32(buf,0);
  membuffer_write(buf,"com.apple.iTunes",16);
  membuffer_write_int32(buf,12 + strlen(name));
  membuffer_write_atom_name(buf,"name");
  membuffer_write_int32(buf,0);
  membuffer_write(buf,name,strlen(name));
  membuffer_write_int32(buf,8 /*data atom header*/ + 8 /*flags + reserved*/ + strlen(value));
  membuffer_write_atom_name(buf,"data");
  membuffer_write_int32(buf,1);//flags
  membuffer_write_int32(buf,0);//reserved
  membuffer_write(buf,value,strlen(value));

}
//}}}
//{{{
static uint32_t myatoi (const char * param)
{
  return param ? atoi(param) : 0;
}
//}}}

//{{{
static uint32_t mp4ff_meta_genre_to_index (const char* genrestr) {

  unsigned n;
  for (n = 0; n < sizeof (ID3v1GenreList)/sizeof(ID3v1GenreList[0]); n++)
    if (!stricmp (genrestr,ID3v1GenreList[n]))
      return n+1;
  return 0;
  }
//}}}
//{{{
static uint32_t create_ilst (const mp4ff_metadata_t* data,void ** out_buffer, uint32_t* out_size)
{
  membuffer * buf = membuffer_create();
  unsigned metaptr;
  char * mask = (char*)malloc(data->count);
  memset(mask,0,data->count);

  {
    const char * tracknumber_ptr = 0, * totaltracks_ptr = 0;
    const char * discnumber_ptr = 0, * totaldiscs_ptr = 0;
    const char * genre_ptr = 0, * tempo_ptr = 0;
    for(metaptr = 0; metaptr < data->count; metaptr++)
    {
      mp4ff_tag_t * tag = &data->tags[metaptr];
      if (!stricmp(tag->item,"tracknumber") || !stricmp(tag->item,"track"))
      {
        if (tracknumber_ptr==0) tracknumber_ptr = tag->value;
        mask[metaptr] = 1;
      }
      else if (!stricmp(tag->item,"totaltracks"))
      {
        if (totaltracks_ptr==0) totaltracks_ptr = tag->value;
        mask[metaptr] = 1;
      }
      else if (!stricmp(tag->item,"discnumber") || !stricmp(tag->item,"disc"))
      {
        if (discnumber_ptr==0) discnumber_ptr = tag->value;
        mask[metaptr] = 1;
      }
      else if (!stricmp(tag->item,"totaldiscs"))
      {
        if (totaldiscs_ptr==0) totaldiscs_ptr = tag->value;
        mask[metaptr] = 1;
      }
      else if (!stricmp(tag->item,"genre"))
      {
        if (genre_ptr==0) genre_ptr = tag->value;
        mask[metaptr] = 1;
      }
      else if (!stricmp(tag->item,"tempo"))
      {
        if (tempo_ptr==0) tempo_ptr = tag->value;
        mask[metaptr] = 1;
      }

    }

    if (tracknumber_ptr) membuffer_write_track_tag(buf,"trkn",myatoi(tracknumber_ptr),myatoi(totaltracks_ptr));
    if (discnumber_ptr) membuffer_write_track_tag(buf,"disk",myatoi(discnumber_ptr),myatoi(totaldiscs_ptr));
    if (tempo_ptr) membuffer_write_int16_tag(buf,"tmpo",(uint16_t)myatoi(tempo_ptr));

    if (genre_ptr)
    {
      uint32_t index = mp4ff_meta_genre_to_index(genre_ptr);
      if (index==0)
        membuffer_write_std_tag(buf,"©gen",genre_ptr);
      else
        membuffer_write_int16_tag(buf,"gnre",(uint16_t)index);
    }
  }

  for(metaptr = 0; metaptr < data->count; metaptr++)
  {
    if (!mask[metaptr])
    {
      mp4ff_tag_t * tag = &data->tags[metaptr];
      const char * std_meta_atom = find_standard_meta(tag->item);
      if (std_meta_atom)
      {
        membuffer_write_std_tag(buf,std_meta_atom,tag->value);
      }
      else
      {
        membuffer_write_custom_tag(buf,tag->item,tag->value);
      }
    }
  }

  free(mask);

  if (membuffer_error(buf))
  {
    membuffer_free(buf);
    return 0;
  }

  *out_size = membuffer_get_size(buf);
  *out_buffer = membuffer_detach(buf);
  membuffer_free(buf);

  return 1;
}
//}}}

//{{{
static uint32_t find_atom (mp4ff_t* f, uint64_t base, uint32_t size, const char* name)
{
  uint32_t remaining = size;
  uint64_t atom_offset = base;
  for(;;)
  {
    uint8_t atom_name[4];
    uint32_t atom_size;

    mp4ff_set_position (f, atom_offset);

    if (remaining < 8) break;
    atom_size = mp4ff_read_int32(f);
    if (atom_size > remaining || atom_size < 8) break;
    mp4ff_read_data (f,atom_name,4);

    if (!memcmp(atom_name,name,4))
    {
      mp4ff_set_position(f,atom_offset);
      return 1;
    }

    remaining -= atom_size;
    atom_offset += atom_size;
  }
  return 0;
}
//}}}
//{{{
static uint32_t find_atom_v2 (mp4ff_t* f,uint64_t base, uint32_t size, const char* name,
                              uint32_t extraheaders, const char* name_inside)
{
  uint64_t first_base = (uint64_t)(-1);
  while(find_atom(f,base,size,name))//try to find atom <name> with atom <name_inside> in it
  {
    uint64_t mybase = mp4ff_position(f);
    uint32_t mysize = mp4ff_read_int32(f);

    if (first_base == (uint64_t)(-1)) first_base = mybase;

    if (mysize < 8 + extraheaders) break;

    if (find_atom(f,mybase+(8+extraheaders),mysize-(8+extraheaders),name_inside))
    {
      mp4ff_set_position(f,mybase);
      return 2;
    }
    base += mysize;
    if (size<=mysize) {size=0;break;}
    size -= mysize;
  }

  if (first_base != (uint64_t)(-1))//wanted atom inside not found
  {
    mp4ff_set_position(f,first_base);
    return 1;
  }
  else return 0;
}
//}}}

//{{{
static uint32_t create_meta (const mp4ff_metadata_t * data, void ** out_buffer, uint32_t* out_size)
{
  membuffer * buf;
  uint32_t ilst_size;
  void * ilst_buffer;

  if (!create_ilst(data,&ilst_buffer,&ilst_size)) return 0;

  buf = membuffer_create();

  membuffer_write_int32(buf,0);
  membuffer_write_atom(buf,"ilst",ilst_size,ilst_buffer);
  free(ilst_buffer);

  *out_size = membuffer_get_size(buf);
  *out_buffer = membuffer_detach(buf);
  membuffer_free(buf);
  return 1;
}
//}}}
//{{{
static uint32_t create_udta (const mp4ff_metadata_t * data, void ** out_buffer, uint32_t* out_size)
{
  membuffer * buf;
  uint32_t meta_size;
  void * meta_buffer;

  if (!create_meta(data,&meta_buffer,&meta_size)) return 0;

  buf = membuffer_create();

  membuffer_write_atom(buf,"meta",meta_size,meta_buffer);

  free(meta_buffer);

  *out_size = membuffer_get_size(buf);
  *out_buffer = membuffer_detach(buf);
  membuffer_free(buf);
  return 1;
}
//}}}
//{{{
static uint32_t modify_moov (mp4ff_t* f, const mp4ff_metadata_t* data, uint8_t** out_buffer, uint32_t* out_size)
{
  uint64_t total_base = f->moov_offset + 8;
  uint32_t total_size = (uint32_t)(f->moov_size - 8);

  uint64_t udta_offset,meta_offset,ilst_offset;
  uint32_t udta_size,  meta_size,  ilst_size;

  uint32_t new_ilst_size;
  void * new_ilst_buffer;

  uint8_t * p_out;
  int32_t size_delta;


  if (!find_atom_v2(f,total_base,total_size,"udta",0,"meta"))
  {
    membuffer * buf;
    void * new_udta_buffer;
    uint32_t new_udta_size;
    if (!create_udta(data,&new_udta_buffer,&new_udta_size)) return 0;

    buf = membuffer_create();
    mp4ff_set_position(f,total_base);
    membuffer_transfer_from_file(buf,f,total_size);

    membuffer_write_atom(buf,"udta",new_udta_size,new_udta_buffer);

    free(new_udta_buffer);

    *out_size = membuffer_get_size (buf);
    *out_buffer = (uint8_t*)membuffer_detach (buf);
    membuffer_free(buf);
    return 1;
  }
  else
  {
    udta_offset = mp4ff_position(f);
    udta_size = mp4ff_read_int32(f);
    if (!find_atom_v2(f,udta_offset+8,udta_size-8,"meta",4,"ilst"))
    {
      membuffer * buf;
      void * new_meta_buffer;
      uint32_t new_meta_size;
      if (!create_meta(data,&new_meta_buffer,&new_meta_size)) return 0;

      buf = membuffer_create();
      mp4ff_set_position(f,total_base);
      membuffer_transfer_from_file(buf,f,(uint32_t)(udta_offset - total_base));

      membuffer_write_int32(buf,udta_size + 8 + new_meta_size);
      membuffer_write_atom_name(buf,"udta");
      membuffer_transfer_from_file(buf,f,udta_size);

      membuffer_write_atom(buf,"meta",new_meta_size,new_meta_buffer);
      free(new_meta_buffer);

      *out_size = membuffer_get_size(buf);
      *out_buffer = (uint8_t*)membuffer_detach(buf);
      membuffer_free(buf);
      return 1;
    }
    meta_offset = mp4ff_position(f);
    meta_size = mp4ff_read_int32(f);
    if (!find_atom(f,meta_offset+12,meta_size-12,"ilst")) return 0;//shouldn't happen, find_atom_v2 above takes care of it
    ilst_offset = mp4ff_position(f);
    ilst_size = mp4ff_read_int32(f);

    if (!create_ilst(data,&new_ilst_buffer,&new_ilst_size)) return 0;

    size_delta = new_ilst_size - (ilst_size - 8);

    *out_size = total_size + size_delta;
    *out_buffer = (uint8_t*)malloc(*out_size);
    if (*out_buffer == 0)
    {
      free(new_ilst_buffer);
      return 0;
    }

    p_out = (uint8_t*)*out_buffer;

    mp4ff_set_position(f,total_base);
    mp4ff_read_data(f,p_out,(uint32_t)(udta_offset - total_base )); p_out += (uint32_t)(udta_offset - total_base );
    *(uint32_t*)p_out = fix_byte_order_32(mp4ff_read_int32(f) + size_delta); p_out += 4;
    mp4ff_read_data(f,p_out,4); p_out += 4;
    mp4ff_read_data(f,p_out,(uint32_t)(meta_offset - udta_offset - 8)); p_out += (uint32_t)(meta_offset - udta_offset - 8);
    *(uint32_t*)p_out = fix_byte_order_32(mp4ff_read_int32(f) + size_delta); p_out += 4;
    mp4ff_read_data(f,p_out,4); p_out += 4;
    mp4ff_read_data(f,p_out,(uint32_t)(ilst_offset - meta_offset - 8)); p_out += (uint32_t)(ilst_offset - meta_offset - 8);
    *(uint32_t*)p_out = fix_byte_order_32(mp4ff_read_int32(f) + size_delta); p_out += 4;
    mp4ff_read_data(f,p_out,4); p_out += 4;

    memcpy(p_out,new_ilst_buffer,new_ilst_size);
    p_out += new_ilst_size;

    mp4ff_set_position(f,ilst_offset + ilst_size);
    mp4ff_read_data(f,p_out,(uint32_t)(total_size - (ilst_offset - total_base) - ilst_size));

    free(new_ilst_buffer);
  }
  return 1;

}
//}}}
//}}}
//{{{  sample utils
//{{{
static int32_t mp4ff_chunk_of_sample (const mp4ff_t* f, int track, int sample,
                                      int32_t* chunk_sample, int32_t *chunk) {

  int32_t total_entries = 0;
  int32_t chunk2entry;
  int32_t chunk1, chunk2, chunk1samples, range_samples, total = 0;

  if (f->track[track] == NULL)
    return -1;

  total_entries = f->track[track]->stsc_entry_count;

  chunk1 = 1;
  chunk1samples = 0;
  chunk2entry = 0;

  do {
    chunk2 = f->track[track]->stsc_first_chunk[chunk2entry];
    *chunk = chunk2 - chunk1;
    range_samples = *chunk * chunk1samples;

    if (sample < total + range_samples) break;

    chunk1samples = f->track[track]->stsc_samples_per_chunk[chunk2entry];
    chunk1 = chunk2;

    if(chunk2entry < total_entries) {
        chunk2entry++;
        total += range_samples;
      }
    } while (chunk2entry < total_entries);

  if (chunk1samples)
    *chunk = (sample - total) / chunk1samples + chunk1;
  else
    *chunk = 1;

  *chunk_sample = total + (*chunk - chunk1) * chunk1samples;

  return 0;
  }
//}}}
//{{{
static int32_t mp4ff_chunk_to_offset (const mp4ff_t* f, int track, int32_t chunk) {

 const mp4ff_track_t* p_track = f->track[track];

  if (p_track->stco_entry_count && (chunk > p_track->stco_entry_count))
    return p_track->stco_chunk_offset[p_track->stco_entry_count - 1];
  else if (p_track->stco_entry_count)
    return p_track->stco_chunk_offset[chunk - 1];
  else
    return 8;

  return 0;
  }
//}}}

//{{{
static int32_t mp4ff_sample_range_size (const mp4ff_t* f, int track, int32_t chunk_sample, int sample) {

  int32_t total = 0;
  const mp4ff_track_t* p_track = f->track[track];

  if (p_track->stsz_sample_size)
    return (sample - chunk_sample) * p_track->stsz_sample_size;
  else {
    if (sample >= p_track->stsz_sample_count)
      return 0;//error

    for (int32_t i = chunk_sample, total = 0; i < sample; i++)
      total += p_track->stsz_table[i];
    }

  return total;
  }
//}}}
//{{{
static int32_t mp4ff_sample_to_offset (const mp4ff_t* f, int track, int sample) {

  int32_t chunk;
  int32_t chunk_sample;
  mp4ff_chunk_of_sample (f, track, sample, &chunk_sample, &chunk);

  int32_t chunk_offset1 = mp4ff_chunk_to_offset (f, track, chunk);
  int32_t chunk_offset2 = chunk_offset1 + mp4ff_sample_range_size (f, track, chunk_sample, sample);

  return chunk_offset2;
  }
//}}}

//{{{
static int32_t mp4ff_audio_frame_size (const mp4ff_t* f, int track, int sample) {

  int32_t bytes;
  const mp4ff_track_t * p_track = f->track[track];

  if (p_track->stsz_sample_size)
    bytes = p_track->stsz_sample_size;
   else
    bytes = p_track->stsz_table[sample];

  return bytes;
  }
//}}}
//{{{
static int32_t mp4ff_set_sample_position (mp4ff_t* f, int track, int sample) {

  int32_t offset = mp4ff_sample_to_offset (f, track, sample);
  mp4ff_set_position (f, offset);
  return 0;
  }
//}}}
//}}}
//{{{  atom utils
//{{{
static void mp4ff_track_add (mp4ff_t* f) {

  f->total_tracks++;
  f->track[f->total_tracks - 1] = (mp4ff_track_t*)malloc (sizeof (mp4ff_track_t));
  memset (f->track[f->total_tracks - 1], 0, sizeof (mp4ff_track_t));
  }
//}}}

//{{{
static int32_t mp4ff_read_stsz (mp4ff_t* f) {

  mp4ff_read_char (f); /* version */
  mp4ff_read_int24 (f); /* flags */

  f->track[f->total_tracks - 1]->stsz_sample_size = mp4ff_read_int32 (f);
  f->track[f->total_tracks - 1]->stsz_sample_count = mp4ff_read_int32 (f);

  if (f->track[f->total_tracks - 1]->stsz_sample_size == 0) {
    int32_t i;
    f->track[f->total_tracks - 1]->stsz_table =
      (int32_t*)malloc(f->track[f->total_tracks - 1]->stsz_sample_count*sizeof(int32_t));

    for (i = 0; i < f->track[f->total_tracks - 1]->stsz_sample_count; i++)
      f->track[f->total_tracks - 1]->stsz_table[i] = mp4ff_read_int32 (f);
    }

  return 0;
  }
//}}}
//{{{
static int32_t mp4ff_read_esds (mp4ff_t* f) {

  mp4ff_read_char (f); /* version */
  mp4ff_read_int24 (f); /* flags */

  /* get and verify ES_DescrTag */
  uint8_t tag = mp4ff_read_char (f);
  if (tag == 0x03) {
    /* read length */
    if (mp4ff_read_mp4_descr_length (f) < 5 + 15)
      return 1;
    /* skip 3 bytes */
    mp4ff_read_int24 (f);
    }
  else
    /* skip 2 bytes */
    mp4ff_read_int16 (f);

  // get and verify DecoderConfigDescrTab
  if (mp4ff_read_char (f) != 0x04)
    return 1;

  // read length
  uint32_t temp = mp4ff_read_mp4_descr_length (f);
  if (temp < 13)
    return 1;

  f->track[f->total_tracks - 1]->audioType = mp4ff_read_char (f);
  mp4ff_read_int32 (f);//0x15000414 ????
  f->track[f->total_tracks - 1]->maxBitrate = mp4ff_read_int32 (f);
  f->track[f->total_tracks - 1]->avgBitrate = mp4ff_read_int32 (f);

  // get and verify DecSpecificInfoTag
  if (mp4ff_read_char(f) != 0x05)
    return 1;

  // read length
  f->track[f->total_tracks - 1]->decoderConfigLen = mp4ff_read_mp4_descr_length (f);

  if (f->track[f->total_tracks - 1]->decoderConfig)
    free(f->track[f->total_tracks - 1]->decoderConfig);
  f->track[f->total_tracks - 1]->decoderConfig = (uint8_t*)malloc (f->track[f->total_tracks - 1]->decoderConfigLen);

  if (f->track[f->total_tracks - 1]->decoderConfig)
    mp4ff_read_data (f, f->track[f->total_tracks - 1]->decoderConfig, f->track[f->total_tracks - 1]->decoderConfigLen);
  else
    f->track[f->total_tracks - 1]->decoderConfigLen = 0;

  // skip remainder of atom
  return 0;
  }
//}}}
//{{{
static int32_t mp4ff_read_stsc (mp4ff_t* f) {

  mp4ff_read_char (f); /* version */
  mp4ff_read_int24 (f); /* flags */

  f->track[f->total_tracks - 1]->stsc_entry_count = mp4ff_read_int32 (f);

  f->track[f->total_tracks - 1]->stsc_first_chunk =
    (int32_t*)malloc (f->track[f->total_tracks-1]->stsc_entry_count * sizeof (int32_t));
  f->track[f->total_tracks - 1]->stsc_samples_per_chunk =
    (int32_t*)malloc (f->track[f->total_tracks-1]->stsc_entry_count * sizeof (int32_t));
  f->track[f->total_tracks - 1]->stsc_sample_desc_index =
    (int32_t*)malloc (f->track[f->total_tracks-1]->stsc_entry_count * sizeof (int32_t));

  for (int i = 0; i < f->track[f->total_tracks - 1]->stsc_entry_count; i++) {
    f->track[f->total_tracks - 1]->stsc_first_chunk[i] = mp4ff_read_int32 (f);
    f->track[f->total_tracks - 1]->stsc_samples_per_chunk[i] = mp4ff_read_int32 (f);
    f->track[f->total_tracks - 1]->stsc_sample_desc_index[i] = mp4ff_read_int32 (f);
    }

  return 0;
  }
//}}}
//{{{
static int32_t mp4ff_read_stco (mp4ff_t* f) {

  mp4ff_read_char (f); /* version */
  mp4ff_read_int24 (f); /* flags */
  f->track[f->total_tracks-1]->stco_entry_count = mp4ff_read_int32 (f);

  f->track[f->total_tracks-1]->stco_chunk_offset =
    (int32_t*)malloc (f->track[f->total_tracks-1]->stco_entry_count * sizeof (int32_t));

  for (int i = 0; i < f->track[f->total_tracks-1]->stco_entry_count; i++)
    f->track[f->total_tracks - 1]->stco_chunk_offset[i] = mp4ff_read_int32 (f);

  return 0;
  }
//}}}
//{{{
static int32_t mp4ff_read_ctts (mp4ff_t* f) {

  mp4ff_track_t* p_track = f->track[f->total_tracks - 1];

  if (p_track->ctts_entry_count)
    return 0;

  mp4ff_read_char (f); /* version */
  mp4ff_read_int24 (f); /* flags */
  p_track->ctts_entry_count = mp4ff_read_int32 (f);

  p_track->ctts_sample_count = (int32_t*)malloc (p_track->ctts_entry_count * sizeof (int32_t));
  p_track->ctts_sample_offset = (int32_t*)malloc (p_track->ctts_entry_count * sizeof (int32_t));

  if (p_track->ctts_sample_count == 0 || p_track->ctts_sample_offset == 0) {
    if (p_track->ctts_sample_count) {
      free (p_track->ctts_sample_count);
      p_track->ctts_sample_count = 0;
      }
    if (p_track->ctts_sample_offset) {
      free (p_track->ctts_sample_offset);
      p_track->ctts_sample_offset = 0;
      }
    p_track->ctts_entry_count = 0;
    return 0;
    }
  else {
    for (int i = 0; i < f->track[f->total_tracks - 1]->ctts_entry_count; i++) {
      p_track->ctts_sample_count[i] = mp4ff_read_int32 (f);
      p_track->ctts_sample_offset[i] = mp4ff_read_int32 (f);
      }
    return 1;
    }
  }
//}}}
//{{{
static int32_t mp4ff_read_stts (mp4ff_t* f) {

  mp4ff_track_t * p_track = f->track[f->total_tracks - 1];
  if (p_track->stts_entry_count)
    return 0;

  mp4ff_read_char(f); /* version */
  mp4ff_read_int24(f); /* flags */
  p_track->stts_entry_count = mp4ff_read_int32 (f);
  p_track->stts_sample_count = (int32_t*)malloc (p_track->stts_entry_count * sizeof (int32_t));
  p_track->stts_sample_delta = (int32_t*)malloc (p_track->stts_entry_count * sizeof (int32_t));

  if ((p_track->stts_sample_count == 0) || (p_track->stts_sample_delta == 0)) {
    if (p_track->stts_sample_count) {
      free (p_track->stts_sample_count);
      p_track->stts_sample_count = 0;
      }
    if (p_track->stts_sample_delta) {
      free (p_track->stts_sample_delta);
      p_track->stts_sample_delta = 0;
      }
    p_track->stts_entry_count = 0;
    return 0;
    }
  else {
    for (int i = 0; i < f->track[f->total_tracks - 1]->stts_entry_count; i++) {
      p_track->stts_sample_count[i] = mp4ff_read_int32(f);
      p_track->stts_sample_delta[i] = mp4ff_read_int32(f);
      }
    return 1;
    }
  }
//}}}
//{{{
static int32_t mp4ff_read_mvhd (mp4ff_t* f) {

  mp4ff_read_char(f); /* version */
  mp4ff_read_int24(f); /* flags */

  /* creation_time */ mp4ff_read_int32(f);
  /* modification_time */ mp4ff_read_int32(f);

  f->time_scale = mp4ff_read_int32(f);
  f->duration = mp4ff_read_int32(f);

  /* preferred_rate */ mp4ff_read_int32(f); /*mp4ff_read_fixed32(f);*/
  /* preferred_volume */ mp4ff_read_int16(f); /*mp4ff_read_fixed16(f);*/

  for (int i = 0; i < 10; i++)
    /* reserved */ mp4ff_read_char(f);

  for (int i = 0; i < 9; i++)
    mp4ff_read_int32(f); /* matrix */

  /* preview_time */ mp4ff_read_int32(f);
  /* preview_duration */ mp4ff_read_int32(f);
  /* poster_time */ mp4ff_read_int32(f);
  /* selection_time */ mp4ff_read_int32(f);
  /* selection_duration */ mp4ff_read_int32(f);
  /* current_time */ mp4ff_read_int32(f);
  /* next_track_id */ mp4ff_read_int32(f);

  return 0;
  }
//}}}
//{{{
static int32_t mp4ff_read_tkhd (mp4ff_t* f) {

  uint8_t version = mp4ff_read_char (f); /* version */
  uint32_t flags = mp4ff_read_int24 (f); /* flags */

  if (version == 1) {
    mp4ff_read_int64 (f); // creation-time
    mp4ff_read_int64 (f); // modification-time
    mp4ff_read_int32 (f); // track-id
    mp4ff_read_int32 (f); // reserved
    f->track[f->total_tracks - 1]->duration = mp4ff_read_int64 (f);//duration
    }
  else { //version == 0
    mp4ff_read_int32 (f); // creation-time
    mp4ff_read_int32 (f); // modification-time
    mp4ff_read_int32 (f); // track-id
    mp4ff_read_int32 (f); // reserved
    f->track[f->total_tracks - 1]->duration = mp4ff_read_int32 (f);//duration
    if (f->track[f->total_tracks - 1]->duration == 0xFFFFFFFF)
      f->track[f->total_tracks - 1]->duration = 0xFFFFFFFFFFFFFFFF;
    }

  mp4ff_read_int32 (f); // reserved
  mp4ff_read_int32 (f); // reserved
  mp4ff_read_int16 (f); // layer
  mp4ff_read_int16 (f); // pre-defined
  mp4ff_read_int16 (f); // volume
  mp4ff_read_int16 (f); // reserved

  // matrix
  mp4ff_read_int32 (f);
  mp4ff_read_int32 (f);
  mp4ff_read_int32 (f);
  mp4ff_read_int32 (f);
  mp4ff_read_int32 (f);
  mp4ff_read_int32 (f);
  mp4ff_read_int32 (f);
  mp4ff_read_int32 (f);
  mp4ff_read_int32 (f);

  mp4ff_read_int32 (f); // width
  mp4ff_read_int32 (f); // height
  return 1;
  }
//}}}
//{{{
static int32_t mp4ff_read_mdhd (mp4ff_t* f) {

  uint32_t version = mp4ff_read_int32 (f);
  if (version == 1) {
    mp4ff_read_int64(f);//creation-time
    mp4ff_read_int64(f);//modification-time
    f->track[f->total_tracks - 1]->timeScale = mp4ff_read_int32(f);//timescale
    f->track[f->total_tracks - 1]->duration = mp4ff_read_int64(f);//duration
    }
  else { //version == 0
    mp4ff_read_int32(f);//creation-time
    mp4ff_read_int32(f);//modification-time
    f->track[f->total_tracks - 1]->timeScale = mp4ff_read_int32(f);//timescale
    uint32_t temp = mp4ff_read_int32(f);
    f->track[f->total_tracks - 1]->duration = (temp == (uint32_t)(-1)) ? (uint64_t)(-1) : (uint64_t)(temp);
    }

  mp4ff_read_int16(f);
  mp4ff_read_int16(f);
  return 1;
  }
//}}}

//{{{
static int32_t mp4ff_read_mp4a (mp4ff_t* f, int indent) {

  for (int i = 0; i < 6; i++)
    mp4ff_read_char (f); /* reserved */
  /* data_reference_index */ mp4ff_read_int16 (f);

  mp4ff_read_int32 (f); /* reserved */
  mp4ff_read_int32 (f); /* reserved */

  f->track[f->total_tracks - 1]->channelCount = mp4ff_read_int16 (f);
  f->track[f->total_tracks - 1]->sampleSize = mp4ff_read_int16 (f);

  mp4ff_read_int16(f);
  mp4ff_read_int16(f);

  f->track[f->total_tracks - 1]->sampleRate = mp4ff_read_int16 (f);

  mp4ff_read_int16 (f);

  uint8_t atom_type = 0;
  uint8_t header_size = 0;
  uint64_t size = mp4ff_atom_read_header (f, &atom_type, &header_size, indent);
  if (atom_type == ATOM_ESDS)
    mp4ff_read_esds (f);

  return 0;
  }
//}}}
//{{{
static int32_t mp4ff_read_meta (mp4ff_t* f, uint64_t size, int indent) {

  mp4ff_read_char (f); /* version */
  mp4ff_read_int24 (f); /* flags */

  uint64_t sumsize = 0;
  uint8_t header_size = 0;
  while (sumsize < (size-(header_size + 4))) {
    uint8_t atom_type;
    uint64_t subsize = mp4ff_atom_read_header (f, &atom_type, &header_size, indent);
    if (subsize <= header_size + 4)
      return 1;
    if (atom_type == ATOM_ILST)
      mp4ff_parse_metadata (f, (uint32_t)(subsize - (header_size + 4)), indent);
    else
      mp4ff_set_position (f, mp4ff_position(f) + subsize - header_size);
    sumsize += subsize;
    }

  return 0;
  }
//}}}
//{{{
static int32_t mp4ff_read_stsd (mp4ff_t* f, int indent) {

  mp4ff_read_char (f); /* version */
  mp4ff_read_int24 (f); /* flags */

  f->track[f->total_tracks - 1]->stsd_entry_count = mp4ff_read_int32 (f);

  for (int32_t i = 0; i < f->track[f->total_tracks - 1]->stsd_entry_count; i++) {
    uint64_t skip = mp4ff_position (f);
    uint8_t atom_type = 0;
    uint8_t header_size = 0;
    uint64_t size = mp4ff_atom_read_header (f, &atom_type, &header_size, indent);
    skip += size;

    if (atom_type == ATOM_MP4A) {
      f->track[f->total_tracks - 1]->type = TRACK_AUDIO;
      mp4ff_read_mp4a (f, indent);
      }
    else if (atom_type == ATOM_MP4V)
      f->track[f->total_tracks - 1]->type = TRACK_VIDEO;
    else if (atom_type == ATOM_MP4S)
      f->track[f->total_tracks - 1]->type = TRACK_SYSTEM;
    else
      f->track[f->total_tracks - 1]->type = TRACK_UNKNOWN;

    mp4ff_set_position (f, skip);
    }

  return 0;
  }
//}}}

//{{{
static int32_t mp4ff_atom_read (mp4ff_t* f, int32_t size, uint8_t atom_type, int indent) {

  uint64_t dest_position = mp4ff_position (f) + size - 8;

  if (atom_type == ATOM_STSZ) /* sample size box */
    mp4ff_read_stsz (f);
  else if (atom_type == ATOM_STTS) /* time to sample box */
    mp4ff_read_stts (f);
  else if (atom_type == ATOM_CTTS) /* composition offset box */
    mp4ff_read_ctts (f);
  else if (atom_type == ATOM_STSC) /* sample to chunk box */
    mp4ff_read_stsc (f);
  else if (atom_type == ATOM_STCO) /* chunk offset box */
    mp4ff_read_stco (f);
  else if (atom_type == ATOM_STSD) /* sample description box */
    mp4ff_read_stsd (f, indent);
  else if (atom_type == ATOM_MVHD) /* movie header box */
    mp4ff_read_mvhd (f);
  else if (atom_type == ATOM_MDHD) /* track header */
    mp4ff_read_mdhd (f);
  else if (atom_type == ATOM_META) /* iTunes Metadata box */
    mp4ff_read_meta (f, size, indent);

  mp4ff_set_position (f, dest_position);
  return 0;
  }
//}}}

//{{{
static int32_t parseSubAtoms (mp4ff_t* f, const uint64_t total_size, int indent) {
// parse atoms that are sub atoms of other atoms

  uint64_t counted_size = 0;

  while (counted_size < total_size) {
    uint8_t atom_type = 0;
    uint8_t header_size = 0;
    uint64_t size = mp4ff_atom_read_header (f, &atom_type, &header_size, indent);
    if (size == 0)
      break;
    counted_size += size;

    if (atom_type == ATOM_TRAK) {
      // new track
      f->total_tracks++;
      f->track[f->total_tracks - 1] = (mp4ff_track_t*)malloc (sizeof (mp4ff_track_t));
      memset (f->track[f->total_tracks - 1], 0, sizeof (mp4ff_track_t));
      }

    if (atom_type < SUBATOMIC)
      parseSubAtoms (f, size - header_size, indent + 1);
    else
      mp4ff_atom_read (f, (uint32_t)size, atom_type, indent);
    }

  return 0;
  }
//}}}
//{{{
static int32_t parseAtoms (mp4ff_t* f) {
// parse root atoms

  f->file_size = 0;

  while (true) {
    uint8_t atom_type = 0;
    uint8_t header_size = 0;
    uint64_t size = mp4ff_atom_read_header (f, &atom_type, &header_size, 0);
    if (size == 0)
      break;

    f->file_size += size;
    f->last_atom = atom_type;

    if ((atom_type == ATOM_MDAT) && f->moov_read) {
      // moov atom is before mdat, we can stop reading when mdat is encountered
      // file position will stay at beginning of mdat data
      //break;
      }

    if ((atom_type == ATOM_MOOV) && (size > header_size)) {
      f->moov_read = 1;
      f->moov_offset = mp4ff_position (f) - header_size;
      f->moov_size = size;
      }

    // parse subatoms
    if (atom_type < SUBATOMIC)
      parseSubAtoms (f, size - header_size, 0);
    else // skip this atom
      mp4ff_set_position (f, mp4ff_position (f) + size - header_size);
    }

  return 0;
  }
//}}}
//}}}

//{{{
mp4ff_t* mp4ff_open (mp4ff_callback_t* streamCallbacks, bool debug) {

  mp4ff_t* ff = (mp4ff_t*)malloc (sizeof (mp4ff_t));
  memset (ff, 0, sizeof (mp4ff_t));

  ff->debug = debug;;
  ff->stream = streamCallbacks;
  parseAtoms (ff);

  return ff;
  }
//}}}
//{{{
void mp4ff_close (mp4ff_t* ff) {

  for (int i = 0; i < ff->total_tracks; i++)
    if (ff->track[i]) {
      free (ff->track[i]->stsz_table);
      free (ff->track[i]->stts_sample_count);
      free (ff->track[i]->stts_sample_delta);
      free (ff->track[i]->stsc_first_chunk);
      free (ff->track[i]->stsc_samples_per_chunk);
      free (ff->track[i]->stsc_sample_desc_index);
      free (ff->track[i]->stco_chunk_offset);
      free (ff->track[i]->decoderConfig);
      free (ff->track[i]->ctts_sample_count);
      free (ff->track[i]->ctts_sample_offset);
      free (ff->track[i]);
      }

  for (unsigned int i = 0; i < ff->tags.count; i++) {
    free (ff->tags.tags[i].item);
    free (ff->tags.tags[i].value);
    }
  free (ff->tags.tags);

  free (ff);
  }
//}}}

//{{{
int32_t mp4ff_total_tracks (const mp4ff_t* f) {
  return f->total_tracks;
  }
//}}}

// per track gets
//{{{
int32_t mp4ff_get_track_type (const mp4ff_t* f, int track) {
  return f->track[track]->type;
  }
//}}}
//{{{
int32_t mp4ff_time_scale (const mp4ff_t* f, int track) {
  return f->track[track]->timeScale;
  }
//}}}

//{{{
uint32_t mp4ff_get_avg_bitrate (const mp4ff_t* f, int track) {
  return f->track[track]->avgBitrate;
  }
//}}}
//{{{
uint32_t mp4ff_get_max_bitrate (const mp4ff_t* f, int track) {
  return f->track[track]->maxBitrate;
  }
//}}}

//{{{
int64_t mp4ff_get_track_duration (const mp4ff_t* f, int track) {
  return f->track[track]->duration;
  }
//}}}
//{{{
int64_t mp4ff_get_track_duration_use_offsets (const mp4ff_t* f, int track) {

  int64_t duration = mp4ff_get_track_duration(f,track);
  if (duration!=-1) {
    int64_t offset = mp4ff_get_sample_offset(f,track,0);
    if (offset > duration)
      duration = 0;
    else
      duration -= offset;
    }

  return duration;
  }
//}}}

//{{{
uint32_t mp4ff_get_audio_type (const mp4ff_t* f, int track) {
  return f->track[track]->audioType;
  }
//}}}
//{{{
int32_t mp4ff_num_samples (const mp4ff_t* f, int track) {

  int32_t total = 0;
  for (int i = 0; i < f->track[track]->stts_entry_count; i++)
    total += f->track[track]->stts_sample_count[i];
  return total;
  }
//}}}
//{{{
uint32_t mp4ff_get_sample_rate (const mp4ff_t* f, int track) {
  return f->track[track]->sampleRate;
  }
//}}}
//{{{
uint32_t mp4ff_get_channel_count (const mp4ff_t* f, int track) {
  return f->track[track]->channelCount;
  }
//}}}

//{{{
int32_t mp4ff_get_decoder_config (const mp4ff_t* f, int track, uint8_t** ppBuf, uint32_t* pBufSize) {

  if (track >= f->total_tracks) {
    *ppBuf = NULL;
    *pBufSize = 0;
    return 1;
    }

  if (f->track[track]->decoderConfig == NULL || f->track[track]->decoderConfigLen == 0) {
    *ppBuf = NULL;
    *pBufSize = 0;
    }

  else {
    *ppBuf = (uint8_t*)malloc (f->track[track]->decoderConfigLen);
    if (*ppBuf == NULL) {
      *pBufSize = 0;
      return 1;
      }

    memcpy (*ppBuf, f->track[track]->decoderConfig, f->track[track]->decoderConfigLen);
    *pBufSize = f->track[track]->decoderConfigLen;
    }

  return 0;
  }
//}}}

//{{{
int32_t mp4ff_get_sample_duration_use_offsets (const mp4ff_t* f, int track, int sample) {

  int32_t d = mp4ff_get_sample_duration (f, track, sample);
  if (d != -1) {
    int32_t o = mp4ff_get_sample_offset (f, track, sample);
    if (o > d)
      d = 0;
    else
      d -= o;
    }

  return d;
  }
//}}}
//{{{
int32_t mp4ff_get_sample_duration (const mp4ff_t* f, int track, int sample) {

  int32_t co = 0;

  for (int32_t i = 0; i < f->track[track]->stts_entry_count; i++) {
    int32_t delta = f->track[track]->stts_sample_count[i];
    if (sample < co + delta)
      return f->track[track]->stts_sample_delta[i];
    co += delta;
    }

  return (int32_t)(-1);
  }
//}}}
//{{{
int64_t mp4ff_get_sample_position (const mp4ff_t* f, int track, int sample) {

  int32_t i, co = 0;
  int64_t acc = 0;

  for (i = 0; i < f->track[track]->stts_entry_count; i++) {
    int32_t delta = f->track[track]->stts_sample_count[i];
    if (sample < co + delta) {
      acc += f->track[track]->stts_sample_delta[i] * (sample - co);
      return acc;
      }
    else
      acc += f->track[track]->stts_sample_delta[i] * delta;
    co += delta;
    }

  return (int64_t)(-1);
  }
//}}}
//{{{
int32_t mp4ff_get_sample_offset (const mp4ff_t* f, int track, int sample) {

  int32_t co = 0;
  for (int32_t i = 0; i < f->track[track]->ctts_entry_count; i++) {
    int32_t delta = f->track[track]->ctts_sample_count[i];
    if (sample < co + delta)
      return f->track[track]->ctts_sample_offset[i];
    co += delta;
    }

  return 0;
  }
//}}}

//{{{
int32_t mp4ff_find_sample (const mp4ff_t* f, int track, int64_t offset, int32_t* toskip) {


  int32_t co = 0;
  int64_t offset_total = 0;
  mp4ff_track_t*  p_track = f->track[track];

  for (int32_t i = 0; i < p_track->stts_entry_count; i++) {
    int32_t sample_count = p_track->stts_sample_count[i];
    int32_t sample_delta = p_track->stts_sample_delta[i];
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
int32_t mp4ff_find_sample_use_offsets (const mp4ff_t* f, int track, int64_t offset, int32_t* toskip) {
  return mp4ff_find_sample (f, track, offset + mp4ff_get_sample_offset (f, track, 0), toskip);
  }
//}}}

//{{{
int32_t mp4ff_read_sample_size (mp4ff_t* f, int track, int sample) {

  int32_t temp = mp4ff_audio_frame_size (f, track, sample);
  if (temp < 0)
    temp = 0;
  return temp;
  }
//}}}
//{{{
int32_t mp4ff_read_sample (mp4ff_t* f, int track, int sample, uint8_t* buffer) {

  int32_t size = mp4ff_audio_frame_size (f, track, sample);
  if (size <= 0)
    return 0;

  mp4ff_set_sample_position (f, track, sample);
  return mp4ff_read_data (f, buffer, size);
  }
//}}}

//{{{  meta gets
//{{{
int32_t mp4ff_meta_get_title (const mp4ff_t* f, char** value) {
  return mp4ff_meta_find_by_name (f, "title", value);
  }
//}}}
//{{{
int32_t mp4ff_meta_get_artist (const mp4ff_t *f, char** value) {
  return mp4ff_meta_find_by_name(f, "artist", value);
  }
//}}}
//{{{
int32_t mp4ff_meta_get_writer (const mp4ff_t* f, char** value) {
  return mp4ff_meta_find_by_name(f, "writer", value);
  }
//}}}
//{{{
int32_t mp4ff_meta_get_album (const mp4ff_t* f, char** value)
{
    return mp4ff_meta_find_by_name(f, "album", value);
}
//}}}
//{{{
int32_t mp4ff_meta_get_date (const mp4ff_t* f, char* *value)
{
    return mp4ff_meta_find_by_name(f, "date", value);
}
//}}}
//{{{
int32_t mp4ff_meta_get_tool (const mp4ff_t* f, char** value)
{
    return mp4ff_meta_find_by_name(f, "tool", value);
}
//}}}
//{{{
int32_t mp4ff_meta_get_comment (const mp4ff_t* f, char** value)
{
    return mp4ff_meta_find_by_name(f, "comment", value);
}
//}}}
//{{{
int32_t mp4ff_meta_get_genre (const mp4ff_t* f, char** value)
{
    return mp4ff_meta_find_by_name(f, "genre", value);
}
//}}}
//{{{
int32_t mp4ff_meta_get_track (const mp4ff_t* f, char** value)
{
    return mp4ff_meta_find_by_name(f, "track", value);
}
//}}}
//{{{
int32_t mp4ff_meta_get_totaltracks (const mp4ff_t* f, char** value)
{
    return mp4ff_meta_find_by_name(f, "totaltracks", value);
}
//}}}
//{{{
int32_t mp4ff_meta_get_disc (const mp4ff_t* f, char** value)
{
    return mp4ff_meta_find_by_name(f, "disc", value);
}
//}}}
//{{{
int32_t mp4ff_meta_get_totaldiscs (const mp4ff_t* f, char** value)
{
    return mp4ff_meta_find_by_name(f, "totaldiscs", value);
}
//}}}
//{{{
int32_t mp4ff_meta_get_compilation (const mp4ff_t* f, char** value)
{
    return mp4ff_meta_find_by_name(f, "compilation", value);
}
//}}}
//{{{
int32_t mp4ff_meta_get_tempo (const mp4ff_t* f, char** value)
{
    return mp4ff_meta_find_by_name(f, "tempo", value);
}
//}}}
//{{{
int32_t mp4ff_meta_get_coverart (const mp4ff_t* f, char** value)
{
    return mp4ff_meta_find_by_name(f, "cover", value);
}
//}}}
//{{{
int32_t mp4ff_meta_get_by_index (const mp4ff_t* f, uint32_t index, char** item, char** value) {

  if (index >= f->tags.count) {
    *item = NULL;
    *value = NULL;
    return 0;
    }
  else {
    *item = strdup(f->tags.tags[index].item);
    *value = strdup(f->tags.tags[index].value);
    return 1;
    }
  }
//}}}

//{{{
int32_t mp4ff_meta_get_num_items (const mp4ff_t* f) {
  return f->tags.count;
  }
//}}}
//{{{
int32_t mp4ff_meta_update (mp4ff_callback_t* f, const mp4ff_metadata_t* data) {

  uint8_t* new_moov_data;
  uint32_t new_moov_size;

  mp4ff_t* ff = (mp4ff_t*)malloc (sizeof (mp4ff_t));
  memset (ff, 0, sizeof (mp4ff_t));
  ff->stream = f;
  mp4ff_set_position (ff, 0);

  parseAtoms (ff);

  if (!modify_moov (ff, data, &new_moov_data, &new_moov_size)) {
    mp4ff_close (ff);
    return 0;
    }

  /* copy moov atom to end of the file */
  if (ff->last_atom != ATOM_MOOV) {
    char* free_data = "free";

    /* rename old moov to free */
    mp4ff_set_position (ff, ff->moov_offset + 4);
    mp4ff_write_data (ff, (uint8_t*)free_data, 4);

    mp4ff_set_position (ff, ff->file_size);
    mp4ff_write_int32 (ff, new_moov_size + 8);
    mp4ff_write_data (ff, (uint8_t*)"moov",4);
    mp4ff_write_data (ff, new_moov_data, new_moov_size);
    }

  else {
    mp4ff_set_position (ff, ff->moov_offset);
    mp4ff_write_int32 (ff, new_moov_size + 8);
    mp4ff_write_data (ff, (uint8_t*)"moov",4);
    mp4ff_write_data (ff, new_moov_data, new_moov_size);
    }

  mp4ff_truncate (ff);
  mp4ff_close (ff);

  return 1;
  }
//}}}
//}}}
