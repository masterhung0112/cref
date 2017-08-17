#ifndef _CHELP_SETTINGS_H
#define _CHELP_SETTINGS_H 1

/**
 * Generic configuration options read from a config file
 *
 * @code
 * settings := (section|keyvalue)*
 * section	:= (name { settings })
 * keyvalue := key = value\n
 * @endcode
 * E.g.:
 * @code
   a = b
   section-one {
  		somevalue = asdf
  		subsection {
  			othervalue = xxx
  		}
   }
   section-two {
   }
   @endcode
 *
 * The values are accessed using the get() functions using dotted keys, e.g.
 *    section-one.subsection.othervalue
 *
 * Currently only a limited set of printf format specifiers are supported
 * (namely %s, %d and %N, see implementation for details).
 *
 * \section includes Including other files
 * Other files can be included, using the include statement e.g.
 * @code
 *		include /somepath/subconfig.conf
 * @endcode
 * Shell patterns like *.conf are possible.
 *
 * If the path is relative, the directory of the file containing the include
 * statement is used as base.
 *
 * Sections loaded from files extend previously loaded sections,
 * already existing values are replaced.
 *
 * All settings included from files are added relative to the section the
 * include statement is in.
 *
 * The following files result in the same final config as above:
 *
 * @code
   a = b
   section-one {
   		include include.conf
   }
   @endcode
 *
 * include.conf
 * @code
 	somevalue = asdf
 	subsection {
 		othervalue = yyy
 	}
   @encode
 */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct settings_t settings_t;

/**
 * Load settings from a file
 *
 * @note If parsing the file fails the object is still created.
 *
 * @param file optional file to read settings from
 * @return settings object
 */
settings_t settingsCreate(char *file);

/**
 * Load setings from a string
 *
 * @note If parsing the file fails the object is still created
 *
 * @param settings string to read settings from
 * @return settings object, or NULL
 */
settings_t settingsCreateString(char *settings);

void settingsDestroy(settings_t *this);

/**
 * Get a settings value as a string
 *
 * @param key	key including sections, printf style format
 * @param def	value returned if key not found
 * @param ...	argument list for key
 * @return		value pointing to internal string
 */
char *settingsGetStr(settings_t *this, char *key, char *def, ...);

/**
 * Get a boolean yes|no, true|false value
 *
 * @param key	key including sections, printf style format
 * @param def	value returned if key not found
 * @param ...	argument list for key
 * @return		value of the key
 */
bool settingsGetBool(settings_t *this, char *key, bool def, ...);

/**
 * Get a settings value as a integer
 *
 * @param key	key including sections, printf style format
 * @param def	value returned if key not found
 * @param ...	argument list for key
 * @return		value of the key
 */
int settingsGetInt(settings_t *this, char *key, int def, ...);

/**
 * Get a settings value as a double
 *
 * @param key	key including sections, printf style format
 * @param def	value returned if key not found
 * @param ...	argument list for key
 * @return		value of the key
 */
double settingsGetDouble(settings_t *this, char *key, double def, ...);

/**
 * Get a settings value as a time value
 *
 * @param key	key including sections, printf style format
 * @param def	value returned if key not found
 * @param ...	argument list for key
 * @return		value of the key
 */
uint32_t settingsGetTime(settings_t *this, char *key, uint32_t def, ...);

/**
 * Set a string value
 *
 * @param key	key including sections, printf style format
 * @param value	value to set (get cloned)
 * @param ...	argument list for key
 */
void settingsSetStr(settings_t *this, char *key, char *value, ...);

/**
 * Set a boolean value
 *
 * @param key	key including sections, printf style format
 * @param value	value to set
 * @param ...	argument list for key
 */
void settingsSetBool(settings_t *this, char *key, bool value, ...);

/**
 * Set a integer value
 *
 * @param key	key including sections, printf style format
 * @param value	value to set
 * @param ...	argument list for key
 */
void settingsSetInt(settings_t *this, char *key, int value, ...);

/**
 * Set a double value
 *
 * @param key	key including sections, printf style format
 * @param value	value to set
 * @param ...	argument list for key
 */
void settingsSetDouble(settings_t *this, char *key, double value, ...);

/**
 * Set a time value
 *
 * @param key	key including sections, printf style format
 * @param value	value to set
 * @param ...	argument list for key
 */
void settingsSetTime(settings_t *this, char *key, uint32_t value, ...);

/**
 * Set a default for string value
 *
 * @param key	key including sections, printf style format
 * @param def	value to set f unconfigured
 * @param ...	argument list for key
 * @return		TRUE if a new default value for key as been set
 */
