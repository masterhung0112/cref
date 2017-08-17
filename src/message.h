#ifndef _CHELP_MESSAGE_H
#define _CHELP_MESSAGE_H 1

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Message representation, encoding/decoding routines.
 */
typedef struct msg_t msg_t;
typedef struct msgParseContext_t msgParseContext_t;
typedef enum msgType_t msgType_t;

/**
 * Message encoding types
 */
enum msgType_t {
	MSG_START = 			0, /**!< never used in an argument list, needed by dump as initial value */
	MSG_SECTION_START = 	1, /**!< begin of new section, argument is section name as char* */
	MSG_SECION_END = 		2, /**!< end of current section, no arguments */
	MSG_KEY_VALUE = 		3, /**!< key/value, arguments are key as char*, value as chunk_t */
	MSG_LIST_START =		4, /**!< list start, argument is list name as char* */
	MSG_LIST_ITEM = 		5, /**!< list item, argument is item value as chunk_t */
	MSG_LIST_END = 			6, /**!< end of list, no arguments */

	MSG_END	=				7  /**!< end of argument list, no arguments (never encoded) */
};

/**
 * Callback function for key/value and list items, invoked by parse().
 *
 * @param userData	user data, as passed to parse()
 * @param message	message currently parsing
 * @param name		name of key or list
 * @param value		parsed value
 * @return			TRUE if parsed successfully
 */
typedef bool (*msgValueCb_t)(void *userData, msg_t *message,
							char *name, chunk_t value);


/**
 * Callback function for sections, invoked by parse().
 *
 * @param userData	user data, as passed to parse()
 * @param message	message currently parsing
 * @param ctx		parse context, to pass to recursive parse() invocations.
 * @param name		name of the section
 * @return			TRUE if parsed successfully
 */
typedef bool (*msgSectionCb_t)(void *userData, msg_t *message, 
								msgParseContext_t *ctx, char *name);

#ifdef HAVE_ENUMERATOR_H

/**
 * Create an enumerator over message contents.
 *
 * The enumerator takes a fixed list of arguments, but depending on the
 * type may set not all of them. It returns MSG_END as last argument
 * to indicate the message end, and returns FALSE if parsing the message
 * failed.
 *
 * @return		enumerator over (msgtype_t, char*, chunk_t)
 */
enumerator_t* msgCreateEnumerator(msg_t *this);

/**
 * Create a message from an enumerator.
 *
 * The enumerator uses the same signature as the enumerator returned
 * by create_enumerator(), and gets destroyed by this function. It should
 * return MSG_END to close the message, return FALSE to indicate a failure.
 *
 * @param enumerator	enumerator over (msgtype_t, char*, chunk_t)
 * @return				message representation, NULL on error
 */
msg_t *msgCreateFromEnumerator(enumerator_t *enumerator);

#endif

/**
 * Get the value of a key/value pair as a string.
 *
 * @param def	default value if not found
 * @param fmt	printf style format string for key, with sections
 * @param ...	arguments to fmt string
 * @return		string
 */
char* msgGetStr(msg_t *this, char *def, char *fmt, ...);

/**
 * Get the value of a key/value pair as a string, va_list variant.
 *
 * @param def	default value if not found
 * @param fmt	printf style format string for key, with sections
 * @param args	arguments to fmt string
 * @return		string
 */
char* msgVgetStr(msg_t *this, char *def, char *fmt, va_list args);

/**
 * Get the value of a key/value pair as integer.
 *
 * @param def	default value if not found
 * @param fmt	printf style format string for key, with sections
 * @param ...	arguments to fmt string
 * @return		value
 */
int msgGetInt(msg_t *this, int def, char *fmt, ...);

/**
 * Get the value of a key/value pair as integer, va_list variant
 *
 * @param def	default value if not found
 * @param fmt	printf style format string for key, with sections
 * @param args	arguments to fmt string
 * @return		value
 */
int msgVgetInt(msg_t *this, int def, char *fmt, va_list args);

/**
 * Get the value of a key/value pair as boolean.
 *
 * @param def	default value if not found
 * @param fmt	printf style format string for key, with sections
 * @param ...	arguments to fmt string
 * @return		value
 */
bool msgGetBool(msg_t *this, bool def, char *fmt, ...);

/**
 * Get the value of a key/value pair as boolean, va_list variant
 *
 * @param def	default value if not found
 * @param fmt	printf style format string for key, with sections
 * @param args	arguments to fmt string
 * @return		value
 */
bool msgVgetBool(msg_t *this, bool def, char *fmt, va_list args);

/**
 * Get the raw value of a key/value pair.
 *
 * @param def	default value if not found
 * @param fmt	printf style format string for key, with sections
 * @param ...	arguments to fmt string
 * @return		value
 */
chunk_t msgGetValue(msg_t *this, chunk_t def, char *fmt, ...);

/**
 * Get the raw value of a key/value pair, va_list variant.
 *
 * @param def	default value if not found
 * @param fmt	printf style format string for key, with sections
 * @param args	arguments to fmt string
 * @return		value
 */
chunk_t msgVgetValue(msg_t *this, chunk_t def, char *fmt, va_list args);

/**
 * Get encoded message.
 *
 * @return		message data, points to internal data
 */
chunk_t msgGetEncoding(msg_t *this);

/**
 * Parse a message using callback functions.
 *
 * Any of the callbacks may be NULL to skip this kind of item. Callbacks are
 * invoked for the current section level only. To descent into sections,
 * call parse() from within a section callback using the provided parse
 * context.
 *
 * @param ctx		parse context, NULL for root level
 * @param section	callback invoked for each section
 * @param kv		callback invoked for key/value pairs
 * @param li		callback invoked for list items
 * @param userData	user data to pass to callbacks
 * @return			TRUE if parsed successfully
 */
bool msgParse(msg_t *this, msgParseContext_t *ctx,
			  msgSectionCb_t section, msgValueCb_t kv,
			  msgValueCb_t li, void *userData);

/**
 * Dump a message text representation to a FILE stream.
 *
 * @param label		label to print for message
 * @param pretty	use pretty print with indentation
 * @param out		FILE stream to dump to
 * @return			TRUE if message valid
 */
bool msgDump(msg_t *this, char *label, bool pretty, FILE *out);

/**
 * Destroy a msg_t.
 */
void msgDestroy(msg_t *this);

/**
 * Create a message from encoded data.
 *
 * @param data			message encoding
 * @param cleanup		TRUE to free data during
 * @return				message representation
 */
msg_t *msgCreateFromData(chunk_t data, bool cleanup);

/**
 * Create message from a variable argument list.
 *
 * @param type			first type beginning message
 * @param ...			msgType_t and args, terminated by MSG_END
 * @return				message representation, NULL on error
 */
msg_t *msgCreateFromArgs(msgType_t type, ...);

/**
 * Check if a chunk has a printable string, and print it to buf.
 *
 * @param chunk			chunk containing potential string
 * @param buf			buffer to write string to
 * @param size			size of buf
 * @return				TRUE if printable and string written to buf
 */
bool msgStringify(chunk_t chunk, char *buf, size_t size);

/**
 * Verify the occurrence of a given type for given section/list nesting
 */
bool msgVerifyType(msgType_t type, uint32_t section, bool list);

#ifdef __cplusplus
}
#endif

#endif /* _CHELP_MESSAGE_H */
