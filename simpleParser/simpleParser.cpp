// MP4Parser.cpp : Defines the entry point for the console application.
//{{{  includes
#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//}}}

#define     MAX_ALLOWED_TRACKS  (5)
const int AheaderNameSize = 7;
char AheaderNames[][6] = { "ftyp", "free", "skip", "wide", "mdat", "moov", "meta" };
typedef enum AheaderTypes{   FTYP,   FREE,   SKIP,   WIDE,   MDAT,   MOOV,  META,  UNKNOWN } eT_AHeader;

const int BMoovNamesSize = 3;
char BMoovNames[][6] = { "mvhd", "drm", "trak" };
typedef enum BMoovTypes{ MVHD,    DRM,  TRAK, MOVUNKNOWN } eT_BMoovType;

const int CTrakNamesSize = 3;
char CTrakNames[][6] = { "tkhd", "clip", "matt", "kmat", "edts", "load", "tref", "imap", "mdia" };
typedef enum CTrakTypes{ TKHD, CLIP, MATT, KMAT, EDTS, LOAD, TREF, IMAP, MDIA, TRKUNKNOWN } eT_CTrakType;

//{{{
eT_AHeader gimmeTypeA (char* sName) {

  int i = 0;
  for( i = 0; i < AheaderNameSize; i++){
    if (strcmp(AheaderNames[i],sName)==0){
      return (eT_AHeader)i;
      }
    }

  return UNKNOWN;
  }
//}}}
//{{{
eT_BMoovType gimmeMoovType (char* sName) {

  int i = 0;
  for ( i = 0; i < BMoovNamesSize; i++){
    if (strcmp(BMoovNames[i],sName)==0){
      return (eT_BMoovType)i;
      }
    }

  return MOVUNKNOWN;
  }
//}}}
//{{{
eT_CTrakType gimmeTrakType (char* sName) {

  int i = 0;
  for ( i = 0; i < CTrakNamesSize; i++){
    if (strcmp (CTrakNames[i],sName)==0){
      return (eT_CTrakType)i;
      }
    }

  return TRKUNKNOWN;
  }
//}}}

//{{{
int findBox (FILE* fpInput, char* sBoxName, unsigned long ulStartOffset, unsigned long ulLookUpSize,
             unsigned long* zulBoxOffset, unsigned long* zulBoxSize, int* pnBoxesFound){

  char s64Buffer[64];
  int  nBoxesFound = 0;

  fseek (fpInput, ulStartOffset+8, SEEK_SET);
  unsigned long ulOffset    = 0;
  unsigned long ulCurrentOffset =  ftell(fpInput);
  while (( ulCurrentOffset >= ulStartOffset ) && (ulCurrentOffset < (ulStartOffset+ulLookUpSize))) {

    printf("\n Am at posiiton : %lu", ulCurrentOffset);
    if (4 == fread( &s64Buffer, 1, 4, fpInput)){ // read long unsigned offset
      s64Buffer[4] = '\0';
      ulOffset =  ((s64Buffer[0]<<24)&0xFF000000)| ((s64Buffer[1]<<16)&0x00FF0000)|
                  ((s64Buffer[2]<<8)&0x0000FF00) | ((s64Buffer[3])&0x000000FF);
      if (4 == fread( s64Buffer, 1, 4, fpInput)){  // read long ASCII text string
        s64Buffer[4] = '\0';
        if (strcmp(s64Buffer, sBoxName) == 0){
          // required box found. add parameters to array.
          printf ("\n INFO: Required Box(%d): %s Found at Position: %lu, with Size: %lu",
                  nBoxesFound, sBoxName, ulCurrentOffset, ulOffset);
          zulBoxSize[nBoxesFound]   = ulOffset;
          zulBoxOffset[nBoxesFound] = ulCurrentOffset;
          nBoxesFound++;
          }
        else{
          if(ulOffset == 0){
            printf("\n Offset is Zero? ");
            break;
            }
          }
        fseek( fpInput, ulOffset-8, SEEK_CUR);
        }
      else{
        printf("\n findBox EOF1");
        break;
        }
      }
    else{
      printf("\n findBox EOF2");
      break;
      }
    ulCurrentOffset =  ftell(fpInput);
    };

  if (nBoxesFound>0){
    *pnBoxesFound = nBoxesFound;
    return 1;
    }

  return -1;
  }
//}}}

