#include "chunk.h"

/* memcpy, malloc, memcmp */
#include <stdargs.h> /* va_list, va_arg, va_end */
/* TRUE */
/* min */

/**
 * Empty chunk.
 */
chunk_t ChunkEmpty = { NULL, 0 };

chunk_t chunkCreateClone(uint8_t *ptr, chunk_t chunk)
{
	chunk_t clone = ChunkEmpty;
	
	if (chunk.ptr && chunk.len > 0) {
		clone.ptr = chunk.ptr;
		clone.len = chunk.len;
		memcpy(clone.ptr, chunk.ptr, chunk.len);
	}
	
	return clone;
}

size_t chunkLen(const char *mode, ...)
{
	va_list chunks;
	size_t len = 0;

	va_start(chunks, mode);
	while (TRUE) {
		switch (*mode++) {
			case 'm':
			case 'c':
			case 's':
			{
				chunk_t ch = va_arg(chunks, chunk_t);
				len += ch.len;
				continue;
			}
			default:
				break;
		}
		break;
	}
	va_end(chunks);
	return len;
}

chunk_t chunkCreateCat(uint8_t *ptr, const char* mode, ...)
{
	va_list chunks;
	chunk_t construct = chunkCreate(ptr, 0);

	va_start(chunks, mode);
	while (TRUE)
	{
		bool freeChunk = FALSE, 
			clearChunk = FALSE;
		chunk_t ch;

		switch (*mode++) {
			case 's':
				clearChunk = TRUE;
				/* FALL */
			case 'm':
				freeChunk = TRUE;
				/* FALL */
			case 'c':
				ch = va_arg(chunks, chunk_t);
				memcpy(ptr, ch.ptr, ch.len);
				ptr += ch.len;
				construct.len += ch.len;
				if (clearChunk) {
					chunkClear(&ch);
				} else if (freeChunk) {
					free(ch.ptr);
				}
				continue;
			default:
				break;
		}
		break;
	}
	va_end(chunks);

	return construct;
}

void chunkSplit(chunk_t chunk, const char *mode, ...)
{
	va_list chunks;
	uint32_t len;
	chunk_t *ch;

	va_start(chunks, mode);
	while (TRUE) {
		if (*mode == '\0') {
			break;
		}
		len = va_arg(chunks, uint32_t);
		ch = va_arg(chunks, chunk_t*);
		/* a null chunk means skipping len bytes */
		if (NULL == ch) {
			chunk = chunkSkip(chunk, len);
			continue;
		}
		switch (*mode++) {
			case 'm':
			{
				ch->len = min(chunk.len, len);
				if (ch->len) {
					ch->ptr = chunk.ptr;
				} else {
					ch->ptr = NULL;
				}
				chunk = chunkSkip(chunk, ch->len);
				continue;
			}
			case 'a':
			{
				ch->len = min(chunk.len, len);
				if (ch->len) {
					ch->ptr = malloc(ch->len);
					memcpy(ch->ptr, chunk.ptr, ch->len);
				} else {
					ch->ptr = NULL;
				}
				chunk = chunkSkip(chunk, ch->len);
				continue;
			}
			case 'c':
			{
				ch->len = min(ch->len, chunk.len);
				ch->len = min(ch->len, len);
				if (ch->len) {
					memcpy(ch->ptr, chunk.ptr, ch->len);
				} else {
					ch->ptr = NULL;
				}
				chunk = chunkSkip(chunk, ch->len);
				continue;
			}
			default:
				break;
		}
		break;
	}
	va_end(chunks);
}

int32_t chunkCompare(chunk_t a, chunk_t b)
{
	int32_t compareLen = a.len - b.len;
	int32_t len = (compareLen < 0) ? a.len : b.len;

	if (compareLen != 0 || len == 0) {
		return compareLen;
	}
	
	return memcmp(a.ptr, b.ptr, len);
}

#ifdef HAVE_PRINTF_HOOK_H

int32_t chunkPrintfHook(printfHookData_t *data, printfHookSpec_t *spec,
					  const void *const *args)
{
	chunk_t *chunk = *((chunk_t**)(args[0]));
	bool first = TRUE;
	chunk_t copy = *chunk;
	int32_t written = 0;

	if (!spec->hash && !spec->plus) {
		uint32_t chunkLen = chunk->len;
		const void *newArgs[] = {&chunk->ptr, &chunkLen};
		return mem_printf_hook(data, spec, newArgs);
	}

	while (copy.len > 0) {
		if (first) {
			first = FALSE;
		} else if (!spec->plus) {
			written += printInHook(data, ":");
		}
		written += printInHook(data, "%02x", *copy.ptr++);
		copy.len--;
	}
	return written;
}

#endif /* #ifdef HAVE_PRINTF_HOOK_H */


