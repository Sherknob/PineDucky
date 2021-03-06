#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#define MAX_KEYWORD_LENGTH 20
#define DEBUG 1

typedef enum Type{Key, Modifier, Keyword, Custom}Type;

/*
*  Struct Entry
*
*  Holds information about an Entry
*
*  @var keyword_length = length of the keyword the @struct Keys
*    will be searched upon
*  @var keyword = Holds keyword as STRING so I can do strcmp
*  @var value = Holds two u_int8_t shifted into one u_int16_t
*    - First 8 bits (read from right to left) = Modifier
*    - Second 8 bits (read from right to left) = Key value
*
*/
typedef struct Entry{
  Type type;
  char * keyword;
  u_int16_t value;
}Entry;

/*
*  Struct Script
*
*  Struct to hold Bytesequences of 8 bytes each
*
*  @var quantity = Holds number of entries in sequences
*  @var sequences = Holds Bytesequences
*
*/
typedef struct Script{
  u_int16_t quantity;
  u_int8_t ** sequences;
}Script;

/*
 *  Struct SplitLine
 *
 *  This struct is used to split a line out of the DuckyScript into
 *    smaller pieces, to evaluate them all one by one.
 *
 *  @var quantity = Holds number of entries in slices
 *  @var length = Holds pointers to length of slices
 *  @var slices = Holds slices of strings
 *
 */
typedef struct SplitLine{
  u_int8_t quantity;
  u_int16_t * length;
  char ** slices;
}SplitLine;

/*
 *  Struct Time
 *
 *  Holds @var delay number of milliseconds
 *    and a position @var position
 *
*/
typedef struct Time{
  int delay;
  u_int16_t position;
}Time;

/*
 *  Struct Delay
 *
 *  Holds a @var quantity of @var entries of type Time
 *
 */
typedef struct Delay{
  u_int16_t quantity;
  Time * entries;
}Delay;

/*
 *  Function setupEntry
 *
 *  Helperfunction to alocate and fill @param dest Entry pointer
 *
 */
static Entry * setupEntry(Entry * dest, char * keyword, char * value, u_int8_t type){
  dest = malloc(sizeof(Entry *));
  dest->keyword = malloc(sizeof(char) * strnlen(keyword, MAX_KEYWORD_LENGTH));
  dest->keyword = strcpy(dest->keyword, keyword);
  dest->value = (u_int16_t) strtol(value, NULL, 16);;
  dest->type = type;
  return dest;
}

/*
 *  Function printSplitLine
 *
 *  Prints all contents of split line onto STDOUT
 *
 */
static void printSplitLine(SplitLine * sl){
  printf("Quantity: %d\n", sl->quantity);
  for (size_t i = 0; i < sl->quantity; i++){
    printf("%s: %d\n", "Slice", i);
    printf("\t%s: %d\n\t", "length", sl->length[i]);
    for (size_t j = 0; j < sl->length[i]; j++) {
      printf("%c", sl->slices[i][j]);
    }
    printf("\n%s\n", "-----------------------------");
  }
}

/*
 *  Function insertIntoDelay
 *
 *  Inserts delays of type Time into @param _delay
 *
 */
static void insertIntoDelay(Delay * _delay, u_int16_t position, int time){
  // Make space for new entry
  if(DEBUG) printf("%s\n", "Allocating memory for new entry");
  _delay->entries = realloc(_delay->entries, sizeof(Time) * (_delay->quantity+1));
  _delay->entries[_delay->quantity].delay = time;
  _delay->entries[_delay->quantity].position = position;
  if(DEBUG) printf("\t%s: %p\n", "Adress of Entry", &_delay->entries[_delay->quantity]);
  if(DEBUG) printf("\t%s: %p\n", "Adress of Entry.delay", &_delay->entries[_delay->quantity].delay);
  if(DEBUG) printf("\t%s: %d\n", "Delay", _delay->entries[_delay->quantity].delay);
  if(DEBUG) printf("\t%s: %d\n", "Position", _delay->entries[_delay->quantity].position);
  _delay->quantity += 1;
  if(DEBUG) printf("\t%s: %d\n", "Delay->quantity", _delay->quantity);
  if(DEBUG) printf("%s\n\n", "Done");
}

