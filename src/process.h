#ifndef _CHELP_PROCESS_H
#define _CHELP_PROCESS_H 1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct process_t process_t;

/**
 * Spawn a child process with redirected I/O.
 *
 * Forks the current process, optionally redirects stdin/out/err to the current
 * process, and executes the provided program with arguments.
 *
 * The process to execute is specified as argv[0], followed by the process
 * arguments, followed by NULL. envp[] has a NULL terminated list of environment arguments
 * to invoke the process with.
 *
 * If any of in/out/err is given, stdin/out/err from the child process get
 * connected over pipe()s to the caller. If close_all is TRUE, all other
 * open file descriptors get closed, regardless of any CLOEXEC setting.
 *
 * A caller must close all of the returned file descriptors to avoid file
 * descriptor leaks.
 *
 * A non-NULL return value does not guarantee that the process has been
 * invoked successfully.
 *
 * @param argv		NULL terminated process arguments, with argv[0] as program
 * @param envp		NULL terminated list of environment variables
 * @param in		pipe fd returned for redirecting data to child stdin
 * @param out		pipe fd returned to redirect child stdout data to
 * @param err		pipe fd returned to redirect child stderr data to
 * @param close_all	close all open file descriptors above 2 before execve()
 * @return			process, NULL on failure
 */
process_t* processStart(char *const argv[], char *const envp[],
						 int32_t *in, int32_t *out, int32_t *err, bool close_all);

/**
 * Spawn a command in a shell child process.
 *
 * Same as processStart(), but passes a single command to a shell, such as
 * "sh -c". See processStart() for I/O redirection notes.
 *
 * @param envp		NULL terminated list of environment variables
 * @param in		pipe fd returned for redirecting data to child stdin
 * @param out		pipe fd returned to redirect child stdout data to
 * @param err		pipe fd returned to redirect child stderr data to
 * @param fmt		printf format string for command
 * @param ...		arguments for fmt
 * @return			process, NULL on failure
 */
process_t* processStartShell(char *const envp[], int32_t *in, int32_t *out, int32_t *err,
							   char *fmt, ...);

/**
 * Wait for a started process to terminate.
 *
 * The process object gets destroyed by this call, regardless of the
 * return value.
 *
 * The returned code is the exit code, not the status returned by waitpid().
 * If the program could not be executed or has terminated abnormally
 * (by signals etc.), FALSE is returned.
 *
 * @param code	process exit code, set only if TRUE returned
 * @return		TRUE if program exited normally through exit()
 */
bool processWait(process_t *this, int32_t *code);

#ifdef __cplusplus
}
#endif


#endif /* _CHELP_PROCESS_H */
