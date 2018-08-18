#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 16384
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

// strcmp_wild will compare two strings for equality. The difference
// here is tha if a string has "*" it in, that character will serve as
// a wild card for the rest of the string
int strcmp_wild(char *a, char *b) {
    int i = 0;
    while (1) {
        // exact string match
        if (a[i] == '\0' && b[i] == '\0') {
            return 0;
        }
        // wild card match
        if (a[i] == '*' || b[i] == '*') {
            return 0;
        }
        // mismatch
        if (a[i] != b[i]) {
            return a[i] - b[i];
        }
        // check further
        i = i + 1;
        if (i > MAX_WORD_SIZE) {
            fprintf(stderr,
                    "Max word size exceeded while doing strcmp_wild check");
            exit(1);
        }
    }
}

// add_cat will take a cat_link and a new category and prepend it to
// the beginning of the linked list.
cat_link *add_cat(cat_link *link, char *category) {
    cat_link *c = (cat_link *)malloc(sizeof(cat_link *));
    c->category = category;
    c->next = (struct cat_link *)link;
    return c;
}

// find_word_linear will do a linear search of the current words in
// our collection. This is necessary before we sort the collection of
// words
word_tag *find_word_linear(word_tag **word_tags, int word_count, char *term) {
    for (int i = 0; i < word_count; i = i + 1) {
        if (strcmp_wild(word_tags[i]->word, term) == 0) {
            return word_tags[i];
        }
    }
    return NULL;
}

// find_word will doe a binary search within the set of word tags
word_tag *find_word(word_tag **word_tags, int word_count, char *term) {
    int i = 0, j = word_count - 1;
    while (i <= j) {
        int k = (i + j) / 2;
        if (strcmp_wild(word_tags[k]->word, term) == 0) {
            return word_tags[k];
        } else if (strcmp_wild(word_tags[k]->word, term) < 0) {
            i = k + 1;
        } else {
            j = k - 1;
        }
    }
    return NULL;
}

// make_word_tags will initialize the set of all word tags
word_tag **make_word_tags(char ***cats, int cat_count, int *word_counts,
                          int *uniq_word_count) {
    int total_word_count = 0;
    for (int i = 0; i < cat_count; i = i + 1) {
        total_word_count += word_counts[i];
    }
    word_tag **word_tags =
        (word_tag **)calloc(total_word_count, sizeof(word_tag *));

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

            // Check to see if the word exists in our array
            t = (word_tag *)find_word_linear(word_tags, *uniq_word_count,
                                             words[j]);
            if (t == NULL) {
                // if the word doesn't exist, we'll allocate new space
                // for the word_tag and initalize it with the word, no
                // categories, and no count
                t = (word_tag *)malloc(sizeof(word_tag));
                word_tags[*uniq_word_count] = t;
                *uniq_word_count += 1;
                t->word = words[j];
                t->cats = NULL;
                t->count = 0;
            }
            // now that we have a tag, we'll add the current category
            cat_link *c = add_cat(t->cats, current_cat);
            // c would have be prepended with the new cateogry, we'll
            // overwrite the pointer within the word_tag
            t->cats = c;
        }
    }
    return word_tags;
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

// word_tag_cmp will compare two word_tags. Used to sort the full
// collection
int word_tag_cmp(const void *a, const void *b) {
    word_tag **ta = (word_tag **)a;
    word_tag **tb = (word_tag **)b;
    return strcmp((*ta)->word, (*tb)->word);
}

int main() {
    char *current_line = NULL;
    char **words = NULL;
    int *word_counts = (int *)calloc(MAX_CATEGORIES, sizeof(char **));
    char ***categories = (char ***)calloc(MAX_CATEGORIES, sizeof(char **));
    cat_count *c = (cat_count *)calloc(MAX_CATEGORIES, sizeof(cat_count));
    int cat_index = 0;
    word_tag **word_tags = NULL;
    word_tag *t = NULL;
    cat_count *cur_cat = NULL;

    // open the list of categories. I should probably make tihs an
    // input or something rather than hard coding.
    FILE *cat_file = fopen("data/empath-categories.tsv", "r");
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

    int unique_word_count = 0;
    word_tags =
        make_word_tags(categories, cat_index, word_counts, &unique_word_count);
    fprintf(stderr, "unique words in dictionary: %d\n", unique_word_count);
    fprintf(stderr, "unique categories in dictionary: %d\n", cat_index);
    fprintf(stderr, "sorting dictionary\n");
    qsort(word_tags, unique_word_count, sizeof(word_tag **), word_tag_cmp);
    fprintf(stderr, "finished sorting\n");

    char word_buf[MAX_WORD_SIZE];
    int scan_amt = 0;
    char *current_word = NULL;

    int scanned_word_count = 0;
    int matched_word_count = 0;

    // At this point, all of the initialization is complete. We'll
    // scan stdin word by workd and do lookups in our sorted array. If
    // the word_tag is found, we'll increment the count
    scan_amt = scanf("%s", word_buf);
    while (scan_amt > 0) {
        scanned_word_count++;
        lowercase(word_buf);
        current_word = trim_space(word_buf);
        t = (word_tag *)find_word(word_tags, unique_word_count, current_word);
        if (t != NULL) {
            matched_word_count++;
            t->count++;
        }
        scan_amt = scanf("%s", word_buf);
    }

    fprintf(stderr, "Total words scanned: %d\n", scanned_word_count);
    fprintf(stderr, "Total words matched: %d\n", matched_word_count);

    for (int i = 0; i < unique_word_count; i = i + 1) {
        cat_link *n = (cat_link *)word_tags[i]->cats;
        do {
            for (int j = 0; j < cat_index; j = j + 1) {
                if (strcmp(c[j].category, n->category) == 0) {
                    c[j].count += word_tags[i]->count;
                    break;
                }
            }
            // Move to the next node in the linked list
            n = (cat_link *)n->next;
        } while (n != NULL);
    }

    // print out the basics
    for (int i = 0; i < cat_index; i = i + 1) {
        printf("%-20s%d\n", c[i].category, c[i].count);
    }

    return 0;
}
