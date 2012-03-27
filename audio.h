#ifndef AUDIO_WAV
#define AUDIO_WAV

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>

/*Lengths*/
#define CHUNK_DESCRIPTOR_LENGTH 4 /*Length of the chunk descriptor*/

/*Generic riff chunk.*/
typedef struct
{
  char label[CHUNK_DESCRIPTOR_LENGTH];
  unsigned int size;
  unsigned char *data;
} Chunk;

/*WAVE format chunk*/
typedef struct
{
  unsigned short compression;
  unsigned short channels;
  unsigned int sampleRate;
  unsigned int bytesPerSecond;
  unsigned short blockAlign;
} FmtChunk;

/*Audio file data.*/
typedef struct
{
  char *input;
  
  unsigned char *buffer;
  int bufferLength;

  Chunk *riff;
  Chunk *fmtHeader;
  FmtChunk *fmt;
  Chunk *data;
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

int stringExists(Audio *audio, char data[], unsigned int offset);

unsigned long audioLength(FILE *input);

void scanChunks(Audio *audio);

void reverseAudio(Audio *audio);

void loadAudio(Audio *audio);

void saveAudio(Audio *audio, Edit *edit);

void performEdit(Audio *audio, Edit *edit);

#endif
