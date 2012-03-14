
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define AUTHOR "Arrian Purcell u5015666"
#define VERSION "0.4"

#define BYTE 8

#define HEADER_LENGTH 44

#define CHUNK_SIZE_OFFSET 4
#define BLOCK_ALIGN_OFFSET 32
#define SUBCHUNK_2_SIZE_OFFSET 40

#define SIZE_LENGTH 4
#define ALIGN_LENGTH 2

/*Holds data for the audio header.*/
typedef struct
{
  char *input;
  unsigned char headerData[44];
  unsigned char *audioData;
  unsigned long audioDataLength;
} Header;

/*Structure for the data for an edit.*/
typedef struct
{
  char *output;
  unsigned int tb;
  unsigned int te;

} Edit;

int terminate(char message[])
{
  printf("TERMINATED: %s\n", message);
  exit(-1);
}

int error(char message[])
{
  printf("ERROR: %s\n", message);
  return 0;
}

/*Returns the length of the given file. Requires open file.*/
unsigned long fileLength(FILE *input)
{
  unsigned long fileLength = 0;

  if(input)
  {
    fseek(input, 0, SEEK_END);
    fileLength = ftell(input);
    fseek(input, 0, SEEK_SET);
  }

  return fileLength;
}

int loadHeader(Header *header, FILE *input)
{
  if(header && header->input && input)
  {
    fread(&(header->headerData), HEADER_LENGTH, 1, input);
    header->audioDataLength = fileLength(input) - HEADER_LENGTH;
    return 1;
  }
  return error("Could not load audio header.");
}

int loadData(Header *header, FILE *input, Edit *edit)
{
  if(header && input && header->audioDataLength)
  {
	header->audioDataLength -= edit->tb;
	header->audioDataLength -= edit->te;

	header->audioData = (unsigned char *)malloc(header->audioDataLength + 1);

	if(!header->audioData) return error("Could not allocate memory for audio data.");

    fread(header->audioData, fileLength(input) - HEADER_LENGTH - edit->te, 1, input + edit->tb);

    return 1;
  }
  return error("Could not load audio data.");
}

/*Loads audio header and data.*/
int loadAudio(Header *header, Edit *edit)
{
  FILE *input = fopen(header->input, "rb");
  if(input)
  {
    if(!loadHeader(header, input)) return 0;
    if(!loadData(header, input, edit)) return 0;
  }
  return 1;
}

/*Changes a section of the buffer to the little endian representation of the given data.*/
void setHeaderLittleEndian(Header *header, unsigned int offset, unsigned int length, int data)
{
  int i;
  for(i = 0; i < length; i++) header->headerData[offset + i] = (data >> (i * BYTE)) & 0xFF;
}

/*Converts the specified section of the buffer from little endian representation to an integer.*/
int fromHeaderLittleEndian(Header *header, unsigned int offset, unsigned int length)
{
  int i;
  int value = 0;
  for(i = 0; i < length; i++) value = value | (header->headerData[i + offset] << (i * BYTE));
  return value;
}

/*Check if data exists in the buffer*/
int headerStringExists(Header *header, char *data, unsigned int offset)
{
  int iter = 0;
  int length = strlen(data);
  for(; iter < length; iter++) if(header->headerData[iter + offset] != data[iter]) return 0;/*Todo: comparing char to pointer?*/
  return 1;
}

/*Checks if the buffer data is a well formed wav file.*/
int isWav(Header *header)
{
  return headerStringExists(header, "RIFF", 0) &&
		 headerStringExists(header, "WAVE", 8) &&
		 headerStringExists(header, "fmt ", 12) &&
		 headerStringExists(header, "data", 36);
}

/*Saves the buffer to a file.*/
int saveAudio(Header *header, Edit *edit)
{
  if(header && edit)
  {
    FILE *output = fopen(edit->output, "wb");
    if(!output) return 0;

    /*Ensuring synthetic size is correct.*/
    setHeaderLittleEndian(header, CHUNK_SIZE_OFFSET, SIZE_LENGTH, (HEADER_LENGTH + header->audioDataLength - CHUNK_SIZE_OFFSET) - SIZE_LENGTH);
    setHeaderLittleEndian(header, SUBCHUNK_2_SIZE_OFFSET, SIZE_LENGTH, (HEADER_LENGTH + header->audioDataLength - SUBCHUNK_2_SIZE_OFFSET) - SIZE_LENGTH);

    fwrite(&(header->headerData), 1, HEADER_LENGTH, output);
    fwrite(header->audioData, 1, header->audioDataLength, output);
    fclose(output);
    return 1;
  }
  return error("Could not save audio.");
}

/*Dumps the file header to the standard out.*/
void dump(Header *header)
{
  unsigned int i;
  if(header && header->input && header->headerData)
  {
    printf("-----------%s Header----------\n", header->input);
    for(i = 0; i < HEADER_LENGTH; i++) printf("%x ", header->headerData[i]);
    printf("\n-------------------------------------\n");
  }
}

int performEdit(Header *header, Edit* edit, int verbose)
{
  if(!header->input) return error("Please specify an input file.\nType 'audioedit -help' for all options.");

  if(!loadAudio(header, edit)) return error("Could not load audio file.");

  if(header->audioDataLength < 1) return error("Input file does not contain any audio data to edit.");

  if(isWav(header)) printf("Input file is OK.\n");
  else return error("Input is not a well formed WAVE file.");

  if(verbose) dump(header);

  if(!edit->output) return 0;

  if(saveAudio(header, edit)) printf("New audio saved as '%s'.\n", edit->output);
  else return error("Could not save audio.");

  return 1;
}

int main(int argc, char *argv[])
{
  Header header;
  Edit edit;

  int i = 1;
  int showHeader = 0;
  
  edit.tb = 0;
  edit.te = 0;

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
      printf("%s v%s\nWritten by %s\n", argv[0], VERSION, AUTHOR);
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

  performEdit(&header, &edit, showHeader);
  if(header.audioData) free(header.audioData);

  return 0;
}

