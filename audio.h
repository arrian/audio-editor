#ifndef AUDIO_WAV
#define AUDIO_WAV

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <stdint.h>

#define CHUNK_DESCRIPTOR_LENGTH 4

/*Generic RIFF chunk.*/
typedef struct
{
  char label[CHUNK_DESCRIPTOR_LENGTH];
  unsigned int size;
  char data;
} Chunk;

/*WAVE format chunk*/
typedef struct
{
  unsigned short compression;
  unsigned short channels;
  unsigned int sampleRate;
  unsigned int bytesPerSecond;
  unsigned short blockAlign;
  unsigned short bitsPerSample;
} FmtChunk;

/*Audio file data.*/
typedef struct
{
  char *input;
  
  char *buffer;
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
  
  /*Extensions*/
  int reverse;
  float amplification;
} Edit;

/*Terminates the program, printing out the given message.*/
int terminate(char message[]);

/* Returns the length of the given audio file. 
 * Requires an open file.
 */
unsigned long audioLength(FILE *input);

/*Scans the file for required chunks.*/
void scanChunks(Audio *audio);

/*Extension - reverses audio.*/
void reverseAudio(Audio *audio);

/* Extension - amplifies audio.
 * Concept from http://www.ypass.net/blog/2010/01/pcm-audio-part-3-basic-audio-effects-volume-control/
 */
void amplifyAudio(Audio *audio, Edit *edit);

/*Loads an audio clip from the filesystem.*/
void loadAudio(Audio *audio);

/*Saves the buffer data to the filesystem.*/
void saveAudio(Audio *audio, Edit *edit);

/*Runs the edits defined in the given Edit struct.*/
void performEdit(Audio *audio, Edit *edit);

#endif
