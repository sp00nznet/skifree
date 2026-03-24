/*
 * SkiFree Configuration System
 * Simple INI-style config file parser.
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_CONFIG_ENTRIES 64
#define MAX_KEY_LEN 64
#define MAX_VAL_LEN 256
#define MAX_SECTION_LEN 32

typedef struct {
    char section[MAX_SECTION_LEN];
    char key[MAX_KEY_LEN];
    char value[MAX_VAL_LEN];
} ConfigEntry;

static ConfigEntry entries[MAX_CONFIG_ENTRIES];
static int num_entries = 0;

static char *trim(char *s) {
    char *end;
    while (isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) *end-- = '\0';
    return s;
}

int config_init(const char *filepath) {
    FILE *f;
    char line[512];
    char current_section[MAX_SECTION_LEN] = "";
    char *trimmed, *eq, *key, *val;
    int len;

    num_entries = 0;

    if (!filepath) return -1;

    f = fopen(filepath, "r");
    if (!f) {
        printf("[config] No config file at %s, using defaults\n", filepath);
        return -1;
    }

    printf("[config] Loading %s\n", filepath);

    while (fgets(line, sizeof(line), f)) {
        trimmed = trim(line);

        /* Skip empty lines and comments */
        if (trimmed[0] == '\0' || trimmed[0] == '#' || trimmed[0] == ';')
            continue;

        /* Section header */
        if (trimmed[0] == '[') {
            len = (int)strlen(trimmed);
            if (len > 2 && trimmed[len - 1] == ']') {
                trimmed[len - 1] = '\0';
                strncpy(current_section, trimmed + 1, MAX_SECTION_LEN - 1);
                current_section[MAX_SECTION_LEN - 1] = '\0';
            }
            continue;
        }

        /* Key=Value */
        eq = strchr(trimmed, '=');
        if (!eq) continue;

        *eq = '\0';
        key = trim(trimmed);
        val = trim(eq + 1);

        if (num_entries < MAX_CONFIG_ENTRIES) {
            strncpy(entries[num_entries].section, current_section, MAX_SECTION_LEN - 1);
            strncpy(entries[num_entries].key, key, MAX_KEY_LEN - 1);
            strncpy(entries[num_entries].value, val, MAX_VAL_LEN - 1);
            num_entries++;
        }
    }

    fclose(f);
    printf("[config] Loaded %d entries\n", num_entries);
    return 0;
}

void config_shutdown(void) {
    num_entries = 0;
}

static const char *find_value(const char *section, const char *key) {
    int i;
    for (i = 0; i < num_entries; i++) {
        if (strcmp(entries[i].section, section) == 0 &&
            strcmp(entries[i].key, key) == 0) {
            return entries[i].value;
        }
    }
    return NULL;
}

int config_get_int(const char *section, const char *key, int default_val) {
    const char *val = find_value(section, key);
    if (!val) return default_val;
    return atoi(val);
}

float config_get_float(const char *section, const char *key, float default_val) {
    const char *val = find_value(section, key);
    if (!val) return default_val;
    return (float)atof(val);
}

const char *config_get_str(const char *section, const char *key, const char *default_val) {
    const char *val = find_value(section, key);
    return val ? val : default_val;
}

int config_classic_mode(void) {
    return config_get_int("game", "classic_mode", 0);
}

int config_sound_enabled(void) {
    return config_get_int("sound", "enabled", 1);
}

int config_sound_volume(void) {
    return config_get_int("sound", "volume", 128);
}

int config_physics_enhanced(void) {
    return config_get_int("physics", "enhanced", 0);
}
