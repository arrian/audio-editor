#ifndef AUDIO_WAV
#define AUDIO_WAV

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>

/*Lengths*/
#define CHUNK_DESCRIPTOR_LENGTH 4 /*Length of the chunk descriptor*/
#define CHUNK_SIZE_LENGTH 4 /*Length of the value representing the length of the chunk*/

/*Offsets*/
#define FIRST_CHUNK 12

#define FMT_COMPRESSION 8
#define FMT_CHANNELS 10
#define FMT_BLOCK_ALIGN 20

/*Audio file data.*/
typedef struct
{
  char *input;
  
  unsigned char *buffer;
  int bufferLength;

  /*fmt chunk*/
  unsigned char *fmt;
  unsigned int *fmtLength;
  unsigned short *fmtCompression;
  unsigned short *fmtChannels;
  unsigned short *fmtBlockAlign;
  
  /*data chunk*/
  unsigned char *data;
  int *dataLength;
 
} Audio;

/*Edit data.*/
typedef struct
{
  char *output;
  int tb;
  int te;
  
  int reverse;
} Edit;

int terminate(char message[]);

int wordAlign(int offset);

int stringExists(Audio *audio, char data[], unsigned int offset);

unsigned long audioLength(FILE *input);

void scanChunks(Audio *audio);

void reverseAudio(Audio *audio);

void loadAudio(Audio *audio);

void saveAudio(Audio *audio, Edit *edit);

void performEdit(Audio *audio, Edit *edit);

#endif
