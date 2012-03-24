#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>

/*Descriptors*/
#define AUTHOR "Arrian Purcell u5015666"
#define PURPOSE "For ANU Comp2300 2012"
#define VERSION "0.6"

/*Lengths*/
#define PCM 16 /*standard pulse code modulated wave header size*/
#define HEADER_LENGTH 44
#define SIZE_LENGTH 4

/*Offsets*/
#define CHUNK_SIZE_OFFSET 4
#define SUBCHUNK_1_SIZE_OFFSET 16
#define BLOCK_ALIGN_OFFSET 32
#define SUBCHUNK_2_SIZE_OFFSET 40

/*Media file data.*/
typedef struct
{
  char *input;
  unsigned char headerData[44];
  unsigned char *audioData;
  int audioDataLength;
  
  int *chunkSize;
  int *subchunk1Size;
  int *subchunk2Size;
  short *blockAlign;
  
} Header;

/*Edit data.*/
typedef struct
{
  char *output;
  int tb;
  int te;

} Edit;

/*Terminates the program, printing out the given message.*/
int terminate(char message[])
{
  printf("ERROR: %s\n", message);
  exit(EXIT_FAILURE);
}

/*Returns the length of the given file. Requires open file.*/
unsigned long fileLength(FILE *input)
{
  unsigned long fileLength = 0;

  assert(input);
  
  fseek(input, 0, SEEK_END);
  fileLength = ftell(input);
  fseek(input, 0, SEEK_SET);

  return fileLength;
}

/*Loads audio file header.*/
int loadHeader(Header *header, FILE *input)
{
  assert(header && input);

  fread(&(header->headerData), HEADER_LENGTH, 1, input);
  header->audioDataLength = fileLength(input) - HEADER_LENGTH;

  if(header->audioDataLength < 0) terminate("The input file is not a well-formed wav file.");
  
  header->chunkSize = (int*) &(header->headerData[CHUNK_SIZE_OFFSET]);
  header->subchunk1Size = (int*) &(header->headerData[SUBCHUNK_1_SIZE_OFFSET]);
  header->subchunk2Size = (int*) &(header->headerData[SUBCHUNK_2_SIZE_OFFSET]);
  header->blockAlign = (short*) &(header->headerData[BLOCK_ALIGN_OFFSET]);
  
  return 1;
}

/*Loads audio file sound data.*/
int loadData(Header *header, FILE *input, Edit *edit)
{
  unsigned char *buffer;
  int beginSamples = (edit->tb * (*header->blockAlign));
  int endSamples = (edit->te * (*header->blockAlign));
 
  assert(header && input && edit);
  if(edit->tb < 0 || edit->te < 0) terminate("Not able to trim negative samples.");
  
  buffer = (unsigned char *)malloc(header->audioDataLength + 1);
  if(!buffer) terminate("Could not allocate memory for the file data.");
  
  fread(buffer, sizeof(unsigned char), header->audioDataLength, input);
  
  header->audioDataLength -= beginSamples;
  header->audioDataLength -= endSamples;
  if(header->audioDataLength < 0) terminate("Too many samples to trim.");
  
  header->audioData = (unsigned char *)malloc(header->audioDataLength + 1);
  if(!header->audioData) terminate("Could not allocate memory when loading data.");
  
  memcpy(header->audioData, buffer + beginSamples, header->audioDataLength);
  
  free(buffer);

  return 1;
}

/*Loads audio header and data.*/
int loadAudio(Header *header, Edit *edit)
{
  FILE *input = fopen(header->input, "rb");
  if(input)
  {
    loadHeader(header, input);
    loadData(header, input, edit);
    return 1;
  }
  return 0;
}

/*Check if data exists in the buffer*/
int headerStringExists(Header *header, char data[], unsigned int offset)
{
  int iter = 0;
  int length = strlen(data);
  for(; iter < length; iter++) if(header->headerData[iter + offset] != data[iter]) return 0;
  return 1;
}

/*Checks if the header appears to be from a well formed PCM(pulse code modulation) wave file.*/
int isWav(Header *header)
{             
  assert(header);

  if(!(headerStringExists(header, "RIFF", 0) &&
       headerStringExists(header, "WAVE", 8) &&
       headerStringExists(header, "fmt ", 12))) terminate("Input is not a wave file.");
  if((*header->subchunk1Size) != PCM) terminate("Input is a wave file but is not PCM.");
  if(!headerStringExists(header, "data", 36)) terminate("Input is not a well formed WAVE file.");
  
  return 1;
}

