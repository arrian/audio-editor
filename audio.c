#include "audio.h"

/*Terminates the program, printing out the given message.*/
int terminate(char message[])
{
  printf("ERROR: %s\n", message);
  exit(EXIT_FAILURE);
}

int isChunk(Chunk *chunk, char label[])
{
  int i = 0;
  for(; i < CHUNK_DESCRIPTOR_LENGTH; i++) if(chunk->label[i] != label[i]) return 0;
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
  audio->riff = (Chunk *) audio->buffer;
  unsigned char *riffData = (unsigned char *) &(audio->riff->data);
  Chunk *chunkIter = (Chunk *) (riffData + CHUNK_DESCRIPTOR_LENGTH);//adding CDL to skip WAVE label
  
  assert(audio);
  
  if(!isChunk(audio->riff, "RIFF")) terminate("Input file is not a WAVE audio file.");//should also check WAVE label
  
  while((unsigned char *)&chunkIter->data < (unsigned char *)(audio->buffer + audio->bufferLength))
  {
    if(isChunk(chunkIter, "fmt "))
    {
      audio->fmtHeader = chunkIter;
      audio->fmt = (FmtChunk *) &audio->fmtHeader->data;
    }
    else if(isChunk(chunkIter, "data")) audio->data = chunkIter;
    else printf("WARNING: Unhandled chunk '%.*s'\n", CHUNK_DESCRIPTOR_LENGTH, chunkIter->label);
    
    if(chunkIter->size <= 0) terminate("Unusual chunk size.");
    
    unsigned char *dataStart = (unsigned char *) &chunkIter->data;
    unsigned int size = chunkIter->size;
    
    chunkIter = (Chunk *) (dataStart + size);
  }

  if(!audio->data) terminate("Input file does not contain a data chunk.");
  if(!audio->fmt) terminate("Input file does not contain a fmt chunk.");
  if((audio->fmt->compression) != 1) printf("WARNING: Input is not a PCM WAVE file. Editing features may not work as expected.\n");
}

void loadAudio(Audio *audio)
{
  assert(audio);

  FILE *input = fopen(audio->input, "rb");
  if(!input) terminate("Could not open input file.");

  audio->bufferLength = audioLength(input);
  audio->buffer = (unsigned char *) malloc(audio->bufferLength + 1);
  if(!audio->buffer) terminate("Could not allocate enough memory for the input file.");
  if(!fread(audio->buffer, audio->bufferLength, sizeof(unsigned char), input)) terminate("Unable to read input file.");

  fclose(input);
  
  scanChunks(audio);
  
  printf("'%s' is OK.\n", audio->input);
}

/*Extension - reverse audio*/
void reverseAudio(Audio *audio)
{
  int forwards = 0;
  int backwards = audio->data->size;
  unsigned char temp;
  
  assert(audio);

  printf("Reversing audio...\n");
  
  while(forwards < backwards)
  {
    temp = *(audio->data->data + backwards);
    *(audio->data->data + backwards) = *(audio->data->data + forwards);
    *(audio->data->data + forwards) = temp;
    
    backwards--;
    forwards++;
  }
}

void trimAudio(Audio *audio, Edit *edit)
{
  /*Multiplying by block align to handle removing data from multiple channels.*/
  unsigned int tbLength = edit->tb * audio->fmt->blockAlign;
  unsigned int teLength = edit->te * audio->fmt->blockAlign;
  unsigned int removed = tbLength + teLength;
  int total = audio->data->size - removed;

  if(total <= 0) terminate("Too many samples were specified.");
  
  memcpy((unsigned char *)&audio->data->data, (unsigned char *)&audio->data->data + tbLength, total);
  memcpy((unsigned char *)&audio->data->data + total, (unsigned char *)&audio->data->data + total, (audio->buffer + audio->bufferLength) - ((unsigned char *)&audio->data->data + total));
  
  /*Ensuring synthetic chunk sizes are correct.*/
  audio->riff->size -= removed;
  audio->data->size -= removed;
  audio->bufferLength -= removed;
}

void saveAudio(Audio *audio, Edit *edit)
{
  int writeCount;
  FILE *output;
  
  assert(audio && edit);
  
  output = fopen(edit->output, "wb");
  if(!output) terminate("Unable to create or overwrite output file.");
  
  writeCount = fwrite(audio->buffer, sizeof(unsigned char), audio->bufferLength, output);
  if(!writeCount) terminate("Could not write data to output file.");
  
  fclose(output);
  
  printf("Saved new audio as '%s'.\n", edit->output);
}

void performEdit(Audio *audio, Edit *edit)
{
  loadAudio(audio);
  if(!edit->output) return;
  trimAudio(audio, edit);
  if(edit->reverse) reverseAudio(audio);
  saveAudio(audio, edit);
}