bool settingsSetDefaultStr(settings_t *this, char *key, char *def, ...);

#ifdef HAVE_ENUMERATOR_H

/**
 * Create a enumerator over subsection names of a section
 *
 * @param section 	section including parents, printf style format
 * @param ...		argument list for key
 * @return			enumerator over subsection names
 */
enumerator_t *settingsCreateSectionEnumerator(settings_t *this, char *section, ...);

/**
 * Create a enumerator over key/value pairs in a section
 *
 * @param section 	section name to list key/value pairs of, printf style
 * @param ...		argument list for section
 * @return			enumerator over (char *key, char *value)
 */
enumerator_t *settingsCreateKeyValueEnumerator(settings_t *this, char *section, ...);

#endif /* HAVE_ENUMERATOR_H */

/**
 * Add a fallback for the given section.
 *
 * Example: When the fallback 'section-two' is configured for 'section-one.two'
 * any failed lookup for a section or key in 'section-one.two'
 * will result in a lookup for the same section/key in 'section-two'.
 *
 * @note Lookups are depth-first and currently strictly top-down
 * For instance, if app.sec had lib1.sec as fallback and lib1 had lib2 as fallback
 * the keys/sections in libs2.sec would not be considered. But if app had lib3
 * as fallback the contents of lib3.sec would (as app is passed during initial lookup).
 * In the last example the order during enumerations would be app.sec, lib1.sec, lib3.sec
 *
 * @note Additional arguments will be applied to both section format strings
 * so they must be compatible
 *
 * @param section	section for which a fallback is configured, printf style
 * @param fallback	fallback section, printf style
 * @param ...		argument list for section and fallback
 */
void settingsAddFallback(settings_t *this, const char *section, const char *fallback, ...);

/**
 * Load settings from the files matching the given pattern
 *
 * If merge is TRUE, existing sections are extended, existing values
 * replaced, by those found in the loaded files. If it is FALSE, existing
 * seciotns are purged before reading the new config
 *
 * @note If nay of the files matching the pattern fails to load, no settings
 * are added at all, So, it's all or nothing
 *
 * @param pattern 	file pattern
 * @param merge 	TRUE to merge config with existing values
 * @return			TRUE, if settings were loaded successfully
 */
bool settingsLoadFiles(settings_t *this, char *pattern, bool merge);

/**
 * Load settings from the files matching the given pattern
 *
 * If merge is TRUE, existing sections are extended, existing values
 * replaced, by those found in the loaded files. If it is FALSE, existing
 * seciotns are purged before reading the new config
 *
 * All settings are loaded relative to the given section. The section is
 * created, if it does not yet exist.
 *
 * @note If nay of the files matching the pattern fails to load, no settings
 * are added at all, So, it's all or nothing
 *
 * @param pattern 	file pattern
 * @param merge 	TRUE to merge config with existing values
 * @param section	section name of parent section, printf style
 * @param ...		argument list for section
 * @return			TRUE, if settings were loaded successfully
 */
bool settingsLoadFilesSection(settings_t *this, char *pattern, bool merge,
									char *section, ...);

/**
 * Load settings from the string
 *
 * If merge is TRUE, existing sections are extended, existing values
 * replaced, by those found in the loaded files. If it is FALSE, existing
 * seciotns are purged before reading the new config
 *
 * @note If the string contains __include__ statements they should be absolute paths
 *
 * @note If any failure occur, no settings are added at all. So, it's all or nothing.
 *
 * @param settings 	string to parse
 * @param merge 	TRUE to merge config with existing values
 * @return			TRUE, if settings were loaded successfully
 */
bool settingsLoadString(settings_t *this, char *settings, bool merge);

/**
 * Load settings from the string
 *
 * If merge is TRUE, existing sections are extended, existing values
 * replaced, by those found in the loaded files. If it is FALSE, existing
 * seciotns are purged before reading the new config
 *
 * @note If the stirng contains __include__ statements they should be absolute paths
 *
 * @note If any failure occur, no settings are added at all. So, it's all or nothing.
 *
 * @param settings 	string to parse
 * @param merge 	TRUE to merge config with existing values
 * @param section	section name of parent section, printf style
 * @param ...		argument list for section
 * @return			TRUE, if settings were loaded successfully
 */
bool settingsLoadStringSection(settings_t *this, char *settings, bool merge,
								char *section, ...);

#ifdef __cplusplus
}
#endif

#endif /* _CHELP_SETTINGS_H */
