#ifndef _SETTINGS_TYPE_H
#define _SETTINGS_TYPE_H 1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kv_t kv_t;
typedef struct section_t section_t;

/**
 * Key/value pair
 */
struct kv_t {
	/**
	 * Key string, relative, not the full name
	 */
	char *key;
	
	/**
	 * Value as string
	 */
	char *value;
};

/**
 * Section containing subsections and key value pairs.
 */
struct section_t {
	/**
	 * Name of the section
	 */
	char *name;
	
	/**
	 * Fallback sections, as section_t
	 */
	array_t *fallbacks;
	
	/**
	 * Subsections, as section_t
	 */
	array_t *section;
	
	/**
	 * Subsections in original order, as section_t
	 */
	array_t *sections_order;
	
	/**
	 * Key value pairs, as kv_t
	 */
	array_t *kv;
	
	/**
	 * Key value pairs in original order, as kv_t
	 */
	array_t *kv_order;
};

/**
 * Create a key/value pair
 *
 * @param key 		key (get adopted)
 * @param value		value (get adopted)
 * @return 			allocated key/value pair
 */
kv_t settingsKvCreate(char *key, char *value);

/**
 * Destroy a key/value pair
 *
 * @param this		key/value pair to destroy
 * @param contents	optional array to store the value in
 */
void settingsKvDestroy(kv_t *this, array_t *contents);

void settingsKvSet(kv_t *this, char *value, array_t *content);
void settingsKvAdd(section_t *section, kv_t *kv, array_t *contents);
section_t *settingSectionCreate(char *name);
void settingsSectionDestroy(section_t *this, array_t *contents);
void settingsSectionAdd(section_t *parent, section_t *section, array_t *contents);
void settingsSectionExtend(section_t *base, section* extension, array_t *contents, bool purge);
int settingsSectionFind(const void *a, const void *b);
int settingsSectionSort(const void *a, const void *b, void *user);
int settingsKvFind(const void *a, const void *b);
int settingsKvSort(const void *a, const void *b, void *user);


#ifdef __cplusplus
}
#endif

#endif /* _SETTINGS_TYPE_H */
