#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 8192
#define MAX_CATEGORIES 1024

char * get_line() {
  char * line = (char *) calloc(MAX_LINE_LENGTH, sizeof(char));
  return fgets(line, MAX_LINE_LENGTH, stdin);
}

char ** get_words(char * line, int * index) {
  int num_words = 0;
  int line_length = strlen(line);
  for (int i = 0; i < line_length; i = i + 1) {
    if (line[i] == ' ' || line[i] == '\n' || line[i] == '\t' || line[i+1] == '\0') {
      num_words = num_words + 1;
    }
  }
  char ** words = (char **) calloc(num_words, sizeof(char*));
  *index = 0;
  int was_end = 1;
  for (int i = 0; i < line_length; i = i + 1) {
    if (line[i] == ' ' || line[i] == '\n' || line[i] == '\t') {
      line[i] = '\0';
      was_end = 1;
    } else if (was_end == 1) {
      was_end = 0;
      words[*index] = &(line[i]);
      *index = *index + 1;
    }
  }
  return words;
}

void print_cats( char *** cats, int cat_count, int * word_counts) {
  printf("Categories: %d\n", cat_count);
  char ** words = NULL;
  int word_count;
  for (int i = 0; i < cat_count; i = i + 1) {
    words = cats[i];
    word_count = word_counts[i];
    for (int j = 0; j < word_count; j = j + 1) {
      printf("%s\n", words[j]);
    }
  }
}

void main(int arc, char ** argv) {
  char * current_line = NULL;
  char ** words = NULL;
  int * word_counts = (int*) calloc(MAX_CATEGORIES, sizeof(char**));
  char *** categories = (char ***) calloc(MAX_CATEGORIES, sizeof(char**));
  int cat_index = 0;

  do {
    current_line = get_line();
    if (current_line != NULL) {
      words = get_words(current_line, &(word_counts[cat_index]));
      categories[cat_index] = words;
      cat_index++;
    }
  } while(current_line != NULL);
  print_cats(categories, cat_index, word_counts);
  exit(0);
}