//{{{
// Give Paths in the Reverse Order they are to be found  mp4a, stsd, stbl, minf, mdia, trak
int fnFollowPath (int nPaths, char** givenPath, FILE* fpInput,
                  unsigned long  ulStartOffset, unsigned long ulSize,
                  unsigned long* pulReturnOffset, unsigned long* pulReturnSize, int* nBoxes) {

  // next up minf
  unsigned long ultempOffset    = 0;
  unsigned long ultempSize      = 0;
  int       ntempBoxes      = 0;
  if (1 == findBox (fpInput, givenPath[nPaths],
                    ulStartOffset, ulSize, &ultempOffset, &ultempSize, &ntempBoxes)){
    printf ("\n INFO: Found %s with Offset: %lu, Size: %lu",givenPath[nPaths], ultempOffset, ultempSize);
    if (nPaths <=0 ){
      // final box found!!!
      *pulReturnOffset  = ultempOffset;
      *pulReturnSize    = ultempSize;
      *nBoxes       = ntempBoxes;
      return 1;
      }
    else{
      return fnFollowPath ((nPaths -1), givenPath, fpInput, ultempOffset, ultempSize, pulReturnOffset, pulReturnSize, nBoxes);
      }
    }
  else{
    return -1;
    }
  }
//}}}

//{{{
// get string from file, with Size.
static int fnGetString(FILE* fpInput, unsigned int nSizeText, char* sStrBuffer){

  if (nSizeText == fread( sStrBuffer, 1, nSizeText, fpInput)){
    sStrBuffer[nSizeText] = '\0';
    return nSizeText;
    }

  return -1;
  }
//}}}
//{{{
static int fnGetUnsignedShort(FILE* fpInput, unsigned short* value){

  char sBuffer[5];
  if (-1 != fnGetString(fpInput, 2, sBuffer)){
    *value =  ((sBuffer[0]<<8)&0xFF00) | ((sBuffer[1])&0x00FF);
    return 1;
    }
  return -1;
  }
//}}}
//{{{
static int fnGetUnsignedLong(FILE* fpInput, unsigned long* value){

  char sBuffer[5];
  if (-1 != fnGetString(fpInput, 4, sBuffer)){
    *value =  ((sBuffer[0]<<24)&0xFF000000)|
          ((sBuffer[1]<<16)&0x00FF0000)|
          ((sBuffer[2]<<8)&0x0000FF00)|
          ((sBuffer[3])&0x000000FF);
    return 1;
    }

  return -1;
  }
//}}}

//{{{
typedef struct {
    unsigned long nSampleLength;
    int       bMPEGVersion; // 0
    int       bIsCRCPresent;  // 0
    int       nAudioObjectType;// 2
    unsigned int nSamplingFrequency;// 4
    unsigned int nMPEG4ChannelConfig;// 2
}stADTSHeaderInfo;
//}}}

