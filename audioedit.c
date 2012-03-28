/** 
 * Audioedit edits a generic pcm wave audio file, 
 * with the ability to trim samples from the beginning and end.
 * Extensions include audio reversal and audio amplification.
 *
 * The Australian National University COMP2300
 * Copyright 2012 Arrian Purcell u5015666
 */

#include <stdio.h>
#include "./audio.h"

#define AUTHOR "Copyright 2012 Arrian Purcell u5015666"
#define PURPOSE "The Australian National University Comp2300"
#define VERSION "1.0"

/*Main method for audioedit.*/
int main(int argc, char *argv[])
{
  int i = 1;
  
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
      printf("NAME\n   %s - command line program for editing WAVE audio files.\n", argv[0]);
      printf("\nSYNOPSIS\n   %s -help\n   %s -version\n", argv[0], argv[0]);
      printf("   %s [-tb n] [-te m] [-r] [-a f] -i inputfile.wav -o outputfile.wav\n", argv[0]);
      printf("\nDESCRIPTION\n");
      printf("   This program enables the basic audio editing of restricted canonical wave audio files.\n");
      printf("\nOPTIONS\n");
      printf("    -help     display the command line options\n");
      printf("    -version  display the version number\n");
      printf("    -r        reverses the audio clip\n");
      printf("    -a f      amplifies the audio clip by the given float factor f\n");
      printf("    -tb n     trim n samples from the beginning of the audio clip\n");
      printf("    -te m     trim m samples off the end of the audio clip\n");
      printf("    -i file   provide the input file name\n");
      printf("    -o file   provide the output file name (overwriting an existing file)\n");
      return 0;
    }
    else if(!strcmp(argv[i], "-version"))
    {
      printf("%s v%s\n%s\n%s\n", argv[0], VERSION, AUTHOR, PURPOSE);
      return 0;
    }
    else if(!strcmp(argv[i], "-r")) edit.reverse = 1;
    else if(i + 1 < argc)
    {
      if(!strcmp(argv[i], "-i")) audio.input = argv[++i];
      else if(!strcmp(argv[i], "-o")) edit.output = argv[++i];
      else if(!strcmp(argv[i], "-tb")) edit.tb = atoi(argv[++i]);
      else if(!strcmp(argv[i], "-te")) edit.te = atoi(argv[++i]);
      else if(!strcmp(argv[i], "-a")) edit.amplification = atof(argv[++i]);
    }
    else
    {
      printf("Unknown option '%s'. Try '%s -help' for more information.\n", argv[i], argv[0]);
      return 0;
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

