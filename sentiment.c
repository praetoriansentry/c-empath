#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 8192


char * get_line() {
  char * line = (char *) calloc(MAX_LINE_LENGTH, sizeof(char));
  return fgets(line, MAX_LINE_LENGTH, stdin);
}

char ** get_words(char * line) {
  int num_words = 0;
  int line_length = strlen(line);
  for (int i = 0; i < line_length; i = i + 1) {
    if (line[i] == ' ' || line[i] == '\n' || line[i] == '\t' || line[i+1] == '\0') {
      num_words = num_words + 1;
    }
  }
  char ** words = (char **) calloc(num_words, sizeof(char*));
  int index = 0;
  int was_end = 1;
  for (int i = 0; i < line_length; i = i + 1) {
    if (line[i] == ' ' || line[i] == '\n' || line[i] == '\t') {
      line[i] = '\0';
      was_end = 1;
    } else if (was_end == 1) {
      was_end = 0;
      words[index++] = &(line[i]);
    }
  }
  return words;
}

void main(int arc, char ** argv) {
  char * current_line = NULL;
  char ** words = NULL;
  do {
    current_line = get_line();
    if (current_line != NULL) {
      words = get_words(current_line);
      int wordcount = sizeof(words) / sizeof(char*);
      for (int i = 0; i < wordcount; i = i + 1) {
        printf("%s\n", words[i]);
      }
    }
  } while(current_line != NULL);
  exit(0);
}

