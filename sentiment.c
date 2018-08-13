#include "trie/trie.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 8192
#define MAX_CATEGORIES 1024
#define MAX_WORD_SIZE 1024

// cat_link is the node structure for a linked list. Every term that
// we use for frequency analysis is associated with 1 or more
// categories like this
typedef struct {
    char *category;
    struct cat_link *next;
} cat_link;

// cat_count is used to summarize all of the findings. Each word is
// associated with categories and we use this struct to count the
// number of times we've seen hits for different categories
typedef struct {
    char *category;
    int count;
} cat_count;

// word_tag represents each word in our dictionary. Each work is
// associated with a set of categories. The count member i used to
// track how many times a particular word has been encountered.
typedef struct {
    char *word;
    int count;
    cat_link *cats;
} word_tag;

// get_line will read a line of MAX_LINE_LENGTH from the provided file
char *get_line(FILE *s) {
    char *line = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
    return fgets(line, MAX_LINE_LENGTH, s);
}

// get_words will take a line of text and split it into words. This is
// done by breaking it up by spaces and tabs. This function also takes
// an index in order to track how many words are allocated
char **get_words(char *line, int *index) {
    int num_words = 0;
    int line_length = strlen(line);

    // Figure out the total number of words
    for (int i = 0; i < line_length; i = i + 1) {
        if (line[i] == ' ' || line[i] == '\n' || line[i] == '\t' ||
            line[i + 1] == '\0') {
            num_words = num_words + 1;
        }
    }

    // allocate space to store pointers to all of the words.
    char **words = (char **)calloc(num_words, sizeof(char *));
    *index = 0;
    int was_end = 1;

    // walk down the line and replace all of the space sparation
    // characters with null bytes. This will allow us to use the
    // original memory allocated from the whole line. No need to copy
    // the strings.
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

// add_cat will take a cat_link and a new category and prepend it to
// the beginning of the linked list.
cat_link *add_cat(cat_link *link, char *category) {
    cat_link *c = (cat_link *)malloc(sizeof(cat_link *));
    c->category = category;
    c->next = (struct cat_link *)link;
    return c;
}

// make_trie will take a list of all of the catgories and build a trie
// used to do rapid lookups of all of the words in our sample text;
struct trie *make_trie(char ***cats, int cat_count, int *word_counts) {
    // allocate space for the trie
    struct trie *main_trie = trie_create();

    char **words = NULL;
    int word_count;
    char *current_cat = NULL;
    word_tag *t;

    for (int i = 0; i < cat_count; i = i + 1) {
        words = cats[i];
        word_count = word_counts[i];
        for (int j = 0; j < word_count; j = j + 1) {
            if (j == 0) {
                current_cat = words[j];
                continue;
            }
            // Check to see if the word exists in our trie
            t = (word_tag *)trie_search(main_trie, words[j]);
            if (t == NULL) {
                // if the word doesn't exist, we'll allocate new space
                // for the word_tag and initalize it with the word, no
                // categories, and no count
                t = (word_tag *)malloc(sizeof(word_tag));
                t->word = words[j];
                t->cats = NULL;
                t->count = 0;
            }
            // now that we have a tag, we'll add the current category
            cat_link *c = add_cat(t->cats, current_cat);
            // c would have be prepended with the new cateogry, we'll
            // overwrite the pointer within the word_tag
            t->cats = c;
            // trie_insert should overwrite the particular value
            // associated with the key
            if (trie_insert(main_trie, words[j], t) != 0) {
                fprintf(stderr, "Failed to insert into trie\n");
                exit(1);
            }
        }
    }
    return main_trie;
}

// trim_space will return a new pointer to a string that has been
// trimmed. This function is a little weird because it modifies the
// original string, but you need to use the return value in order to
// get the new starting pointer.
char *trim_space(char *str) {
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str)) {
        str++;
    }

    if (*str == 0) {
        return str;
    }

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && !isalpha((unsigned char)*end)) {
        end--;
    }

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

// lowercase will convert all of the characters of a input string to
// lowecase
void lowercase(char *str) {
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i = i + 1) {
        str[i] = tolower(str[i]);
    }
}

// count_category_visitor implements a trie interface which is used as
// a callback while walking the trie. In this particular case, the
// function takes data which is a word_tag and cats which is a
// collection of all of the cat_counts. While walking the trie, we'll
// update the aggregate counts.
int count_category_visitor (const char *key, void *data, void *cats) {
    (void) key; // avoid compiler warnings
    cat_count *c = (cat_count *)cats;
    word_tag *t = (word_tag *)data;
    cat_link *n = (cat_link *)t->cats;

    // for each cat_link we'll search all of the categories for a match category
    do {
        for (int i = 0; i < MAX_CATEGORIES; i = i + 1) {
            // When we find a matching category, we'll increment it's
            // aggregate count with the number of times that the word
            // has been seen
            if (strcmp(c[i].category, n->category) == 0) {
                c[i].count += t->count;
                break;
            }
        }
        // Move to the next node in the linked list
        n = (cat_link *)n->next;
    } while (n != NULL);
    return 0;
}

int main() {
    char *current_line = NULL;
    char **words = NULL;
    int *word_counts = (int *)calloc(MAX_CATEGORIES, sizeof(char **));
    char ***categories = (char ***)calloc(MAX_CATEGORIES, sizeof(char **));
    int cat_index = 0;
    struct trie *main_trie = NULL;
    word_tag *t = NULL;
    cat_count *c = (cat_count *)calloc(MAX_CATEGORIES, sizeof(cat_count));
    cat_count *cur_cat = NULL;

    // open the list of categories. I should probably make tihs an
    // input or something rather than hard coding.
    FILE *cat_file = fopen("empath/empath/data/categories.tsv", "r");
    if (cat_file == NULL) {
        fprintf(stderr, "Missing Category Datafile\n");
        exit(1);
    }

    // iterate through the input categories and build our basic data structures
    do {
        current_line = get_line(cat_file);
        if (current_line != NULL) {
            words = get_words(current_line, &(word_counts[cat_index]));
            categories[cat_index] = words;
            cur_cat = (cat_count *)malloc(sizeof(cat_count));
            cur_cat->category = words[0]; // hopefully it's there
            cur_cat->count = 0;
            c[cat_index] = *cur_cat;
            cat_index++;
        }
    } while (current_line != NULL);

    // Should be safe to close the file now
    fclose(cat_file);

    // after reading all of the data from the file, we'll make our advanced data type
    main_trie = make_trie(categories, cat_index, word_counts);

    // print out some basic information about the parsed trie
    fprintf(stderr, "Tree loaded\nTree Count: %zd\nTrie Size: %zd\n ",
            trie_count(main_trie, ""), trie_size(main_trie));

    char word_buf[MAX_WORD_SIZE];
    int scan_amt = 0;
    char *current_word = NULL;

    // At this point, all of the initialization is complete. We'll
    // scan stdin word by workd and do lookups in our trie. If the
    // word_tag is found, we'll increment the count
    do {
        scan_amt = scanf("%s", word_buf);
        if (scan_amt > 0) {
            lowercase(word_buf);
            current_word = trim_space(word_buf);
            t = (word_tag *)trie_search(main_trie, current_word);
            if (t != NULL) {
                t->count++;
            }
        }
    } while (scan_amt > 0);

    // All of the words have been counted from the stdin, we'll visit
    // every node in the trie and count all of the cateogires.
    trie_visit(main_trie, "", count_category_visitor, c);

    // print out the basics 
    for (int i = 0; i < cat_index; i = i + 1) {
        printf("%s --- %d\n", c[i].category, c[i].count);
    }

    return 0;
}
