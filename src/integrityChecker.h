#ifndef _CHELP_INTEGRITYCHECKER_H
#define _CHELP_INTEGRITYCHECKER_H 1

/**
 * Code integrity checker to detect non-malicious file manipulation.
 *
 * The integrity checker reads the checksums from a separate library
 * libchecksum.so to compare the checksums.
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct integrityChecker_t integrityChecker_t;
typedef struct integrityChecksum_t integrityChecksum_t;

/**
 * Struct to hold a precalculated checksum, implemented in the checksum library.
 */
struct integrityChecksum_t {
	char *name;			/**!< name of the checksum */
	size_t fileLen;		/**!< size in bytes of the file on disk */
	uint32_t file;		/**!< checksum of the file on disk */
	size_t segmentLen;	/**!< size in bytes of executable segment in memory */
	uint32_t segment;	/**!< checksum of the executable segment in memory */
};

/**
 * Create a integrityChecker instance.
 *
 * @param checksum_library		library containing checksums
 */
integrityChecker_t *integrityCheckerCreate(char *checksum_library);

/**
 * Destroy a integrityChecker_t.
 */
void integrityCheckerDestroy(integrity_checker_t *this);

/**
 * Check the integrity of a file on disk.
 *
 * @param name		name to lookup checksum
 * @param file		path to file
 * @return			TRUE if integrity tested successfully
 */
bool integrityCheckerCheckFile(integrityChecker_t *this, char *name, char *file);

/**
 * Build the integrity checksum of a file on disk.
 *
 * @param file		path to file
 * @param len		return length in bytes of file
 * @return			checksum, 0 on error
 */
uint32_t integrityCheckerBuildFile(integrityChecker_t *this, char *file, size_t *len);

/**
 * Check the integrity of the code segment in memory.
 *
 * @param name		name to lookup checksum
 * @param sym		a symbol in the segment to check
 * @return			TRUE if integrity tested successfully
 */
bool integrityCheckerCheckSegment(integrityChecker_t *this, char *name, void *sym);

/**
 * Build the integrity checksum of a code segment in memory.
 *
 * @param sym		a symbol in the segment to check
 * @param len		return length in bytes of code segment in memory
 * @return			checksum, 0 on error
 */
uint32_t integrityCheckerBuildSegment(integrityChecker_t *this, void *sym, size_t *len);

/**
 * Check both, on disk file integrity and loaded segment.
 *
 * @param name		name to lookup checksum
 * @param sym		a symbol to look up library and segment
 * @return			TRUE if integrity tested successfully
 */
bool integrityCheckerCheck(integrityChecker_t *this, char *name, void *sym);

#ifdef __cplusplus
}
#endif


#endif /* _CHELP_INTEGRITYCHECKER_H */
