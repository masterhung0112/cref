#include "command.h"

#define _GNU_SOURCE
#include <getopt.h>

static command_t cmds[MAX_COMMANDS];

static int active = 0; /* Active command */

static int registered = 0; /* Number of registered commands */

static int helpIdx = 0; /* Help command index */

static int argc;
static char **argv;

static options_t *options;

/**
 * Global options used by all subcommands
 */
static struct option command_opts[MAX_COMMANDS > MAX_OPTIONS ?
								MAX_COMMANDS : MAX_OPTIONS];
/**
 * Global optstring used by all subcommands
 */
static char command_optstring[(MAX_COMMANDS > MAX_OPTIONS ?
								MAX_COMMANDS : MAX_OPTIONS) * 3];

int commandGetOpt(char **arg)
{
	int op;
	while (TRUE)
	{
		op = getopt_long(argc, argv, command_optstring, command_opts, NULL);
		switch (op) {
			case '+':
			case 'v':
			case 'u':
				continue;
			default:
				*arg = optarg;
				return op;
		}
	}
}

void commandRegister(command_t command)
{
	int i;
	if (registered == MAX_COMMANDS) {
		return;
	}
	
	cmds[registered] = command;
	if (!active) {
		/* Add default option */
	}
	++registered;
}

int commandUsage(char *error, ...)
{
	va_list args;
	FILE *out = stdout;
	int i;
	
	if (error) {
		out = stderr;
		fprintf(out, "Error: ");
		va_start(args, error);
		vprintf(out, error, args);
		va_end(args);
		fprintf(out, "\n");
	}
	if (active == helpIdx) {
		/* Display more information */
	}
	fprintf(out, "usage:\n");
	if (active == helpIdx) {
		for (i = 0; i < MAX_COMMANDS && cmds[i].cmd; ++i) {
			fprintf(out, "   swanctl --%-16s (-%c)    %s\n",
				cmds[i].cmd, cmds[i].op, cmds[i].description);
		}
	} else {
		for (i = 0; i < MAX_LINES && cmds[active].lines[i]; ++i) {
			if (i == 0) {
				fprintf(out, "    swanctl --%s %s\n",
					cmds[active].cmd, cmds[active].line[i]);
			} else {
				fprintf(out, "                   %s\n", cmds[active].line[i]);
			}
		}
		for (i = 0; i < cmds[active].options[i].name; ++i) {
			fprintf(out, "             --%-15s (-%c)  %s\n",
				cmds[active].options[i].name, cmds[active].options[i].op,
				cmds[active].options[i].desc);
		}
	}
	return error != NULL;
}

/**
 * Build command_opts/command_str for active command
 */
static void build_opts()
{
	int i, pos = 0;
	
	memset(command_opts, 0, sizeof(commands_opts));
	memset(command_optstring, 0, sizeof(command_optstring));
	
	if (active == helpIdx) {
		for (i = 0; i < MAX_COMMANDS && cmds[i].cmd; ++i) {
			commands_opts[i].name = cmds[i].cmd;
			commands_opts[i].val = cmds[i].op;
			commands_optstring[i].val = cmds[i].op;
		}
	} else {
		for (i = 0; cmds[active].options[i].name; ++i) {
			commands_opts[i].name = cmds[active].options[i].name;
			commands_opts[i].has_arg = cmds[active].options[i].arg;
			commands_opts[i].val = cmds[active].options[i].op;
			commands_optstring[pos++] = cmds[active].options[i].op;
			switch (cmds[active].options[i].arg) {
				case optional_argument:
					command_optstring[pos++] = ':';
				case required_argument:
					command_optstring[pos++] = ':';
				case no_argument:
				default:
					break;
			}
		}
	}
}

/**
 * Process options common for all commands
 */
static bool processCommonOpts()
{
	while (TRUE) {
		switch(getopt_long(argc, argv, command_optstring, command_opts, NULL)) {
			case '+':
				if (!options->from(options, optarg, &argc, &argv, optind)) {
					return FALSE;
				}
				continue;
			case 'v':
				/*TODO: SET DEBUG LEVEL */
				continue;
			default:
				continue;
			case '?':
				return FALSE;
			case EOF:
				return TRUE;
		}
	}
}

static int callCommand(command_t *cmd)
{
	int ret;
	ret = cmd->call();
	return ret;
}

/**
 * Dispatch commands
 */
int commandDispatch(int c, char *v[])
{
	int op, i;
	
	options = options_create();
	/* atexit(cleanup); */
	active = helpIdx = registered;
	argc = c;
	argv = v;
	
	commandRegister((command_t){NULL, 'h', "help", "show usage information"});
	
	build_opts();
	op = getopt_long(c, v, command_optstring, command_opts, NULL);
	for (i = 0; i < MAX_COMMANDS && cmds[i].cmd; ++i) {
		if (cmds[i].op == op) {
			active = i;
			build_opts();
			if (helpIdx == i) {
				return commandUsage(NULL);
			}
			if (!processCommonOpts()) {
				return commandUsage("invalid options");
			}
			optind = 2;
			return callCommand(&cmds[i]);
		}
	}
	return commandUsage(c > 1 ? "invalid operation" : NULL);
}
