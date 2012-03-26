#include <stdio.h>
#include "./audio.h"

#define AUTHOR "Arrian Purcell u5015666"
#define PURPOSE "For ANU Comp2300 2012"
#define VERSION "0.6"

/*Program that edits an audio file, cutting off samples from the beginning and end.*/
int main(int argc, char *argv[])
{
  int i = 1;
  int showHeader = 0;
  
  Edit edit = {0};
  Audio audio = {0};
  
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
    else if(!strcmp(argv[i], "-reverse")) edit.reverse = 1;
    else if(i + 1 < argc)
    {
      if(!strcmp(argv[i], "-i")) audio.input = argv[i + 1];
      else if(!strcmp(argv[i], "-o")) edit.output = argv[i + 1];
      else if(!strcmp(argv[i], "-tb")) edit.tb = atoi(argv[i + 1]);
      else if(!strcmp(argv[i], "-te")) edit.te = atoi(argv[i + 1]);
    }
  }
  
  if(!audio.input)
  {  
    printf("Please specify an input file. Try '%s -help' for more information.\n", argv[0]);
    return 0;
  }

  performEdit(&audio, &edit);

  return 0;
}

