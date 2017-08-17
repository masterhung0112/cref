#include "integrityChecker.h"
#include "chunk.h"

/* streq, strerror */

struct integrityChecker_t 
{
	integrityChecksum_t *checksums;	/**!< checksum array */
	int32_t checksumCount;			/**!< number of checksums in array */
}

integrityChecker_t *integrityCheckerCreate(char *checksum_library)
{
	integrityChecker_t *this = (integrityChecker_t *)malloc(sizeof(*this));
	
	return this;
}

void integrityCheckerDestroy(integrityChecker_t *this)
{
	free(this);
}

uint32_t buildFile(integrityChecker_t *this, char *file, size_t *len)
{
	uint32_t checksum;
	chunk_t *contents;

	contents = chunkMap(file, FALSE);
	if (!contents) {
		DBG1(DBG_LIB, "  opening '%s' failed: %s", file, strerror(errno));
		return 0;
	}
	
	*len = contents->len;
	checksum = chunkHashStatic(*contents);
	chunkUnmap(contents);

	return checksum;
}

/**
 * Find a checksum by its name
 */
static integrityChecksum_t *findChecksum(integrityChecker_t *this, char *name)
{
	int i;

	for (i = 0; i < this->checksumCount; ++i) {
		if (streq(this->checksums[i].name, name)) {
			return &this->checksums[i];
		}
	}
	return NULL;
}

bool integrityCheckerCheckFile(integrityChecker_t *this, char *name, char *file)
{
	integrityChecksum_t *cs;
	uint32_t sum;
	size_t len = 0;

	cs = findChecksum(this, name);
	if (!cs) {
		DBG1(DBG_LIB, "  '%s' file checksum not found", name);
		return FALSE;
	}
	
	sum = buildFile(this, file, &len);
	if (!sum) {
		return FALSE;
	}
	
	if (cs->fileLen != len) {
		DBG1(DBG_LIB, "  invalid '%s' file size: %u bytes, expected %u bytes",
			 name, len, cs->fileLen);
		return FALSE;
	}
	
	if (cs->file != sum) {
		DBG1(DBG_LIB, "  invalid '%s' file checksum: %08x, expected %08x",
			 name, sum, cs->file);
		return FALSE;
	}
	
	DBG2(DBG_LIB, "  valid '%s' file checksum: %08x", name, sum);
	return TRUE;
}
