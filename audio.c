#include "audio.h"

/*Terminates the program, printing out the given message.*/
int terminate(char message[])
{
  printf("ERROR: %s\n", message);
  exit(EXIT_FAILURE);
}

/* Checks if the given offset is word aligned.
 * Returns the offset as a word aligned value (multiple of 2).
 * RIFF file chunks must be word aligned, 
 * but word alignment padding not counted in chunk size.
 */
int wordAlign(int offset)
{
  if(offset % 2 == 0) return offset;
  return offset + 1;
}

/*Check if data exists in the buffer*/
int stringExists(Audio *audio, char data[], unsigned int offset)
{
  int i = 0;
  int length = strlen(data);
  for(; i < length; i++) if(*(audio->buffer + i + offset) != data[i]) return 0;
  return 1;
}

/* Returns the length of the given audio file. 
 * Requires an open file.
 */
unsigned long audioLength(FILE *input)
{
  unsigned long audioLength = 0;

  assert(input);
  
  fseek(input, 0, SEEK_END);
  audioLength = ftell(input);
  fseek(input, 0, SEEK_SET);

  return audioLength;
}

/*Scans the file for required chunks.*/
void scanChunks(Audio *audio)
{
  int i = FIRST_CHUNK;
  int j;
  
  assert(audio);
  
  if(!stringExists(audio, "RIFF", 0) || 
     !stringExists(audio, "WAVE", CHUNK_DESCRIPTOR_LENGTH + CHUNK_SIZE_LENGTH))
  {     
    terminate("Input file is not a WAVE audio file.");
  }
  
  while(i < audio->bufferLength)
  {
    if(stringExists(audio,"fmt ", i))
    {
      audio->fmt = audio->buffer + i;
      audio->fmtLength = (int *) (audio->buffer + i + CHUNK_DESCRIPTOR_LENGTH);
      audio->fmtCompression = (int *) (audio->buffer + FMT_COMPRESSION);
      audio->fmtChannels = (int *) (audio->buffer + FMT_CHANNELS);
      audio->fmtBlockAlign = (short *) (audio->buffer + FMT_BLOCK_ALIGN);
    }
    else if(stringExists(audio,"data", i))
    {
      audio->data = audio->buffer + i;
      audio->dataLength = (int *) (audio->buffer + i + CHUNK_DESCRIPTOR_LENGTH);
    }
    else
    {
      printf("NOTE: Unhandled chunk ");
      for(j = 0; j < CHUNK_DESCRIPTOR_LENGTH; j++) printf("%c", *(audio->buffer + j + i));
      printf("\n");
    }
    i += *((int *) (audio->buffer + i + CHUNK_DESCRIPTOR_LENGTH)) + CHUNK_DESCRIPTOR_LENGTH + CHUNK_SIZE_LENGTH;
    i = wordAlign(i);/*as required by the RIFF specification.*/
  }
}

void loadAudio(Audio *audio)
{
  assert(audio);

  FILE *input = fopen(audio->input, "rb");
  if(!input) terminate("Could not open input file.");

  audio->bufferLength = audioLength(input);
  audio->buffer = (unsigned char *) malloc(audio->bufferLength + 1);
  if(!fread(audio->buffer, audio->bufferLength, sizeof(unsigned char), input)) terminate("Unable to read input file.");
  
  fclose(input);
  
  scanChunks(audio);
  
  if(!audio->data) terminate("Input file does not contain a data chunk.");
  if(!audio->fmt) terminate("Input file does not contain a fmt chunk.");
  
  if(*(audio->fmtLength) != 16) printf("NOTE: Input is not a PCM WAVE file.\n");
}

void saveAudio(Audio *audio, Edit *edit)
{
  assert(audio && edit);
  
  int oldDataLength = (*(audio->dataLength));
  
  int writeCount = 0;
  
  FILE *output = fopen(edit->output, "wb");
  if(!output) terminate("Unable to create or overwrite output file.");
  
  /*Multiplying by block align to handle removing data from multiple channels.*/
  int tbLength = edit->tb * (*(audio->fmtBlockAlign));
  int teLength = edit->te * (*(audio->fmtBlockAlign));
  int total = tbLength + teLength;
  if(total > (*audio->dataLength)) terminate("Too many samples were specified.");
  
  /*Ensuring synthetic chunk sizes are correct.*/
  *((int *) (audio->buffer + CHUNK_DESCRIPTOR_LENGTH)) = ((audio->bufferLength - total) - CHUNK_DESCRIPTOR_LENGTH) - CHUNK_SIZE_LENGTH;
  *(audio->dataLength) -= total;
  
  /*Writing first section of buffer up to data chunk.*/
  writeCount += fwrite(audio->buffer, sizeof(unsigned char), (audio->data - audio->buffer) + CHUNK_DESCRIPTOR_LENGTH + CHUNK_SIZE_LENGTH, output);
  
  /*Writing data chunk.*/
  writeCount += fwrite(audio->data + CHUNK_DESCRIPTOR_LENGTH + CHUNK_SIZE_LENGTH + tbLength, sizeof(unsigned char), *(audio->dataLength), output);
  
  /*Writing post data chunk buffer.*/
  //writeCount += fwrite(audio->data + CHUNK_DESCRIPTOR_LENGTH + CHUNK_SIZE_LENGTH + oldDataLength, sizeof(unsigned char), audio->bufferLength - ((audio->data - audio->buffer) + CHUNK_DESCRIPTOR_LENGTH + CHUNK_SIZE_LENGTH + oldDataLength), output);
  
  if(!writeCount) terminate("Could not write data to output file.");
  
  fclose(output);
}

void performEdit(Audio *audio, Edit *edit)
{
  loadAudio(audio);
  printf("'%s' is OK.\n", audio->input);
  if(!edit->output) return;
  saveAudio(audio, edit);
  printf("Saved new audio as '%s'.\n", edit->output);
}

