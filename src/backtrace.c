#include "backtrace.h"

#include <stdarg.h> /* va_list, va_start, va_end */
/* max */
/* calloc */
/* memcpy */
/* println */
/* fputs */


struct backtrace_t
{
	backtrace_t public;	/**!< Public backtrace_t interface */
	int32_t frameCount;	/**!< Number of stacks frames obtained in stack_frames */
	void *frames[];		/**!< Recorded stack frames */
}

/**
 * Write a format string with arguments to a FILE line, if it is NULL to DBG
 */
static void println(FILE *file, char *format, ...)
{
	char buf[512];
	va_list args;

	va_start(args, format);
	if (file) {
		vfprintf(file, format, args);
		fputs("\n", file);
	} else {
		vsnprintf(buf, sizeof(buf), format, args);
		DBG1(DBG_LIB, "%s", buf);
	}
	va_end(args);
}

void backtraceInit()
{
	
}

void backtraceDeinit()
{

}

backtrace_t *backtraceCreate(int32_t skip)
{
	backtrace_t *this;
	void *frames[50];
	int frameCount = 0;
	
#ifdef HAVE_LIBUNWIND_H
	frameCount = backtrace_unwind(frames, countof(frames));
#elif defined(HAVE_BACKTRACE)
	frameCount = backtrace(frames, countof(frames));
#elif defined(HAVE_DBGHELP)
	frameCount = backtrace_win(frames, countof(frames));
#elif defined(WIN32)
	frameCount = CaptureStackBackTrace(skip, countof(frames), frames, NULL);
	skip = 0;
#endif
	
	frameCount = max(frameCount - skip, 0);
	this = calloc(1, sizeof(backtrace_t) + frame_count * sizeof(void*));
	memcpy(this->frames, frames + skip, frame_count * sizeof(void*));
	this->frameCount = frameCount;

	return &this->public;
}

bool backtraceEquals(backtrace_t *this, backtrace_t *other)
{
	int32_t i;

	if (this == other) {
		return TRUE;
	}
	
	if (this->frameCount != other->frameCount) {
		return FALSE;
	}
	
	for (i = 0; i < this->frameCount; i++) {
		if (this->frames[i] != other->frames[i]) {
			return FALSE;
		}
	}
	
	return TRUE;
}

backtrace_t* backtraceClone(backtrace_t *this)
{
	backtrace_t *clone;

	clone = calloc(1, sizeof(backtrace_t) + this->frameCount * sizeof(void*));
	memcpy(clone->frames, this->frames, this->frameCount * sizeof(void*));
	clone->frameCount = this->frameCount;
	
	return clone;
}

void backtraceDestroy(backtrace_t *this)
{
	free(this);
}

void backtraceDump(char *label, FILE *file, bool detailed)
{
	backtrace_t *backtrace;

	backtrace = backtraceCreate(2);

	if (label) {
		println(file, "Debug backtrace: %s", label);
	}
	backtraceLog(backtrace, file, detailed);
	backtraceDestroy(backtrace);
}

void backtraceLog(backtrace_t *this, FILE *file, bool detailed)
{
	/*TODO: According to the library, add implementation */
}

bool backtraceContainsFunction(backtrace_t *this, char *function[], int32_t count)
{
	/*TODO: According to the library, add implementation */
	return FALSE;
}

#ifdef HAVE_ENUMERATOR_H

/**
 * Frame enumerator
 */
typedef struct {
	enumerator_t public;		/** implements enumerator_t */
	backtrace_t *bt;			/** reference to backtrace */
	int32_t i;					/** current position */
} frameEnumerator_t;

bool _frameEnumerate(frameEnumerator_t *this, void **addr)
{
	if (this->i < this->bt->frame_count) {
		*addr = this->bt->frames[this->i++];
		return TRUE;
	}
	return FALSE;
}

enumerator_t* backtraceCreateFrameEnumerator(backtrace_t *this)
{
	frameEnumerator_t *enumerator;

	enumerator = (frameEnumerator_t *)calloc(1, sizeof(*enumerator));
	
	enumerator->enumerate = (void*)_frameEnumerate;
	enumerator->destroy = (void*)free;
	enumerator->bt = this;
	
	return enumerator;
}

#endif /* ifdef HAVE_ENUMERATOR_H */

