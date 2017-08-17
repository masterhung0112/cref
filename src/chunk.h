#ifndef _CHELP_CHUNK_H
#define _CHELP_CHUNK_H	1

/* memwipe, memeq */
/* free */

#ifdef HAVE_PRINTF_HOOK_H
#include "printfHook.h" /* printfHookData_t, printfHookSpec_t */
#endif /* HAVE_PRINTF_HOOK_H */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct chunk_t chunk_t;

/**
 * General purpose pointer/length abstraction.
 */
struct chunk_t {
	uint8_t *ptr;		/** Pointer to start of data */
	size_t len;			/** Length of data in bytes */
};

/**
 * A { NULL, 0 }-chunk handy for initialization.
 */
extern chunk_t ChunkEmpty;

/**
 * Create a new chunk pointing to "ptr" with length "len"
 */
static inline chunk_t chunkCreate(uint8_t *ptr, size_t len)
{
	chunk_t chunk = {ptr, len};
	return chunk;
}

/**
 * Create a clone of a chunk pointing to "ptr"
 */
chunk_t chunkCreateClone(uint8_t *ptr, chunk_t chunk);

/**
 * Calculate length of multiple chunks
 *
 * The mode string specifies the number of chunks, and how to handle each of
 * them with a single character: 'c' for copy (allocate new chunk), 'm' for move
 * (free given chunk) or 's' for sensitive-move (clear given chunk, then free).
 */
size_t chunkLen(const char *mode, ...);

/**
 * Concatenate chunks into a chunk pointing to "ptr".
 *
 * The mode string specifies the number of chunks, and how to handle each of
 * them with a single character: 'c' for copy (allocate new chunk), 'm' for move
 * (free given chunk) or 's' for sensitive-move (clear given chunk, then free).
 */
chunk_t chunkCreateCat(uint8_t *ptr, const char* mode, ...);

/**
 * Split up a chunk into parts, "mode" is a string of "a" (alloc),
 * "c" (copy) and "m" (move). Each letter say for the corresponding chunk if
 * it should get allocated on heap, copied into existing chunk, or the chunk
 * should point into "chunk". The length of each part is an argument before
 * each target chunk. E.g.:
 * chunk_split(chunk, "mcac", 3, &a, 7, &b, 5, &c, d.len, &d);
 */
void chunkSplit(chunk_t chunk, const char *mode, ...);

/**
 * Skip n bytes in chunk (forward pointer, shorten length)
 * NOTE: Use with caution
 */
static inline chunk_t chunkSkip(chunk_t chunk, size_t bytes)
{
	if (chunk.len > bytes) {
		chunk.ptr += bytes;
		chunk.len -= bytes;
		return chunk;
	}
	return chunk_empty;
}

/**
 * Skip a leading zero-valued byte
 */
static inline chunk_t chunkSkipZero(chunk_t chunk)
{
	if (chunk.len > 1 && *chunk.ptr == 0x00) {
		chunk.ptr++;
		chunk.len--;
	}
	return chunk;
}

/**
 *  Compare two chunks, returns zero if a equals b
 *  or negative/positive if a is small/greater than b
 */
int32_t chunkCompare(chunk_t a, chunk_t b);


/**
 * Free contents of a chunk
 */
static inline void chunkFree(chunk_t *chunk)
{
	free(chunk->ptr);
	*chunk = chunk_empty;
}

/**
 * Overwrite the contents of a chunk and free it
 */
static inline void chunkClear(chunk_t *chunk)
{
	if (chunk->ptr) {
		memwipe(chunk->ptr, chunk->len);
		chunkFree(chunk);
	}
}

/**
 * Compare two chunks for equality,
 * NULL chunks are never equal.
 */
static inline bool chunkEquals(chunk_t a, chunk_t b)
{
	return a.ptr != NULL && b.ptr != NULL &&
			a.len == b.len && memcmp(a.ptr, b.ptr, a.len) == 0;
}

/**
 * mmap() a file to a chunk
 *
 * The returned chunk structure is allocated from heap, but it must be freed
 * through chunkUnmap(). A user may alter the chunk ptr or len, but must pass
 * the chunk pointer returned from chunkMap() to chunkUnmap() after use.
 *
 * On error, errno is set appropriately.
 *
 * @param path			path of file to map
 * @param wr			TRUE to sync writes to disk
 * @return				mapped chunk, NULL on error
 */
chunk_t *chunkMap(char *path, bool wr);

/**
 * munmap() a chunk previously mapped with chunkMap()
 *
 * When unmapping a writeable map, the return value should be checked to
 * ensure changes landed on disk.
 *
 * @param chunk			pointer returned from chunkMap()
 * @return				TRUE of changes written back to file
 */
bool chunkUnmap(chunk_t *chunk);

/**
 * Computes a 32 bit hash of the given chunk.
 *
 * Compared to chunkHash() this will always calculate the same output for the
 * same input.  Therefore, it should not be used for hash tables (to prevent
 * hash flooding).
 *
 * @note This hash is not intended for cryptographic purposes.
 *
 * @param chunk			data to hash
 * @return				hash value
 */
uint32_t chunkHashStatic(chunk_t chunk);

#ifdef HAVE_PRINTF_HOOK_H
/**
 * printf hook function for chunk_t.
 *
 * Arguments are:
 *	chunk_t *chunk
 * Use #-modifier to print a compact version
 * Use +-modifier to print a compact version without separator
 */
int32_t chunkPrintfHook(printfHookData_t *data, 
						printfHookSpec_t *spec,
						const void *const *args);

#endif /* HAVE_PRINTF_HOOK_H */

/**
 * Initialize a chunk from a string, not containing 0-terminator
 */
#define chunkFromStr(str) ({char *x = (str); chunkCreate((uint8_t*)x, strlen(x));})


#ifdef __cplusplus
}
#endif

#endif /* _CHELP_CHUNK_H */