/*
 *  Function splitLine
 *
 *  Splits line into smaller pieces by delimiter
 *
 *  @return struct SplitLine see headerfile for more information
 *  @var tmp = Holds temporary sequence of chars, before
 *    they get inserted into struct @var result
 *
 */
static SplitLine * splitLine(char * line, u_int16_t len, const char delim){
  //number of entries in the first pointer
  SplitLine * result;
  result = (SplitLine *)calloc(1, sizeof(struct SplitLine));
  result->quantity = 0;
  result->length = malloc(sizeof(u_int16_t));
  result->slices = malloc(sizeof(char *));
  u_int16_t tmp_allocated = 10;
  char * tmp; tmp = malloc(tmp_allocated * sizeof(char));
  //
  // result->slices   p--->   char *  p---> char *
  //
  //
  u_int16_t offset = 0;
  for(int i = 0; i < len; i++){
    if(i != 0 && line[i] == delim || i == len-1){
      result->length = realloc(result->length, result->quantity+1);
      result->slices = realloc(result->slices, (result->quantity+2) * sizeof(char *));
      if(DEBUG) printf("result address: %p\n", result);
      if(DEBUG) printf("result->quantity address: %p\n", &result->quantity);
      if(DEBUG) printf("result->slices address: %p\n", result->slices);
      if(DEBUG) printf("result->quantity value: %d\n", result->quantity);
      if(DEBUG) printf("offset value: %d\n", i-offset);
      result->slices[result->quantity] = malloc(i-offset+1 * sizeof(char)); // +1 because of the '\0' following
      if(DEBUG) printf("result->slices[result->quantity] malloced size: %d\n", i-offset+1);

      if(DEBUG) printf("result->slices[result->quantity] address: %p\n", &result->slices[result->quantity]);
      u_int16_t j = 0;
      while (j < i-offset){
        // if(DEBUG) printf("insert: %c at ", tmp[j]);
        // if(DEBUG) printf("j: %d\n", j);
        result->slices[result->quantity][j] = tmp[j];
        j++;
      }
      if(DEBUG) printf("insert: %s at ", "\\0");
      if(DEBUG) printf("j: %d\n", j);
      result->slices[result->quantity][j] = '\0';
      if(DEBUG) printf("%s: %d\n", "Done with inserting i",i);
      offset = (u_int16_t)i+1;
      if(DEBUG) printf("%s: %p\n", "Address of result->length[result->quantity]", &result->length[result->quantity]);
      result->length[result->quantity] = j;
      if(DEBUG) printf("%s %d %s%d%s\n", "inserting", j, "into result->length[", result->quantity, "]");
      if(DEBUG) printf("Currently inserted %d\n", result->length[result->quantity]);
      free(tmp); tmp_allocated = 10;
      tmp = malloc(tmp_allocated * sizeof(char));
      result->quantity += 1;
      if(DEBUG) printf("%s\n", "----------------------------------------------------------");
    }else{
      if(i-offset > tmp_allocated/2){
        tmp_allocated += 25;
        tmp = realloc (tmp, tmp_allocated * sizeof(char));
      }
      if(DEBUG) printf("| %c |", line[i]);
      if(DEBUG) printf("%s%d%s\n", "tmp[", i-offset, "]");
      tmp[i-offset] = line[i];
    }
  }
  free(tmp);
  return result;
}

/*
 *  Function openFile
 *
 *  Opens file and @return filepointer
 *
 */
static FILE * openFile(char * path, char * args){
  FILE * fp;
  fp = fopen(path, args);
  if (fp == NULL){
    printf("%s\n", "File not found, exiting...");
    exit(EXIT_FAILURE);
  }
  return fp;
}

/*
 *  Function closeFile
 *
 *  Closes file @param fp and frees @param line
 *
 */
static void closeFile(FILE * fp, char * line){
  fclose(fp);
  if (line)
      free(line);
}

/*
 *  Function freeSplitLine
 *
 *  Well, frees struct @param _sl
 *
 */
static void freeSplitLine(SplitLine * _sl){
  for (size_t i = 0; i < _sl->quantity; i++) {
    if(_sl->slices[i]){
      // printf("slice %p gets freed\n", _sl->slices[i]);
      free(_sl->slices[i]);
    }
  }
  if(_sl->slices) free(_sl->slices);
  if(_sl->length) free(_sl->length);
  if(_sl) free(_sl);
}

