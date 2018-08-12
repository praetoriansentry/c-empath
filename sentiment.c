#include "trie/trie.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 8192
#define MAX_CATEGORIES 1024

char *get_line(FILE *s) {
    char *line = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
    return fgets(line, MAX_LINE_LENGTH, s);
}

typedef struct {
    char *category;
    struct cat_link *next;
} cat_link;

typedef struct {
    char *word;
    cat_link *cats;
} word_tag;

char **get_words(char *line, int *index) {
    int num_words = 0;
    int line_length = strlen(line);
    for (int i = 0; i < line_length; i = i + 1) {
        if (line[i] == ' ' || line[i] == '\n' || line[i] == '\t' ||
            line[i + 1] == '\0') {
            num_words = num_words + 1;
        }
    }

    char **words = (char **)calloc(num_words, sizeof(char *));
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

cat_link *add_cat(cat_link *link, char *category) {
    cat_link *c = (cat_link *)malloc(sizeof(cat_link *));
    c->category = category;
    c->next = (struct cat_link *)link;
    return c;
}

struct trie *make_trie(char ***cats, int cat_count, int *word_counts) {
    printf("Categories: %d\n", cat_count);
    struct trie *main_trie = trie_create();

    char **words = NULL;
    int word_count;
    char *current_cat = NULL;
    word_tag *t;
    // cat_link * cl = NULL;

    for (int i = 0; i < cat_count; i = i + 1) {
        words = cats[i];
        word_count = word_counts[i];
        for (int j = 0; j < word_count; j = j + 1) {
            if (j == 0) {
                current_cat = words[j];
                continue;
            }
            t = (word_tag *)trie_search(main_trie, words[j]);
            if (t == NULL) {
                t = (word_tag *)malloc(sizeof(word_tag));
                t->word = words[j];
                t->cats = NULL;
            }
            cat_link *c = add_cat(t->cats, current_cat);
            t->cats = c;
            if (trie_insert(main_trie, words[j], t) != 0) {
                fprintf(stderr, "Failed to insert into trie\n");
                exit(1);
            }
            /* printf("%s: ", t->word); */
            /* cl = t->cats; */
            /* while(cl != NULL && cl->category != NULL) { */
            /*   printf("%s ", cl->category); */
            /*   cl = (cat_link *) cl->next; */
            /* } */
            /* printf("\n"); */
        }
    }
    return main_trie;
}

int main() {
    char *current_line = NULL;
    char **words = NULL;
    int *word_counts = (int *)calloc(MAX_CATEGORIES, sizeof(char **));
    char ***categories = (char ***)calloc(MAX_CATEGORIES, sizeof(char **));
    int cat_index = 0;
    FILE *cat_file = fopen("categories.tsv", "r");
    struct trie *main_trie = NULL;

    if (cat_file == NULL) {
        fprintf(stderr, "Missing Category Datafile\n");
        exit(1);
    }

    do {
        current_line = get_line(cat_file);
        if (current_line != NULL) {
            words = get_words(current_line, &(word_counts[cat_index]));
            categories[cat_index] = words;
            cat_index++;
        }
    } while (current_line != NULL);
    main_trie = make_trie(categories, cat_index, word_counts);
    fprintf(stderr, "Tree loaded\nTree Count: %zd\nTrie Size: %zd\n ",
            trie_count(main_trie, ""), trie_size(main_trie));
    return 0;
}