//{{{
int fnPrepareADTSPacket (stADTSHeaderInfo* pstHeader, int* nOutHeaderSize, unsigned char** ppOutHeader){

  int bMPEGVersion    = 0;  // MPEG Version: 0 for MPEG-4, 1 for MPEG-2
  int bIsCRCPresent   = 0;  // CRC present  1, else 0
  int nAudioObjectType  = 2;  // http://wiki.multimedia.cx/index.php?title=MPEG-4_Audio#Audio_Object_Types
  unsigned int nSamplingFrequency   = 4;  // 4: 44100 Hz
  unsigned int nMPEG4ChannelConfig  = 2;  // 2: 2 channels: front-left, front-right
  unsigned long nPacketLength     = 0;
  unsigned char cTempByte       = 0;
  unsigned long nSampleLength     = pstHeader->nSampleLength;
  // from http://wiki.multimedia.cx/index.php?title=ADTS
  // AAAAAAAA AAAABCCD EEFFFFGH HHIJKLMM MMMMMMMM MMMOOOOO OOOOOOPP (QQQQQQQQ QQQQQQQQ)
  unsigned char* sADTSHeader = *ppOutHeader;
  //sADTSHeader = (unsigned char*)malloc(sizeof(char)* 9);
  if(sADTSHeader == NULL){
    printf("\n ERROR: Insufficient Memory for ADTS Header");
    return -1;
  }
  sADTSHeader[0] = 0xFF;
  sADTSHeader[1] = 0xFF;
  // B   1   MPEG Version: 0 for MPEG-4, 1 for MPEG-2
  if(bMPEGVersion == 0)
    sADTSHeader[1] &= ~(1 << 3); // clear bit at position 3.
  else
    sADTSHeader[1] |= 1 << 3; // set bit at position 3.

  sADTSHeader[1] &= ~(1<<2);  // C
  sADTSHeader[1] &= ~(1<<1);  // C
  if(bIsCRCPresent == 0)  // Warning, set to 1 if there is no CRC and 0 if there is CRC
    sADTSHeader[1] |= 1 << 0; // D
  else
    sADTSHeader[1] &= ~(1 << 0);  // D

  // 3rd Byte
  sADTSHeader[2]  = (nAudioObjectType-1); // profile, the MPEG-4 Audio Object Type minus 1
  sADTSHeader[2]  = sADTSHeader[2]<<6;          // E
  sADTSHeader[2]  &= ~0x3F; // clear the last 6 bits  // F
  sADTSHeader[2]  |= (nSamplingFrequency<<2) & 0x3F;    // F
  sADTSHeader[2]  &= ~(1<<1); // G, private bit
  if(nMPEG4ChannelConfig & (1<<2)){
    sADTSHeader[2]  |= (1<<0);
  }else{
    sADTSHeader[2]  &= ~(1<<0);
  }

  // 4th Byte
  if(nMPEG4ChannelConfig & (1<<1)){
    sADTSHeader[3]  |= (1<<7);
  }else{
    sADTSHeader[3]  &= ~(1<<7);
  }
  if(nMPEG4ChannelConfig & (1<<0)){
    sADTSHeader[3]  |= (1<<6);
  }else{
    sADTSHeader[3]  &= ~(1<<6);   // H
  }
  sADTSHeader[3]  &= ~(1<<5); // I , originality, originality, set to 0 when encoding, ignore when decoding
  sADTSHeader[3]  &= ~(1<<4); // J , home, set to 0 when encoding, ignore when decoding
  sADTSHeader[3]  &= ~(1<<3); // K, copyrighted stream, set to 0 when encoding, ignore when decoding
  sADTSHeader[3]  &= ~(1<<2); // L, copyright start, set to 0 when encoding, ignore when decoding
  if(bIsCRCPresent == 0){
    nPacketLength = 7;
  }else{
    nPacketLength = 9;
  }
  nPacketLength += nSampleLength;
  if(nPacketLength & (1<<12)){
    sADTSHeader[3]  |= (1<<1);  // M
  }else{
    sADTSHeader[3]  &= ~(1<<1);
  }
  if(nPacketLength & (1<<11)){
    sADTSHeader[3]  |= (1<<0);
  }else{
    sADTSHeader[3]  &= ~(1<<0); // M
  }

  // 5th Byte, all M's
  sADTSHeader[4] = (nPacketLength>>3 & 0xFF); // M

  // 6th Byte
  if(nPacketLength & (1<<2)){
    sADTSHeader[5]  |= (1<<7);
  }else{
    sADTSHeader[5]  &= ~(1<<7);
  }
  if(nPacketLength & (1<<1)){
    sADTSHeader[5]  |= (1<<6);
  }else{
    sADTSHeader[5]  &= ~(1<<6);
  }
  if(nPacketLength & (1<<0)){
    sADTSHeader[5]  |= (1<<5);
  }else{
    sADTSHeader[5]  &= ~(1<<5); // M
  }

  // Buffer fullness, set to 0x7FF for VBR
  sADTSHeader[5]  |=  0x1F;   // 0

  // 7th Byte, since number of ADTS frames are always 1.
  // Number of AAC frames (RDBs) in ADTS frame minus 1, for maximum compatibility always use 1 AAC frame per ADTS frame
  sADTSHeader[6]    = 0xFC;


  *nOutHeaderSize   = 7;
  //*ppOutHeader    = sADTSHeader;
  return 1;
}
//}}}
//{{{
int fnParseSTSZForSamples (FILE* fp, unsigned long ulSTSZOffset, unsigned long ulSTSZSize,
                           unsigned long* pulTotalSamples, unsigned long* pnSampleSize, unsigned long** ppulSampleSizeArray){

  unsigned long ulSampleSize    = 0;
  unsigned long ulNumberSamples = 0;
  unsigned long ulCurSampleSize = 0;
  unsigned long *zulSampleSizes = NULL;

  fseek (fp, ulSTSZOffset +(8) + 4, SEEK_SET);// (size+name) +(version + flags)
  if (1 == fnGetUnsignedLong(fp, &ulSampleSize) ){
    printf("\n INFO: Obtained Sample Size as : %lu ", ulSampleSize);
    if( 1 ==  fnGetUnsignedLong(fp, &ulNumberSamples) ){
      printf("\n INFO: Obtained the Number of Samples as : %lu ", ulNumberSamples);
      if(ulSampleSize != 0){
        printf("\n INFO: Constant Header Size ..(%lu)", ulSampleSize);
        *pulTotalSamples  = ulNumberSamples;
        *pnSampleSize   = ulSampleSize;
        return 1;
      }else{
        zulSampleSizes = (unsigned long*)malloc(sizeof(unsigned long)*(ulNumberSamples+1));
        if(zulSampleSizes == NULL){
          printf("\n ERROR: Memory Allocation failure when attempting to create : zulSampleSizes");
          return -1;
        }
      }
      unsigned long i = 0;
      for( i = 0; i < ulNumberSamples; i++){
        if( 1 ==  fnGetUnsignedLong(fp, &ulCurSampleSize)){
          zulSampleSizes[i] = ulCurSampleSize;
        }else{
          printf("\n ERROR: Failed when attempting to obtain Sample : %lu", i);
          return -1;
        }
      }
    }else{
      printf("\n ERROR: STSZ Failed to Obtain Number Of Samples ");
      return -1;
    }
  }else{
    printf("\n ERROR: STSZ Failed to Obtain Sample Size");
    return -1;
  }
  *pulTotalSamples    = ulNumberSamples;
  *pnSampleSize     = 0;
  *ppulSampleSizeArray  = zulSampleSizes;
  return 1;
}
//}}}
//{{{
int fnParseSTCOForChunks (FILE* fp, unsigned long ulSTCOOffset,unsigned long ulSTCOSize,
                          unsigned long* pulTotalChunks, unsigned long **ppulChunkOffset){

  fseek (fp, ulSTCOOffset + (8) + (4), SEEK_SET); // (offset + text) + (1 byte version + 3 byte flags)
  unsigned long ulTotalOffsetsCount = 0;
  unsigned long* pulChunkOffset   = NULL;

  if( 1 == fnGetUnsignedLong(fp, &ulTotalOffsetsCount) ){
    printf("\n Total Number Of Chunks are : %lu", ulTotalOffsetsCount);
    if(pulChunkOffset == NULL && (ulTotalOffsetsCount>0)){
      unsigned long ulCount = 0;
      unsigned long ulOffset  = 0;
      pulChunkOffset = (unsigned long*)malloc(sizeof(unsigned long)*ulTotalOffsetsCount);
      if(pulChunkOffset != NULL){
        for(ulCount = 0; ulCount < ulTotalOffsetsCount; ulCount++){
          if( 1 == fnGetUnsignedLong( fp, &ulOffset)){
            pulChunkOffset[ulCount] = ulOffset;
          }else{
            printf("\n ERROR: STCO parsing 2 failed");
            return -1;
          }
        }// for
      }else{
        printf("\n ERROR: STCO Memory Allocation failed");
        return -1;
      }
    }else{
      printf("\n ERROR: This Function Expects a NULL Pointer for pulChunkOffset(%lu)",ulTotalOffsetsCount);
      return -1;
    }
  }else{
    printf("\n ERROR: STCO parsing failed");
    return -1;
  }
  *pulTotalChunks   = ulTotalOffsetsCount;
  *ppulChunkOffset  = pulChunkOffset;
  return 1;
}
//}}}
//{{{
int fnParseSTSCForSamples (FILE* fp, unsigned long ulSTSCOffset,unsigned long ulSTSCSize,
                           unsigned long* nTotalChunks, unsigned long *nSamplesInChunk){

  fseek(fp, ulSTSCOffset+(8)+(4), SEEK_SET); // (size + text) + ( 4 bytes version)

  unsigned long nChunks       = 0;
  unsigned long nTotalBlocks    = 0;
  unsigned long nCurrentBlock   = 0;
  unsigned long ulFirstBlock    = 0;
  unsigned long ulNextBlock     = 0;
  unsigned long ulNFrames     = 0;
  unsigned long ulNBFrames      = 0;
  unsigned long ulDescriptionID   = 0;
  unsigned long ulNBDescriptionID = 0;

  if(1 == fnGetUnsignedLong(fp, &nTotalBlocks)){
    printf("\n INFO: STSC Entry Count is : %d", nTotalBlocks);
    if(1 == fnGetUnsignedLong(fp, &ulFirstBlock)){
      do{
        nCurrentBlock++;
        if(1 == fnGetUnsignedLong(fp, &ulNFrames)){
          if(1 == fnGetUnsignedLong(fp, &ulDescriptionID)){
            if(nCurrentBlock < nTotalBlocks){
              if(1 == fnGetUnsignedLong(fp, &ulNextBlock)){
                while(nChunks<(ulNextBlock -1)){
                  nSamplesInChunk[nChunks] =  ulNFrames;
                  nChunks++;
                };
              }else{// ulNextBlock
                printf("\n ERROR: Get Next Block Failed!!!");
                return -1;
              }
            }else{
              nSamplesInChunk[nChunks] =  ulNFrames;
              nChunks++;
            }
          }// ulDescriptionID
        }// ulNFrames
      }while(nCurrentBlock < nTotalBlocks);
    }// ulFirstBlock
  }// nTotalBlocks
  *nTotalChunks = nChunks;
  return 1;
}
//}}}