/*
 *  Function strrev
 *
 *  Reverses a given str and @return returns it
 *
 */
static char * strrev(char * str){
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}

/*
 *  Function printIntToBinary
 *
 *  @return char * with @param in binary
 *
 */
static char * printIntToBinary(char * p, u_int8_t n){
  int pos = 0;
  do{
    if(n % 2 == 0) p[pos++] = '0';
    else p[pos++] = '1';
    n = n / 2;
  }while(n > 0);
  for (size_t i = pos; i < 8; i++) { p[pos++] = '0'; }
  p[pos] = '\0';
  return strrev(p);
}

/*
 *  Function inSequence
 *
 *  Checks if @param key is in @param sequence
 *
 */
static u_int8_t inSequence(u_int8_t key, u_int8_t * sequence){
  for (size_t i = 0; i < 8; i++) {
    if(key == sequence[i]) return 1;
  }
  return 0;
}

/*
 *  Function inPrevSequence
 *
 *  Checks if @param key is in @param _sc previous sequence
 *
 */
static u_int8_t inPrevSequence(Script * _sc, u_int8_t key){
  if(_sc->quantity <= 1) return 0;
  for (size_t i = 0; i < 8; i++) {
    if(key == _sc->sequences[_sc->quantity-1][i]) return 1;
  }
  return 0;
}

/*
 *  Function isEmpty
 *
 *  Checks, if @param seq is empty and @return 1 if so
 *
 */
static u_int8_t isEmpty(u_int8_t * seq){
  for (size_t i = 0; i < 8; i++) {
    if(seq[i] != 0) return 0;
  }
  return 1;
}

/*
 * Function writeSequences
 *
 *  Well, writes sequences to @param fp
 *
 */
static u_int8_t writeSequences(Script * _sc, Delay * _dl, FILE * fp){
  fprintf(fp, "%s\n\n", "#!/bin/bash");
  for (size_t i = 0; i < _sc->quantity; i++) {
    fprintf(fp, "%s", "echo -ne \"");
    for (size_t j = 0; j < 8; j++) {
      fprintf(fp, "%s%x", "\\x", _sc->sequences[i][j]);
    }
    if(isEmpty(_sc->sequences[i]) || _sc->sequences[i][0] != 0) fprintf(fp, "%s\n", "\" > /dev/hidg0");
    else fprintf(fp, "%s\n", "\" > /dev/hidg0 &&");
    for (size_t k = 0; k < _dl->quantity; k++) {
      if(_dl->entries[k].position == i){
        fprintf(fp, "%s %lf\n", "sleep", (float)_dl->entries[k].delay / 1000);
      }
    }
  }
}

/*
 *  Function insertIntoScript
 *
 *  Adding functionality to insert nullSequence, when
 *    @param sequence and @param mask == nullptr
 *
 *  Inserts @param sequence into @param _sc (COPYING)
 *  Resets @param mask to default
 *  Renews @param sequence
 *
 */
static void insertIntoScript(Script * _sc, u_int8_t ** sequence, u_int8_t * mask){
  // Make space for another entry
  _sc->sequences = realloc(_sc->sequences, sizeof(char *) * _sc->quantity+1);
  _sc->sequences[_sc->quantity] = calloc(8, sizeof(char));
  for (size_t i = 0; i < 8; i++){
    _sc->sequences[_sc->quantity][i] = (!sequence && !mask) ? 0 : (*sequence)[i];
  }
  _sc->quantity += 1;

  // Resetting sequence
  if(sequence) *sequence = calloc(8, sizeof(u_int8_t));
  // Reset mask
  if(mask) *mask = 0b01000000;
}

/*
 *  Function nextFreeSpot
 *
 *  @return position of next 0 bit in @param mask
 *  Note: read from left to right
 *
 */
static u_int8_t nextFreeSpot(char * mask){
  for (size_t i = 2; i < 8; i++) {
    if(mask[i] == '0') return i;
  }
  // Not returning in the for loop above should not happen,
  // because it is checked before, if sequence is full
}

/*
 *  Function modifyBit
 *
 *  Sets bit in @param mask to @param value at @param p position
 *
 */
