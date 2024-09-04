#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"



// Helper function to trim whitespace
void trim_whitespace(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0)
        return;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
}

// Parse the config file
int parse_config(const char *filename, st_config_file_t *config) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return -1;
    }

    char line[MAX_LINE_LENGTH];
    st_config_section_t *current_section = NULL;

    config->sections = NULL;
    config->section_count = 0;

    while (fgets(line, sizeof(line), file)) {
        trim_whitespace(line);

        // Skip empty lines and comments
        if (strlen(line) == 0 || line[0] == '#') {
            continue;
        }

        // Handle section headers
        const char s = line[0];
        const char e = line[strlen(line) - 1];

        if (s == '[' && e == ']') {
            line[strlen(line) - 1] = '\0'; // Remove trailing ']'
            char section_name[MAX_SECTION_NAME_LENGTH];
            strncpy(section_name, line + 1, sizeof(section_name));
            trim_whitespace(section_name);

            config->sections = realloc(config->sections, sizeof(st_config_section_t) * (config->section_count + 1));
            current_section = &config->sections[config->section_count];
            strncpy(current_section->section, section_name, sizeof(current_section->section));
            current_section->entries = NULL;
            current_section->entry_count = 0;
            config->section_count++;
        } else {
            if (current_section) {
                char *delimiter = strchr(line, '=');
                if (delimiter) {
                    *delimiter = '\0';
                    char key[MAX_KEY_NAME_LENGTH];
                    char value[MAX_VALUE_LENGTH];
                    strncpy(key, line, sizeof(key));
                    strncpy(value, delimiter + 1, sizeof(value));
                    trim_whitespace(key);
                    trim_whitespace(value);

                    current_section->entries = realloc(current_section->entries, sizeof(st_config_entry_t) * (current_section->entry_count + 1));
                    st_config_entry_t *entry = &current_section->entries[current_section->entry_count];
                    strncpy(entry->key, key, sizeof(entry->key));
                    strncpy(entry->value, value, sizeof(entry->value));
                    current_section->entry_count++;
                }
            }
        }
    }

    fclose(file);
    return 0;
}

// Get the value for a given section and key
const char* get_config_value(const st_config_file_t *config, const char *section, const char *key) {
    for (size_t i = 0; i < config->section_count; i++) {
        if (strcmp(config->sections[i].section, section) == 0) {
            for (size_t j = 0; j < config->sections[i].entry_count; j++) {
                if (strcmp(config->sections[i].entries[j].key, key) == 0) {
                    return config->sections[i].entries[j].value;
                }
            }
        }
    }
    return NULL;
}

// Free the memory used by the configuration
void free_config(st_config_file_t *config) {
    for (size_t i = 0; i < config->section_count; i++) {
        free(config->sections[i].entries);
    }
    free(config->sections);
}