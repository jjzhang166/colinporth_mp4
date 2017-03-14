// mp4ffapp.cpp
//{{{  includes
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h> // Sleep

#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "../mp4ff/mp4ff.h"
//}}}

//{{{
uint32_t seekCallback (void* user_data, uint64_t position) {
  return (uint32_t)fseek ((FILE*)user_data, position, SEEK_SET);
  }
//}}}
//{{{
uint32_t readCallback (void* user_data, void* buffer, uint32_t length) {
  return fread (buffer, 1, length, (FILE*)user_data);
  }
//}}}

//{{{
int GetAACTrack (mp4ff_t* mp4ff) {

  // find AAC track
  int numTracks = mp4ff_total_tracks (mp4ff);
  printf ("numTracks:%d\n", numTracks);

  for (int i = 0; i < numTracks; i++) {
    unsigned char* buff = NULL;
    unsigned int buff_size = 0;
    mp4ff_get_decoder_config (mp4ff, i, &buff, &buff_size);
    free (buff);
    return i;
    }

  return -1;
  }
//}}}

int main (int argc, char** argv) {
  //{{{  parse args
  bool mLogInfo = false;
  uint32_t mAlpha = 200;
  uint32_t mScale = 2;
  unsigned int frequency = 0;

  int arg = 1;
  if (argc > 1)
    while (arg < argc) {
      if (!strcmp(argv[arg], "i")) { mLogInfo = true; arg++; }
      else if (!strcmp(argv[arg], "a"))  { arg++; mAlpha = atoi (argv[arg++]); }
      else if (!strcmp(argv[arg], "s"))  { arg++; mScale = atoi (argv[arg++]); }
      else if (!strcmp(argv[arg], "itv")) { frequency = 650000000; arg++; }
      else if (!strcmp(argv[arg], "bbc")) { frequency = 674000000; arg++; }
      else if (!strcmp(argv[arg], "hd"))  { frequency = 706000000; arg++; }
      else break;
      }
  //}}}

  auto mp4File = fopen (argv[arg], "rb");
  if (!mp4File) {
    //{{{  error return
    printf ("unable to open %s\n", argv[arg]);
    return -1;
    }
    //}}}
  //{{{  set callbacks
  auto mp4cb = (mp4ff_callback_t*)malloc (sizeof (mp4ff_callback_t));
  mp4cb->seek = seekCallback;
  mp4cb->read = readCallback;
  mp4cb->user_data = mp4File;
  //}}}

  auto mp4ff = mp4ff_open_read (mp4cb);
  if (!mp4ff) {
    //{{{  error return
    printf ("unable to open mp4ff\n");
    return -1;
    }
    //}}}

  auto track = GetAACTrack (mp4ff);
  printf ("first track:%d\n", track);

  unsigned char* buffer;
  unsigned int buffer_size;
  mp4ff_get_decoder_config (mp4ff, track, &buffer, &buffer_size);

  auto timescale = mp4ff_time_scale (mp4ff, track);
  long samples = mp4ff_num_samples (mp4ff, track);
  printf ("timescale:%d, samples:%d\n", timescale, samples);

  int j = mp4ff_meta_get_num_items (mp4ff);
  for (int k = 0; k < j; k++) {
    char* tag = NULL;
    char* item = NULL;
    if (mp4ff_meta_get_by_index (mp4ff, k, &item, &tag)) {
      if (item != NULL && tag != NULL) {
        printf ("meta:%d %s %s\n", k, item, tag);
        free (item);
        item = NULL;
        free (tag);
        tag = NULL;
        }
      }
    }

  long sampleId = 0;
  auto dur = mp4ff_get_sample_duration (mp4ff, track, sampleId);
  auto rc = mp4ff_read_sample (mp4ff, track, sampleId, &buffer,  &buffer_size);

  mp4ff_close (mp4ff);

  free (mp4cb);
  fclose (mp4File);
  Sleep (10000);
  return 0;
  }
