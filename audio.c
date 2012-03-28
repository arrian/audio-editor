#include "audio.h"

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

unsigned long audioLength(FILE *input)
{
  unsigned long audioLength = 0;

  assert(input);
  
  fseek(input, 0, SEEK_END);
  audioLength = ftell(input);
  fseek(input, 0, SEEK_SET);

  return audioLength;
}

void scanChunks(Audio *audio)
{
  audio->riff = (Chunk *) audio->buffer;
  char *riffData = (char *) &(audio->riff->data);
  Chunk *chunkIter = (Chunk *) (riffData + CHUNK_DESCRIPTOR_LENGTH);//adding CDL to skip WAVE label
  
  assert(audio);
  
  if(!isChunk(audio->riff, "RIFF")) terminate("Input file is not a WAVE audio file.");//should also check WAVE label
  
  while((char *)&chunkIter->data < (char *)(audio->buffer + audio->bufferLength))
  {
    if(isChunk(chunkIter, "fmt "))
    {
      audio->fmtHeader = chunkIter;
      audio->fmt = (FmtChunk *) &audio->fmtHeader->data;
    }
    else if(isChunk(chunkIter, "data")) audio->data = chunkIter;
    else printf("WARNING: Unhandled chunk '%.*s'\n", CHUNK_DESCRIPTOR_LENGTH, chunkIter->label);
    
    if(chunkIter->size <= 0) terminate("Unusual chunk size.");
    
    char *dataStart = (char *) &chunkIter->data;
    unsigned int size = chunkIter->size;
    
    chunkIter = (Chunk *) (dataStart + size);
  }

  if(!audio->data) terminate("Input file does not contain a data chunk.");
  if(!audio->fmt) terminate("Input file does not contain a fmt chunk.");
  if((audio->fmt->compression) != 1) printf("WARNING: Input is not a PCM WAVE file. Edit features may not work as expected.\n");
}

void loadAudio(Audio *audio)
{
  assert(audio);

  FILE *input = fopen(audio->input, "rb");
  if(!input) terminate("Could not open input file.");

  audio->bufferLength = audioLength(input);
  audio->buffer = (char *) malloc(audio->bufferLength + 1);
  if(!audio->buffer) terminate("Could not allocate enough memory for the input file.");
  if(!fread(audio->buffer, audio->bufferLength, sizeof(char), input)) terminate("Unable to read input file.");

  fclose(input);
  
  scanChunks(audio);
  
  printf("Input '%s' is OK.\n", audio->input);
}

void reverseAudio(Audio *audio)
{
  int forwards = 0;
  int backwards = audio->data->size;
  char temp;
  char *data = (char *)(&audio->data->data);
  
  assert(audio);

  printf("Reversing audio...\n");
  
  while(forwards < backwards)
  {
    temp = *(data + backwards);
    *(data + backwards) = *(data + forwards);
    *(data + forwards) = temp;
    
    backwards--;
    forwards++;
  }
}

void amplifyAudio(Audio *audio, Edit *edit)
{
  int samples;
  int i = 0;
  int new;
  int max;
  int min;
  int16_t *start;
  
  assert(audio && edit);
  
  if(edit->amplification <= 0.0f) terminate("Amplification factor should be more than 0.");

  //will only handle 16 bit samples
  if(audio->fmt->bitsPerSample != 16)
  {
    printf("Unable to amplify audio with a sample length of %d bits.\n", audio->fmt->bitsPerSample);
    return;
  }
  
  printf("Amplifying audio by %.2f...\n", edit->amplification);
  
  start = (int16_t *) (&audio->data->data);
  samples = (int) (audio->data->size / sizeof(int16_t));
    
  while(i < samples)
  {
    new = (int) (*(start + i) * edit->amplification);
    if(new > INT16_MAX) *(start + i) = INT16_MAX;
    else if(new < INT16_MIN) *(start + i) = INT16_MIN;
    else *(start + i) = new;
    i++;
  }
}

void trimAudio(Audio *audio, Edit *edit)
{
  //Multiplying by block align to handle removing data from multiple channels.
  //Multiplying by 2 since block align measured in bytes (in this case 2 chars per byte)
  unsigned int tbLength = edit->tb * audio->fmt->blockAlign * 2;
  unsigned int teLength = edit->te * audio->fmt->blockAlign * 2;
  
  unsigned int removed = tbLength + teLength;
  int total = audio->data->size - removed;

  if(total <= 0) terminate("Too many samples were specified.");
  
  memcpy((char *)&audio->data->data, (char *)&audio->data->data + tbLength, total);
  memcpy((char *)&audio->data->data + total, (char *)&audio->data->data + total, (audio->buffer + audio->bufferLength) - ((char *)&audio->data->data + total));
  
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
  
  writeCount = fwrite(audio->buffer, sizeof(char), audio->bufferLength, output);
  if(!writeCount) terminate("Could not write data to output file.");
  
  fclose(output);
  
  printf("Saved new audio as '%s'.\n", edit->output);
}

void performEdit(Audio *audio, Edit *edit)
{
  loadAudio(audio);
  if(!edit->output) return;
  trimAudio(audio, edit);
  if(edit->amplification) amplifyAudio(audio, edit);
  if(edit->reverse) reverseAudio(audio);
  saveAudio(audio, edit);
}

