#include <stdio.h>
#include <stdlib.h>

#define MAX_LINE_LENGTH 8192


char * get_line() {
  char * line = (char *) calloc(MAX_LINE_LENGTH, sizeof(char));
  return fgets(line, MAX_LINE_LENGTH, stdin);
}

void main(int arc, char ** argv) {
  char * current_line = NULL;
  do {
    current_line = get_line();
    if (current_line != NULL) {
      printf("%s\n", current_line);
    }
  } while(current_line != NULL);
  exit(0);
}