static void modifyBit(u_int8_t * mask, u_int8_t p, u_int8_t value){
  p = 8 - p - 1;
  u_int8_t _m = 1 << p;
  char q[9];
  if(DEBUG) printf("%s:\n", "Applying mask");
  if(DEBUG) printf("  %s\n", printIntToBinary(q, *mask));
  if(DEBUG) printf("  %s\n", printIntToBinary(q, _m));
  if(DEBUG) printf("  %s\n", "--------");
  *mask = ((*mask & ~_m) | (value << p));
  if(DEBUG) printf("= %s\n", printIntToBinary(q, *mask));

}

/*
 *  Function insertIntoSequence
 *
 *  Does various checks and then
 *    inserts @var keyValue into @param tmp_sequence
 *
 */
static void insertIntoSequence(Script * _sc, u_int8_t * mask, u_int8_t ** tmp_sequence, u_int16_t seq){
  if(DEBUG) printf("\n-------------------------------------------\n");
  if(DEBUG) printf("%s: %d\n", "Current Sequence count", _sc->quantity);
  char * maskInBin = calloc(9, sizeof(char));
  printIntToBinary(maskInBin, *mask);

  if(DEBUG) printf("%s: \n\t%s\n", "Mask before", maskInBin);
  if(DEBUG) printf("Address of tmp_sequence: %p\n", *tmp_sequence);
  if(DEBUG) printf("%s: \n\t", "Sequence");
  for (size_t i = 0; i < 8; i++) {
    if(DEBUG) printf("%x ", (*tmp_sequence)[i]);
  }

  u_int8_t modifier = seq & 0x00FF;
  u_int8_t keyValue = seq >> 8;
  if(DEBUG) printf("Key value is: %x\n", keyValue);
  if(DEBUG) printf("Modifier is: %x\n", modifier);

  // Checking if tmp_sequence is full, if yes, send and start inserting into new sequence
  if(*mask > 0b01111110) {
    insertIntoScript(_sc, tmp_sequence, mask);
    if(DEBUG) printf("FULL\n");
  }

  // Checking if the current value is already present in tmp_sequence, or in sequence before, if yes send Sequence and insert current value into new one
  if(keyValue != 0 && (inSequence(keyValue, (*tmp_sequence)) || inPrevSequence(_sc, keyValue))){
    insertIntoScript(_sc, tmp_sequence, mask);
    insertIntoScript(_sc, tmp_sequence, mask);
    if(DEBUG) printf("Value was already present\n");
  }
  // mask is empty, or modifier is same
  // checking if sequence is empty, then its okay to add another modifier, but if its already filled with a key, then its not okay to add another modifier
  if(!(*mask == 0b01000000 || modifier == (*tmp_sequence)[0])){
    insertIntoScript(_sc, tmp_sequence, mask);
    insertIntoScript(_sc, tmp_sequence, mask);
    if(DEBUG) printf("MODIFIER WAS NOT 0 so sending\n");
  }
  if(keyValue == 0x0a) insertIntoScript(_sc, tmp_sequence, mask);
  /*
   *  Adding two modifiers with bitwise OR '|'
   *
   *  Note: Currently kind of tested...
   *
   *  SHIFT = 00000010
   *  ALT   = 00000001
   *          --------
   *        = 00000011
   *
   */
  (*tmp_sequence)[0] = (*tmp_sequence)[0] | modifier;
  // if @var keyValue is 0 then we dont need to waste a free spot, we can just return here.
  if(keyValue == 0) return;
  printIntToBinary(maskInBin, *mask);
  u_int8_t free = nextFreeSpot(maskInBin);
  if(DEBUG) printf("Inserting into: tmp_sequence[%d]\n", free);

  (*tmp_sequence)[keyValue == 0x0a ? 7 : free] = keyValue;
  if(keyValue == 0x0a) insertIntoScript(_sc, tmp_sequence, mask);

  //Set bit at position @var free to 1
  modifyBit(mask, free, 1);
  if(DEBUG) printf("\n%s:\n\t", "Sequence");
  for (size_t i = 0; i < 8; i++) {
    if(DEBUG) printf("%x ", (*tmp_sequence)[i]);
  }

  if(DEBUG) printf("\n-------------------------------------------\n");
}





















//
