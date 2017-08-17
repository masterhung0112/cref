#ifndef _CHELP_MSGBUILDER_H
#define _CHELP_MSGBUILDER_H 1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct msgBuilder_t msgBuilder_t;

/**
 * Append a generic message element to message.
 *
 * The additional arguments are type specific, it may be nothing, a string,
 * a chunk value or both.
 *
 * @param type	element type to add
 * @param ...	additional type specific arguments
 */
void msgBuilderAdd(msgBuilder_t *this, msgType_t type, ...);

/**
 * Append a key/value element using a format string.
 *
 * Instead of passing the type specific value as a chunk, this method
 * takes a printf() style format string followed by its arguments. The
 * key name for a key/value type is still a fixed string.
 *
 * @param key	key name of the key/value to add
 * @param fmt	value format string
 * @param ...	arguments to value format string
 */
void msgBuilderAddKv(msgBuilder_t *this, char *key, char *fmt, ...);

/**
 * Append a message element using a format string and va_list.
 *
 * Instead of passing the type specific value as a chunk, this method
 * takes a printf() style format string followed by its arguments. The
 * key name for a key/value type is still a fixed string.
 *
 * @param key	key name of the key/value to add
 * @param fmt	value format string
 * @param args	arguments to value format string
 */
void msgBuilderVaddKv(msgBuilder_t *this, char *key, char *fmt, va_list args);

/**
 * Append a list item element using a format string.
 *
 * Instead of passing the type specific value as a chunk, this method
 * takes a printf() style format string followed by its arguments.
 *
 * @param fmt	value format string
 * @param ...	arguments to value format string
 */
void msgBuilderAddLi(msgBuilder_t *this, char *fmt, ...);

/**
 * Append a list item element using a format string and va_list.
 *
 * Instead of passing the type specific value as a chunk, this method
 * takes a printf() style format string followed by its arguments.
 *
 * @param fmt	value format string
 * @param args	arguments to value format string
 */
void msgBuilderVaddLi(msgBuilder_t *this, char *fmt, va_list args);

/**
 * Begin a new section.
 *
 * @param name	name of section to begin
 */
void msgBuilderBeginSection(msgBuilder_t *this, char *name);

/**
 * End the currently open section.
 */
void msgBuilderEndSection(msgBuilder_t *this);

/**
 * Begin a new list.
 *
 * @param name	name of list to begin
 */
void msgBuilderBeginList(msgBuilder_t *this, char *name);

/**
 * End the currently open list.
 */
void msgBuilderEndList(msgBuilder_t *this);

/**
 * Finalize a vici message with all added elements, destroy builder.
 *
 * @return		vici message, NULL on error
 */
msg_t* msgBuilderFinalize(msgBuilder_t *this);

/**
 * Create a msgBuilder_t instance.
 */
msgBuilder_t *msgBuilderCreate();

/**
 * Destroy a vici builder without finalization.
 *
 * Note that finalize() already destroys the message, and calling destroy()
 * is required only if the message does not get finalize()d.
 */
void msgBuilderDestroy(msgBuilder_t *this);

#ifdef __cplusplus
}
#endif

#endif /* _CHELP_MSGBUILDER_H */