//{{{
int main (int argc, char* argv[]) {

  char      s64Buffer[64];

  FILE*     fpInput     = fopen (argv[1], "rb");
  unsigned long ulOffset    = 0;
  unsigned long ulCurrentOffset = 0;
  int       bWasFTYPFound = 0;
  int       bWasMDATFound = 0;
  int       bWasMOOVFound = 0;
  unsigned long ulFTYPOffset  = 0;
  unsigned long ulMDATOffset  = 0;
  unsigned long ulMOOVOffset  = 0;
  unsigned long ulFTYPSize    = 0;
  unsigned long ulMDATSize    = 0;
  unsigned long ulMOOVSize    = 0;
  int       nTracksInFile = 0;
  unsigned long*  zulTrackOffset;
  unsigned long*  zulTrackSize;

  int nRequiredTrack = -1;
  int nSizePathToTrace = 3;
  char** pzcPathToTrace  = NULL;

  char zzcPathToTrace[][5] = {"stbl", "minf", "mdia"};
  pzcPathToTrace = (char**)malloc(sizeof(char*)*nSizePathToTrace);

  int i = 0;
  for (i = 0; i< nSizePathToTrace; i++){
    pzcPathToTrace[i] = (char*)malloc(sizeof(char)*sizeof(zzcPathToTrace[i]));
    strcpy(pzcPathToTrace[i], zzcPathToTrace[i]);

    }

  if (fpInput){
    fseek (fpInput, 0, SEEK_SET);
    while (!feof(fpInput)){
      ulCurrentOffset =  ftell (fpInput);
      printf ("\n Am at posiiton : %lu", ulCurrentOffset);
      if (4 == fread( &s64Buffer, 1, 4, fpInput)) { // read long unsigned offset
        s64Buffer[4] = '\0';
        ulOffset =  ((s64Buffer[0]<<24)&0xFF000000)|
              ((s64Buffer[1]<<16)&0x00FF0000)|
              ((s64Buffer[2]<<8)&0x0000FF00)|
              ((s64Buffer[3])&0x000000FF);

        if(4 == fread( s64Buffer, 1, 4, fpInput)){  // read long ASCII text string
          s64Buffer[4] = '\0';
          switch(gimmeTypeA(s64Buffer)){
            case FTYP:
              bWasFTYPFound = 1;
              ulFTYPOffset  = ulCurrentOffset;
              ulFTYPSize    = ulOffset;
              printf("\n INFO: FTYP found with size %lu %x", ulOffset, ulOffset);
              break;
            case FREE:
            case SKIP:
            case WIDE:
              printf("\n INFO: FREE found with size %lu", ulOffset);
              break;
            case MDAT:
              bWasMDATFound = 1;
              ulMDATOffset  = ulCurrentOffset;
              ulMDATSize    = ulOffset;
              printf("\n INFO: MDAT found with size %lu", ulOffset);
              break;
            case MOOV:
              bWasMOOVFound = 1;
              ulMOOVOffset  = ulCurrentOffset;
              ulMOOVSize    = ulOffset;
              printf("\n INFO: MOOV found with size %lu", ulOffset);
              break;
            case META:
              printf("\n INFO: META found with size %lu", ulOffset);
              break;
            default:
              printf("\n INFO: Unknown found with size %lu", ulOffset);
              break;
            }
          fseek( fpInput, ulOffset-8, SEEK_CUR);
          }
        else{
          printf("\n EOF");
          break;
          }
        } 
      else{
        printf("\n 1st EOF");
        break;
        }
      };

    // If we have found all the main boxes, let us investigate the moov box
    if( (bWasFTYPFound == 1) && (bWasMDATFound == 1) && (bWasMOOVFound == 1)) {
      // Let's Browse MOOV box and find trak
      zulTrackOffset  = (unsigned long*)malloc(sizeof(unsigned long)*MAX_ALLOWED_TRACKS);
      if( NULL != zulTrackOffset){
        zulTrackSize  = (unsigned long*)malloc(sizeof(unsigned long)*MAX_ALLOWED_TRACKS);
        if( NULL != zulTrackSize){
          if(1 == findBox( fpInput, "trak", ulMOOVOffset, ulMOOVSize, zulTrackOffset, zulTrackSize, &nTracksInFile)){
            int i = 0;
            int       nSTSDBoxes    = 0;
            int       nSTBLBoxes    = 0;
            unsigned long ulSTSDSize    = 0;
            unsigned long ulSTSDOffset  = 0;
            unsigned long ulMP4ABoxSize = 0;
            unsigned long ulMP4ABoxOffset = 0;
            unsigned long ulSTBLBoxOffset = 0;
            unsigned long ulSTBLBoxSize = 0;
            for( i = 0; i < nTracksInFile; i++){
              printf("\n INFO: Track: %d, Size: %lu, Offset: %lu",
                i, zulTrackSize[i], zulTrackOffset[i]);
              // for each track, dig in till you find the required track. 'mp4a'
              if( 1 == fnFollowPath((nSizePathToTrace-1), pzcPathToTrace,
                 fpInput, zulTrackOffset[i], zulTrackSize[i],
                 &ulSTBLBoxOffset, &ulSTBLBoxSize,
                 &nSTBLBoxes
                 )){
                   printf("\n INFO: Required Track Found!!! STBL at Offset: %lu, Size: %lu",
                     ulSTBLBoxOffset, ulSTBLBoxSize);
                   if(1 == findBox( fpInput, "stsd", ulSTBLBoxOffset, ulSTBLBoxSize,
                        &ulSTSDOffset, &ulSTSDSize, &nSTSDBoxes)){
                       printf("\n INFO: Required Track Found!!! STSD at Offset: %lu, Size: %lu",
                         ulSTSDOffset, ulSTSDSize);
                       // find if this is the stsd we are interested in
                      fseek( fpInput, ulSTSDOffset+(8)+(8), SEEK_SET);// (offset+stsd)+(version+description), length,mp4a
                      ulMP4ABoxOffset = ftell(fpInput);
                      fnGetUnsignedLong(fpInput, &ulMP4ABoxSize);
                      fseek( fpInput, ulSTSDOffset+(8)+(8)+ (4), SEEK_SET);// (offset+stsd)+(version+description)+ length,mp4a
                      // if this contains the string 'mp4a' you are done.. :)
                      if(4 == fread( s64Buffer, 1, 4, fpInput)){  // read long ASCII text string
                        s64Buffer[4] = '\0';
                        if(strcmp(s64Buffer, "mp4a") == 0){
                          // others are enca, samr, sawb
                          printf("\n :) You Have the Required AAC Track in this file");
                          nRequiredTrack = i;
                          int       nESDSBoxes      = 0;
                          unsigned short  usChannels      = 0;
                          unsigned short  usSampleSize    = 0;
                          unsigned short  sAudioCId     = 0;
                          unsigned short  sAudioPacketSize  = 0;
                          unsigned long ulAudioSampleRate = 0;
                          unsigned long ulESDSOffset    = 0;
                          unsigned long   ulESDSSize      = 0;
                          unsigned long ultempVariable    = 0;
                          unsigned long ulMaxBitRate    = 0;
                          unsigned long ulAvgBitRate    = 0;
                          // You can continue reading the esds decriptor from this location..
                          // 6 bytes reserved = 48-bit value set to zero, 2 bytes data reference index
                          // 2 bytes QUICKTIME audio encoding version, 2 bytes QUICKTIME audio encoding revision level
                          fseek(fpInput, (6+2+2+2), SEEK_CUR);
                          // 4 bytes QUICKTIME audio encoding vendor
                          if(-1 != fnGetString(fpInput, 4, s64Buffer)){
                            printf("\n Vendor : %s ",s64Buffer);
                          }
                          // 2 bytes audio channels = short unsigned count, (mono = 1 ; stereo = 2)
                          if(-1 != fnGetUnsignedShort(fpInput, &usChannels)){
                            printf("\n Audio Channels are : %hu",usChannels);
                          }
                          // 2 bytes audio sample size,  (8 or 16)
                          if(-1 != fnGetUnsignedShort(fpInput, &usSampleSize)){
                            printf("\n Audio Sample Size : %hu",usSampleSize);
                          }
                          // 2 bytes QUICKTIME audio compression id = short integer value
                          if(-1 != fnGetUnsignedShort(fpInput, &sAudioCId)){ // WRONG UNSIGNED vs SIGNED
                            printf("\n Audio Compression ID : %hu",sAudioCId);
                          }
                          //  2 bytes QUICKTIME audio packet size = short value set to zero
                          if(-1 != fnGetUnsignedShort(fpInput, &sAudioPacketSize)){ // WRONG UNSIGNED vs SIGNED
                            printf("\n Audio Packet Size : %hu",sAudioPacketSize);
                          }
                          // 4 bytes audio sample rate
                          if(-1 != fnGetUnsignedLong(fpInput, &ulAudioSampleRate)){
                            printf("\n Audio SampleRate: %lu", ulAudioSampleRate);
                          }
                          ulCurrentOffset = ftell(fpInput)-8;
                          ulOffset    = (ulMP4ABoxSize-(ulCurrentOffset - ulMP4ABoxOffset));
                          // ulMP4ABoxOffset + <somebytes> = ulCurrentOffset;
                          //
                          if(1 == findBox( fpInput, "esds", ulCurrentOffset, ulOffset, &ulESDSOffset, &ulESDSSize, &nESDSBoxes)){
                            printf("\n Found the ESDS box at : %lu with Size: %lu", ulESDSOffset, ulESDSSize);
                            // 4 bytes version/flags, 1 byte ES descriptor , 3 bytes extended descriptor type tag
                            // 1 byte descriptor type length
                            // // 2 bytes ES ID, 1 byte stream priority,
                            // // 1 byte decoder config descriptor type , 3 bytes extended descriptor type
                            // // 1 byte descriptor type length
                            fseek(fpInput,(ulESDSOffset+8+9+8), SEEK_SET);
                            printf("\n DEBUG : Am at position: %lu",ftell(fpInput));
                            unsigned char descriptor;
                            fread(&descriptor, 1, 1, fpInput);
                            printf("\n Obtained descriptor to be : %u", descriptor);
                            if(descriptor == 64)  printf("\n MPEG-4 audio");
                            if(descriptor == 102) printf("\n MPEG-4 ADTS main");
                            if(descriptor == 103) printf("\n MPEG-4 ADTS Low Complexity");
                            if(descriptor == 104) printf("\n MPEG-4 ADTS Scalable Sampling Rate");
                            if(descriptor == 105) printf("\n MPEG-2 ADTS");
                            if(descriptor == 107) printf("\n MPEG-1 ADTS");
                            if(descriptor == 107) printf("\n MPEG-1 ADTS");
                            if(descriptor == 192) printf("\n private audio");
                            if(descriptor == 224) printf("\n16-bit PCM LE audio");
                            if(descriptor == 225) printf("\n vorbis audio");
                            if(descriptor == 226) printf("\n dolby v3 (AC3) audio");
                            if(descriptor == 227) printf("\n alaw audio");
                            if(descriptor == 228) printf("\n mulaw audio");
                            if(descriptor == 229) printf("\n  G723 ADPCM audio");
                            if(descriptor == 230) printf("\n 16-bit PCM Big Endian audio");
                            // 1 complicated byte
                            // 3 bytes buffer size;
                            fseek(fpInput, 4, SEEK_CUR);
                            // 4 bytes maximum bit rate
                            if(-1 != fnGetUnsignedLong(fpInput, &ulMaxBitRate)){
                              printf("\n Max BitRate: %lu", ulMaxBitRate);
                            }
                            // 4 bytes average bit rate
                            if(-1 != fnGetUnsignedLong(fpInput, &ulAvgBitRate)){
                              printf("\n Avg BitRate: %lu", ulAvgBitRate);
                            }
                          }// findBox( fpInput, "esds"
                          // Also find and save the offsets for the stts, stsd, stsz, stsc, stco
                          // You have the stbl info, you can now get the needed stsc, stco
                          int       nSTSCBoxes    = 0;
                          int       nSTCOBoxes    = 0;
                          int       nSTSZBoxes    = 0;
                          unsigned long ulSTCOOffset  = 0;
                          unsigned long ulSTCOSize    = 0;
                          unsigned long ulSTSCOffset  = 0;
                          unsigned long ulSTSCSize    = 0;
                          unsigned long ulSTSZOffset  = 0;
                          unsigned long ulSTSZSize    = 0;
                          if(1 == findBox( fpInput, "stsc", ulSTBLBoxOffset, ulSTBLBoxSize,
                                  &ulSTSCOffset, &ulSTSCSize, &nSTSCBoxes)){
                              printf("\n Found STSC with Offset: %lu, Size: %lu",
                                ulSTSCOffset, ulSTSCSize);
                            if(1 == findBox( fpInput, "stco", ulSTBLBoxOffset, ulSTBLBoxSize,
                                  &ulSTCOOffset, &ulSTCOSize, &nSTCOBoxes)){
                              printf("\n Found STSC with Offset: %lu, Size: %lu",
                                ulSTCOOffset, ulSTCOSize);
                              if(1 == findBox( fpInput, "stsz", ulSTBLBoxOffset, ulSTBLBoxSize,
                                    &ulSTSZOffset, &ulSTSZSize, &nSTSZBoxes)){
                                printf("\n Found STSZ with Offset: %lu, Size: %lu",
                                  ulSTSZOffset, ulSTSZSize);
                                unsigned long nTotalChunks  = 0;
                                unsigned long*  nSamplesInChunk = NULL;
                                // TODO: Add a function to find max chunks to allocate memory correctly
                                nSamplesInChunk = (unsigned long*)malloc(sizeof(unsigned long)* 100);
                                if(nSamplesInChunk != NULL){
                                  if( 1 ==  fnParseSTSCForSamples( fpInput, ulSTSCOffset, ulSTSCSize,
                                    &nTotalChunks, nSamplesInChunk)){
                                    unsigned long i = 0;
                                    printf("\n INFO: TotalChunks are : %lu", nTotalChunks);
                                    for( i = 0; i < nTotalChunks; i++){
                                      printf("\n INFO: nSamplesInChunk[%lu] = %lu ",
                                        i, nSamplesInChunk[i]);
                                    }
                                    unsigned long ulSTCOTotalChunks = 0;
                                    unsigned long* pulSTCOChunks  = NULL;
                                    if( 1 == fnParseSTCOForChunks( fpInput, ulSTCOOffset, ulSTCOSize,
                                          &ulSTCOTotalChunks, &pulSTCOChunks)){
                                      printf("\n INFO: Obatained STCO Chunks as : %lu ",ulSTCOTotalChunks);
                                      for( i = 0; i < ulSTCOTotalChunks; i++){
                                        printf("\n INFO: STCOOffset[%lu] = %lu ",
                                          i, pulSTCOChunks[i]);
                                      }
                                      unsigned long ulSTSZTotalSamples  = 0;
                                      unsigned long ulSTSZSampleSize    = 0;
                                      unsigned long* ulSTSZSampleArray  = 0;
                                      if( 1 == fnParseSTSZForSamples(fpInput, ulSTSZOffset, ulSTSZSize,
                                        &ulSTSZTotalSamples, &ulSTSZSampleSize, &ulSTSZSampleArray)){
                                          printf("\n INFO: Total Samples in STSZ are : %lu", ulSTSZSampleSize);
                                          if(ulSTSZSampleSize != 0){
                                            printf("\n INFO: Constant Sample Size(%lu) for all (%lu)Samples",
                                                ulSTSZSampleSize, ulSTSZTotalSamples);
                                          }else{
                                            for( i = 0; i < ulSTSZTotalSamples; i++){
                                              printf("\n INFO: Sample Size of (%lu) is (%lu)", i, ulSTSZSampleArray[i] );
                                            }// for ulSTSZTotalSamples
                                          }
                                          // :) you can now actually write a valid .aac file to disk.. Good luck
                                          unsigned long ulCurActiveChunk  = 0;
                                          unsigned long ulCurSamples    = 0;
                                          stADTSHeaderInfo stHeader;
                                          int nHeaderSize = 0;
                                          unsigned char* zAACHeader = NULL;
                                          zAACHeader = (unsigned char*)malloc(sizeof(unsigned char) * 10);
                                          if(zAACHeader == NULL){
                                            printf("\n ERROR: Insufficient Memory when attempting to create : zAACHeader");
                                          }else{
                                            unsigned char* pcOutBuffer = (unsigned char*)malloc(sizeof(char)*1024); // 1024 bytes should be enough for any sample
                                            if(NULL != pcOutBuffer){
                                              FILE* fpOut = fopen("audio.aac","wb");
                                              if(fpOut){
                                                fseek(fpInput, pulSTCOChunks[ulCurActiveChunk], SEEK_SET);
                                                for( i = 0; i < ulSTSZTotalSamples; i++){ // ulSTSZTotalSamples
                                                  if (ulSTSZSampleArray[i] == fread(pcOutBuffer, 1, ulSTSZSampleArray[i], fpInput)){
                                                    stHeader.nSampleLength = ulSTSZSampleArray[i];
                                                    if( 1 == fnPrepareADTSPacket(&stHeader, &nHeaderSize, &zAACHeader)){
                                                      printf("\n INFO: Header for : %lu :",i);
                                                      int j = 0;
                                                      for( j = 0; j < nHeaderSize; j++){
                                                        printf("%X ", zAACHeader[j]);
                                                      }// for j loop
                                                      fwrite(zAACHeader, 1, nHeaderSize, fpOut);
                                                      fwrite(pcOutBuffer, 1, ulSTSZSampleArray[i], fpOut);
                                                      ulCurSamples++;
                                                      if(nTotalChunks != ulSTCOTotalChunks){
                                                        printf("\n ERROR: Chunks Numbers Vary!!");
                                                        // TODO: Change location...
                                                        return -1;
                                                      }
                                                      if(ulCurSamples >= nSamplesInChunk[ulCurActiveChunk]){
                                                        // advance to next chunk.
                                                        ulCurSamples = 0;
                                                        ulCurActiveChunk++;
                                                        fseek(fpInput, pulSTCOChunks[ulCurActiveChunk], SEEK_SET);
                                                      }
                                                      if(ulCurActiveChunk >ulSTCOTotalChunks){
                                                        printf("\n ERROR.. Samples exceed Chunks");
                                                      }
                                                    }else{
                                                      printf("\n ERROR: Obtaining ADTS Header failed for : %lu",i);
                                                    }
                                                  }else{
                                                    printf("\n Error When reading Samples : %lu ",i);
                                                  }
                                                  //printf("\n INFO: Sample Size of (%lu) is (%lu)", i, ulSTSZSampleArray[i] );
                                                }// for ulSTSZTotalSamples

                                              }else{// fpOut
                                                printf("\n ERROR: File Open failed when writing output");
                                              }
                                              fclose(fpOut);
                                              if(pcOutBuffer!=NULL) free(pcOutBuffer);
                                            }else{
                                              printf("\n ERROR: Memory Allocation Failed for pcOutBuffer");
                                            }
                                            free(zAACHeader);
                                          }// zAACHeader
                                          if(ulSTSZSampleArray) free(ulSTSZSampleArray);
                                      }
                                      if(pulSTCOChunks) free(pulSTCOChunks);
                                    }// fnParseSTCOForChunks
                                  }// fnParseSTSCForChunks
                                  free(nSamplesInChunk);
                                }// nSamplesInChunk
                              // get the offset of the 1st chunk
                              }// findBox( fpInput, "stsz"
                            }// findBox( fpInput, "stco"
                          }// findBox( fpInput, "stsc"
                          i = nTracksInFile; // can break all loops now
                        }
                      }
                } // // findBox( fpInput, "stsd"...
              }// follow path
            }// for nTracksInFile loop
          }// findBox( fpInput, "trak"...
          free(zulTrackSize);
        }else{
          printf("\n ERROR: Insufficient Memory: Failed for zulTrackSize");
        }// NULL != zulTrackSize
        free(zulTrackOffset);
      }else{
        printf("\n ERROR: Insufficient Memory: Failed for zulTrackOffset");
      }// NULL != zulTrackOffset
    }// ends if((bWasFTYPFound == 1))..
    fclose(fpInput);
  }else{
    printf("\n ERROR: Unable to Open the File : %s for reading", argv[1]);
  }
  for( i = 0; i< nSizePathToTrace; i++){
    free(pzcPathToTrace[i]);
  }
  free(pzcPathToTrace);
  return 0;
}
//}}}
