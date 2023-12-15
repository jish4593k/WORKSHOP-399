#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#define ROOT "."

const char *PATTERNS[] = {"_"};

#define BOLD "\033[1m"
#define ENDC "\033[0m"
#define TEMPLATE "%s -- %s"

typedef struct Cleaner {
    const char *root;
    const char **patterns;
    int patterns_size;
    int use_torch;
} Cleaner;

void report(Cleaner *cleaner, int t, const char *f) {
    char *temp;
    asprintf(&temp, TEMPLATE, (t ? "D" : "F"), f);
    if (cleaner->use_torch && t) {
        printf(BOLD "%s" ENDC "\n", temp);
        free(temp);
    } else {
        printf("%s\n", temp);
        free(temp);
    }
}

int matches(Cleaner *cleaner, const char *f) {
    for (int i = 0; i < cleaner->patterns_size; ++i) {
        const char *pattern = cleaner->patterns[i];
        if (strstr(f, pattern) != NULL && strstr(f, pattern) == (f + strlen(f) - strlen(pattern))) {
            return 1;
        }
    }
    return 0;
}

int recurse(Cleaner *cleaner, const char *f, int progress_bar) {
    int is_directory = 0;
    if (access(f, F_OK) == 0) {
        is_directory = 1;
    }

    if (is_directory) {
        int d = matches(cleaner, f);
        report(cleaner, d, f);
        if (d) {
            rmdir(f);
        } else {
            int file_count = 0;
            DIR *dir = opendir(f);
            if (dir != NULL) {
                struct dirent *entry;
                while ((entry = readdir(dir)) != NULL) {
                    if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                        char newf[strlen(f) + strlen(entry->d_name) + 2];
                        snprintf(newf, sizeof(newf), "%s/%s", f, entry->d_name);
                        file_count += recurse(cleaner, newf, progress_bar);
                    }
                }
                closedir(dir);
            }
            return file_count;
        }
    } else {
        int d = matches(cleaner, f);
        if (d) {
            remove(f);
        }
        report(cleaner, d, f);
        return 1;
    }
    return 0;
}

void clean(Cleaner *cleaner) {
    int file_count = recurse(cleaner, cleaner->root, 1);
    printf("Total files processed: %d\n", file_count);
}

int main() {
    const char *patterns[] = {".p__"};
    Cleaner cleaner = {.root = ROOT, .patterns = patterns, .patterns_size = 2, .use_torch = 0};
    clean(&cleaner);
    return 0;
}
