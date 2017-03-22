// cMp4App.cpp
//{{{  includes
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "cMp4.h"
//}}}

//{{{
uint8_t* makeAdtsHeader (int frameSize, int header_type, int sbr, int channels, int object_type, int sampleRate) {

  const int kAdtsSampleRates[] = { 96000, 88200, 64000, 48000,
                                   44100, 32000, 24000, 22050,
                                   16000, 12000, 11025,  8000,
                                    7350,     0,     0,     0 };
  int sampleRateIndex = 0;
  for (; sampleRateIndex < sizeof(kAdtsSampleRates); sampleRateIndex++)
    if (sampleRate == kAdtsSampleRates[sampleRateIndex])
      break;

  int profile = (object_type - 1) & 0x3;

  int dataSize = 7;
  auto data = (uint8_t*)malloc (dataSize);
  frameSize += dataSize;

  data[0] = 0xFF;                       // 8b: syncword

  data[1] = 0xF1;                       // 4b: syncword
                                        // 1b: mpeg id = 0
                                        // 2b: layer = 0
                                        // 1b: protection absent

  data[2] = ((profile << 6) & 0xC0);    // 2b: profile
  data[2] += ((sampleRateIndex << 2) & 0x3C);  // 4b: sampling_frequency_index
                                        // 1b: private = 0
  data[2] += ((channels >> 2) & 0x1);   // 1b: channel_configuration

  data[3] = ((channels << 6) & 0xC0);   // 2b: channel_configuration
                                        // 1b: original
                                        // 1b: home
                                        // 1b: copyright_id
                                        // 1b: copyright_id_start
  data[3] += ((frameSize >> 11) & 0x3); // 2b: aac_frame_length

  data[4] = ((frameSize >> 3) & 0xFF);  // 8b: aac_frame_length

  data[5] = ((frameSize << 5) & 0xE0);  // 3b: aac_frame_length
  data[5] += ((0x7FF >> 6) & 0x1F);     // 5b: adts_buffer_fullness

  data[6] = ((0x7FF << 2) & 0x3F);      // 6b: adts_buffer_fullness
                                        // 2b: num_raw_data_blocks
  return data;
  }
//}}}

int main (int argc, char** argv) {
  //{{{  parse args
  bool mLogInfo = false;

  int arg = 1;
  if (argc > 1)
    while (arg < argc) {
      if (!strcmp(argv[arg], "i")) {
        mLogInfo = true;
        arg++;
        }
      else
        break;
      }
  //}}}
  auto mp4File = fopen (argv[arg], "rb");
  if (!mp4File) {
    //{{{  error return
    printf ("Mp4App unable to open %s\n", argv[arg]);
    return -1;
    }
    //}}}

  cMp4 mp4 (mp4File, true);
  printf ("file timescale:%lld duration:%lld\n", mp4.getTimeScale(), mp4.getDuration());
  //{{{  get metadata
  auto meta_num_items = mp4.meta_get_num_items();
  for (auto metaItem = 0; metaItem < meta_num_items; metaItem++) {
    char* tag = NULL;
    char* item = NULL;
    if (mp4.meta_get_by_index (metaItem, &item, &tag)) {
      if (item != NULL && tag != NULL)
        printf ("meta:%d %s %s\n", metaItem, item, tag);
      free (item);
      free (tag);
      }
    }
  //}}}
  //{{{  get audio
  auto track = mp4.getAudioTrack();
  auto timeScale = mp4.getTimeScale (track);
  auto numSamples = mp4.getNumSamples (track);
  printf ("audio track:%d timescale:%d, samples:%d\n", track, timeScale, numSamples);

  auto adtsFile = fopen ("C:/Users/colin/Desktop/nnnn.adts", "wb");
  for (auto sample = 0; sample < numSamples; sample++) {
    auto duration = mp4.getSampleDuration (track, sample);
    auto bufferSize = mp4.getSampleSize (track, sample);
    auto buffer = (uint8_t*)malloc (bufferSize);
    auto bytes = mp4.getSample (track, sample, buffer, bufferSize);
    auto adtsHeader = makeAdtsHeader (bufferSize, 0, 0, 2, 2, mp4.getSampleRate (track));
    fwrite (adtsHeader, 1, 7, adtsFile);
    fwrite (buffer, 1, bufferSize, adtsFile);
    free (buffer);
    }
  fclose (adtsFile);
  //}}}

  // get video
  track = mp4.getVideoTrack();
  timeScale = mp4.getTimeScale (track);
  numSamples = mp4.getNumSamples (track);
  printf ("video track:%d timescale:%d, samples:%d\n", track, timeScale, numSamples);
  for (auto sample = 0; sample < numSamples; sample++) {
    auto duration = mp4.getSampleDuration (track, sample);
    auto bufferSize = mp4.getSampleSize (track, sample);
    printf ("sample:%d duration:%d, size:%d\n", sample, duration, bufferSize);
    }

  fclose (mp4File);

  while (true) {}
  return 0;
  }