/*Saves the header and selected audio data to a file.*/
int saveAudio(Header *header, Edit *edit)
{
  assert(header && edit);
  
  FILE *output = fopen(edit->output, "wb");
  if(!output) terminate("Not able to create or overwrite output file.");

  /*Ensuring synthetic riff sizes are correct.*/
  *(header->chunkSize) = (HEADER_LENGTH + header->audioDataLength - CHUNK_SIZE_OFFSET) - SIZE_LENGTH;
  *(header->subchunk2Size) = (HEADER_LENGTH + header->audioDataLength - SUBCHUNK_2_SIZE_OFFSET) - SIZE_LENGTH;
  
  if(!fwrite(&(header->headerData), 1, HEADER_LENGTH, output) ||
     !fwrite(header->audioData, 1, header->audioDataLength, output)) terminate("Could not write data to output file.");
  
  fclose(output);
  
  return 1;
}

/*Dumps the file header to the standard out.*/
void dump(Header *header)
{
  unsigned int i;
  
  assert(header);
  
  printf("-----------%s Header----------\n", header->input);
  printf("Chunk Size: %d \nSubchunk 2 Size: %d \nBlock Align: %d\n", *header->chunkSize, *header->subchunk2Size, *header->blockAlign);
  for(i = 0; i < HEADER_LENGTH; i++) printf("%x ", header->headerData[i]);
  printf("\n-------------------------------------\n");
}

/*Performs an edit if the edit is acceptable.*/
int performEdit(Header *header, Edit* edit, int showHeader)
{
  assert(header && edit);
  
  if(!loadAudio(header, edit)) terminate("Could not load input file.");
  if(header->audioDataLength <= 0) terminate("Input file does not contain any data to edit.");
  if(isWav(header)) printf("'%s' is OK.\n", header->input);
  if(showHeader) dump(header);
  if(!edit->output) return 1;/*no output file given, so silently die.*/
  if(saveAudio(header, edit)) printf("New audio saved as '%s'.\n", edit->output);
  if(header->audioData) free(header->audioData);
  
  return 1;
}

/*Program that edits an audio file, cutting off samples from the beginning and end.*/
int main(int argc, char *argv[])
{
  int i = 1;
  int showHeader = 0;
  
  Edit edit = {0};
  Header header = {0};
  
  if(argc <= 1)
  {
    printf("%s: missing operand\nTry '%s -help' for more information.\n", argv[0], argv[0]);
    return 0;
  }

  /*Getting command line data.*/
  for(; i < argc; i++)
  {
    if(!strcmp(argv[i], "-help"))
    {
      printf("NAME\n   %s - command line program for editing WAVE audio files.\n\nSYNOPSIS\n   %s -help\n   %s -version\n   %s [-tb n] [-te m] -i inputfile.wav -o outputfile.wav\n\nDESCRIPTION\n   This program enables the basic audio editing of restricted canonical wave audio files.\n\nOPTIONS\n    -help     display the command line options\n    -version  display the version number\n    -header   displays the wav file header data\n    -tb n     trim n samples from the beginning of the audio clip\n    -te m     trim m samples off the end of the audio clip\n    -i file   provide the input file name\n    -o file   provide the output file name (overwriting an existing file)\n", argv[0], argv[0], argv[0], argv[0]);
      return 0;
    }
    else if(!strcmp(argv[i], "-version"))
    {
      printf("%s v%s\nWritten by %s\n%s\n", argv[0], VERSION, AUTHOR, PURPOSE);
      return 0;
    }
    else if(!strcmp(argv[i], "-header")) showHeader = 1;
    else if(i + 1 < argc)
    {
      if(!strcmp(argv[i], "-i")) header.input = argv[i + 1];
      else if(!strcmp(argv[i], "-o")) edit.output = argv[i + 1];
      else if(!strcmp(argv[i], "-tb")) edit.tb = atoi(argv[i + 1]);
      else if(!strcmp(argv[i], "-te")) edit.te = atoi(argv[i + 1]);
    }
  }
  
  if(!header.input)
  {  
    printf("Please specify an input file. Try '%s -help' for more information.\n", argv[0]);
    return 0;
  }

  performEdit(&header, &edit, showHeader);

  return 0;
}

