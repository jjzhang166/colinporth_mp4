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

  for (int track = 0; track < numTracks; track++) {
    printf ("%d %d\n", track, mp4ff_get_track_type (mp4ff, track));

    unsigned char* buff = NULL;
    unsigned int buff_size = 0;
    mp4ff_get_decoder_config (mp4ff, track, &buff, &buff_size);
    free (buff);
    return track;
    }

  return -1;
  }
//}}}

int adts_sample_rates[] = { 96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350,0,0,0};
//{{{
int findAdtsSRIndex (int sampleRate) {

  for (int i = 0; i < 16; i++)
    if (sampleRate == adts_sample_rates[i])
      return i;

  return 16 - 1;
  }
//}}}
//{{{
unsigned char* makeAdtsHeader (int framesize) {

  // set stuff
  int object_type = 2;
  int samplerate = 44100;
  int header_type = 0;
  int sbr = 0;
  int channels = 2;

  int profile = (object_type - 1) & 0x3;
  int sr_index = findAdtsSRIndex(samplerate);

  int dataSize = 7;
  unsigned char* data = (unsigned char*)malloc (dataSize);
  memset (data, 0, dataSize);

  data[0] += 0xFF;                      /* 8b: syncword */

  data[1] += 0xF0;                      /* 4b: syncword */
                                        /* 1b: mpeg id = 0 */
                                        /* 2b: layer = 0 */
  data[1] += 1;                         /* 1b: protection absent */

  data[2] += ((profile << 6) & 0xC0);   /* 2b: profile */
  data[2] += ((sr_index << 2) & 0x3C);  /* 4b: sampling_frequency_index */
                                        /* 1b: private = 0 */
  data[2] += ((channels >> 2) & 0x1);   /* 1b: channel_configuration */

  data[3] += ((channels << 6) & 0xC0);  /* 2b: channel_configuration */
                                        /* 1b: original */
                                        /* 1b: home */
                                        /* 1b: copyright_id */
                                        /* 1b: copyright_id_start */
  data[3] += ((framesize >> 11) & 0x3); /* 2b: aac_frame_length */

  data[4] += ((framesize >> 3) & 0xFF); /* 8b: aac_frame_length */

  data[5] += ((framesize << 5) & 0xE0); /* 3b: aac_frame_length */
  data[5] += ((0x7FF >> 6) & 0x1F);     /* 5b: adts_buffer_fullness */

  data[6] += ((0x7FF << 2) & 0x3F);     /* 6b: adts_buffer_fullness */
                                        /* 2b: num_raw_data_blocks */
  return data;
  }
//}}}

//{{{
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
  long numSamples = mp4ff_num_samples (mp4ff, track);
  printf ("timescale:%d, samples:%d\n", timescale, numSamples);

  int meta_num_items = mp4ff_meta_get_num_items (mp4ff);
  for (int k = 0; k < meta_num_items; k++) {
    char* tag = NULL;
    char* item = NULL;
    if (mp4ff_meta_get_by_index (mp4ff, k, &item, &tag)) {
      if (item != NULL && tag != NULL)
        printf ("meta:%d %s %s\n", k, item, tag);
      free (item);
      free (tag);
      }
    }

  auto adtsFile = fopen ("C:/Users/colin/Desktop/nnnn.adts", "wb");
  for (long sampleId = 0; sampleId < numSamples; sampleId++) {
    auto duration = mp4ff_get_sample_duration (mp4ff, track, sampleId);
    auto rc = mp4ff_read_sample (mp4ff, track, sampleId, &buffer,  &buffer_size);
    printf ("reading id:%d dur:%d buf:%p bufSize:%d        \r", sampleId, duration, buffer, buffer_size);

    auto adtsHeader = makeAdtsHeader (buffer_size);
    fwrite (adtsHeader, 1, 7, adtsFile);
    fwrite (buffer, 1, buffer_size, adtsFile);
    free (buffer);
    }

  fclose (adtsFile);
  mp4ff_close (mp4ff);

  free (mp4cb);
  fclose (mp4File);

  printf ("\ndone, sleep 10s\n");
  Sleep (10000);
  return 0;
  }
//}}}
