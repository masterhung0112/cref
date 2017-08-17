#ifndef _CHELP_COMMAND_H
#define _CHELP_COMMAND_H 1

/**
 * static int logcmd() {
 *	command_format_options_t format = COMMAND_FORMAT_NONE;
 *  char *arg;
 *	int ret;
 *	
 *	while(TRUE) {
 *		switch(commandGetOpt(&arg) {
 *			case 'h':
 *				return commandUsage(NULL);
 *			case 'P':
 *				format |= COMMAND_FORMAT_PRETTY;
 *			case 'r':
 *				format |= COMMAND_FORMAT_RAW;
 *				continue;
 *			case EOF:
 *				break;
 *			default:
 *				return commandUsage("invalid --log option");
 *		}
 *		break;
 *	}
 * }
 *
 * command_register((command_t) {
 *  logcmd, 'T', "log", "trace logging output",
 *  {"[--raw|--pretty]"},
 *  {
 *   	{"help", 'h', 0, "Show usage information"},
 *		{"raw",  'r', 0, "dump raw response message"},
 *		{"pretty", 'P', 0, "dump raw response message in pretty print"}
 * });
 *
 */


#ifndef __cplusplus
extern "C" {
#endif

#define MAX_COMMANDS 20

/**
 * Maximum number of options in a command
 */
#define MAX_OPTIONS 32

/**
 * Maximum number of usage summary lines
 */
#define MAX_LINES 	10

typedef struct command_t command_t;
typedef struct command_option_t command_option_t;

struct command_option_t {
	char *name;		/**!< long option string of option */
	char op;		/**!< short option character of a option */
	int arg;		/**!< expected argument to option, no/req/opt_argument */
	char *desc;		/**!< description of the option */
};

struct command_t {
	int (*call)();  /**!< Function implement a command */
	char op;		/**!< Short option character */
	char *cmd;		/**!< long option string */
	char *description; /**!< Description of command */
	char *line[MAX_LINES]; /**! usage summary of the command */
	command_option_t options[MAX_OPTIONS]; /**!< List of options the command accepts */
};

/**
 * Get the next option, as with getopt
 */
int commandGetOpt(char **arg);

/**
 * Register a command
 */
void commandRegister(command_t command);

/**
 * Dispatch a commands
 */
int commandDispatch(int argc, char *argv[]);

/**
 * Show usage information of active command.
 */
int commandUsage(char *error, ...);


#ifndef __cplusplus
}
#endif

#endif /* _CHELP_COMMAND_H */
