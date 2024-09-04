#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdio.h>

#define MAX_LINE_LENGTH 256
#define MAX_SECTION_NAME_LENGTH 50
#define MAX_KEY_NAME_LENGTH 50
#define MAX_VALUE_LENGTH 100

typedef struct st_config_entry{
    char key[MAX_KEY_NAME_LENGTH];
    char value[MAX_VALUE_LENGTH];
} st_config_entry_t;

typedef struct st_config_section{
    char section[MAX_SECTION_NAME_LENGTH];
    st_config_entry_t *entries;
    size_t entry_count;
} st_config_section_t;

typedef struct st_config_file{
    st_config_section_t *sections;
    size_t section_count;
} st_config_file_t;


int parse_config(const char *filename, st_config_file_t *config);
const char* get_config_value(const st_config_file_t *config, const char *section, const char *key);
void free_config(st_config_file_t *config);
#endif