/*
 * SkiFree Configuration System
 * Simple INI-style config file parser.
 */

#ifndef SKIFREE_CONFIG_H
#define SKIFREE_CONFIG_H

/* Initialize config from a file. Returns 0 on success, -1 if file not found (uses defaults). */
int config_init(const char *filepath);

/* Shut down and free config data. */
void config_shutdown(void);

/* Get an integer value. Returns default_val if key not found. */
int config_get_int(const char *section, const char *key, int default_val);

/* Get a float value. Returns default_val if key not found. */
float config_get_float(const char *section, const char *key, float default_val);

/* Get a string value. Returns default_val if key not found.
 * Returned pointer is valid until config_shutdown(). */
const char *config_get_str(const char *section, const char *key, const char *default_val);

/* Convenience accessors for common settings */
int config_classic_mode(void);
int config_sound_enabled(void);
int config_sound_volume(void);
int config_physics_enhanced(void);

#endif /* SKIFREE_CONFIG_H */
