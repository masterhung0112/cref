#include "printfHookBuiltin.h"

/* CHAR_BIT */
/* uintmax_t, uintptr_t */
/* strlen, strnlen, strerror */

/**
 * Printf format modifier flags
 */
typedef enum {
	FL_ZERO		= 0x01,
	FL_MINUS	= 0x02,
	FL_PLUS		= 0x04,
	FL_TICK		= 0x08,
	FL_SPACE	= 0x10,
	FL_HASH		= 0x20,
	FL_SIGNED	= 0x40,
	FL_UPPER	= 0x80,
} bpf_flag_t;

/**
 * Size of format string arguments
 */
typedef enum {
	RNK_CHAR		= -2,
	RNK_SHORT		= -1,
	RNK_INT			=  0,
	RNK_LONG		=  1,
	RNK_LONGLONG	=  2,

	RNK_INTMAX		= RNK_LONGLONG,
	RNK_SIZE_T		= RNK_LONG,
	RNK_PTRDIFF_T	= RNK_LONG,

	RNK_MIN			= RNK_CHAR,
	RNK_MAX			= RNK_LONGLONG,
} bpf_rank_t;

/**
 * Printf specifier Parser state
 */
typedef enum {
	ST_NORMAL,		/**!< Ground state */
	ST_FLAGS,		/**!< Special flags */
	ST_WIDTH,		/**!< Field width */
	ST_PREC,		/**!< Field precision */
	ST_MODIFIERS,	/**!< Length or conversion modifiers */
} bpf_state_t;

#define EMIT(x) ({ if (o<n){*q++ = (x);} ++o; })

int builtin_vsnprintf(char *buffer, size_t n, const char *format, va_list ap)
{
	const char *p = format;
	char ch;
	char *q = buffer;
	size_t sz;
	size_t o = 0;	/* Number of characters output */
	int base;
	int rank = RNK_INT;
	int width = 0;
	int prec = -1;
	uintmax_t val = 0;
	bpf_flag_t flag = 0;
	bpf_state_t state = ST_NORMAL;
	const char *sarg;	/* %s string argument */
	char carg;			/* %c char argument */
	int slen;			/* String length */
	
	while ((ch = *p++)) {
		switch(state) {
			case ST_NORMAL:
			{
				if (ch == '%') {
					state = ST_FLAGS;
					flags = 0;
					rank = RNK_INT;
					width = 0;
					prec = -1;
				} else {
					EMIT(ch);
				}
				break;
			} /* case ST_NORMAL */
			case ST_FLAGS:
			{
				switch (ch)
				{
					case '-':
						flags |= FL_MINUS;
						break;
					case '+':
						flags |= FL_PLUS;
						break;
					case '\'':
						flags |= FL_TICK;
						break;
					case ' ':
						flags |= FL_SPACE;
						break;
					case '#':
						flags |= FL_HASH;
						break;
					case '0':
						flags |= FL_ZERO;
						break;
					default:
						state = ST_WIDTH;
						/* Process this character again */
						p--;
						break;
				}
				break;
			} /* case ST_FLAGS */
			case ST_WIDTH:
			{
				if (ch >= '0' && ch <= '9')	{
					width = width * 10 + (ch - '0');
				} else if (ch == '*') {
					width = va_arg(ap, int32_t);
					if (width < 0) {
						width = -width;
						flags |= FL_MINUS;
					}
				} else if (ch == '.') {
					/* Precision given */
					prec = 0;
					state = ST_PREC;
				} else {
					state = ST_MODIFIERS;
					/* Process this character again */
					p--;
				}
				break;
			} /* case ST_WIDTH */
			case ST_PREC:
			{
				if (ch >= '0' && ch <= '9') {
					prec = prec * 10 + (ch - '0');
				} else if (ch == '*') {
					prec = va_arg(ap, int);
					if (prec < 0) {
						prec = -1;
					}
				} else {
					state = ST_MODIFIERS;
					/* Process this character again */
					p--;
				}
				break;
			} /* case ST_PREC */
			case ST_MODIFIERS:
			{
				switch (ch)
				{
					/* Length modifiers - nonterminal sequences */
					case 'h':
						rank--;
						break;
					case 'l':
						rank++;
						break;
					case 'j':
						rank = RNK_INTMAX;
						break;
					case 'z':
						rank = RNK_SIZE_T;
						break;
					case 't':
						rank = RNK_PTRDIFF_T;
						break;
					case 'L':
					case 'q':
						rank += 2;
						break;
					default:
					{
						/* Output modifiers - terminal sequences */

						/* Next state will be normal */
						state = ST_NORMAL;

						/* Canonicalize rank */
						if (rank < RNK_MIN)	{
							rank = RNK_MIN;
						} else if (rank > RNK_MAX) {
							rank = RNK_MAX;
						}
						
						switch (ch)
						{
							case 'p':
							{
								/* Pointer */
								base = 16;
								prec = (CHAR_BIT*sizeof(void *)+3)/4;
								flags |= FL_HASH;
								val = (uintmax_t)(uintptr_t)va_arg(ap, void *);
								goto is_integer;
							}
							case 'd':
							case 'i':
							{
								/* Signed decimal output */
								base = 10;
								flags |= FL_SIGNED;
								switch (rank)
								{
									case RNK_CHAR:
										/* Yes, all these casts are needed... */
										val = (uintmax_t)(intmax_t)(signed char)
												va_arg(ap, signed int);
										break;
									case RNK_SHORT:
										val = (uintmax_t)(intmax_t)(signed short)
												va_arg(ap, signed int);
										break;
									case RNK_INT:
										val = (uintmax_t)(intmax_t)
												va_arg(ap, signed int);
										break;
									case RNK_LONG:
										val = (uintmax_t)(intmax_t)
												va_arg(ap, signed long);
										break;
									case RNK_LONGLONG:
										val = (uintmax_t)(intmax_t)
												va_arg(ap, signed long long);
										break;
								}
								goto is_integer;
							} /* case 'd': case 'i': */
							case 'o':
							{
								/* Octal */
								base = 8;
								goto is_unsigned;
							}
							case 'u':
							{
								/* Unsigned decimal */
								base = 10;
								goto is_unsigned;
							}
							case 'X':
							{
								/* Upper case hexadecimal */
								flags |= FL_UPPER;
								/* fall through */
							}
							case 'x':
							{
								/* Hexadecimal */
								base = 16;
								goto is_unsigned;
							}
							is_unsigned:
							{
								switch (rank) {
									case RNK_CHAR:
										val = (uintmax_t)(unsigned char)
												va_arg(ap, unsigned int);
										break;
									case RNK_SHORT:
										val = (uintmax_t)(unsigned short)
												va_arg(ap, unsigned int);
										break;
									case RNK_INT:
										val = (uintmax_t)
												va_arg(ap, unsigned int);
										break;
									case RNK_LONG:
										val = (uintmax_t)
												va_arg(ap, unsigned long);
										break;
									case RNK_LONGLONG:
										val = (uintmax_t)
												va_arg(ap, unsigned long long);
										break;
								}
								goto is_integer;
							} /* is_unsigned */
							is_integer:
							{
								sz = formatInt(q, (o < n) ? n - o : 0,
												val, flags, base, width, prec);
								q += sz;
								o += sz;
								break;
							} /* is_integer */
							case 'c':
							{
								/* Character */
								carg = (char)va_arg(ap, int32_t);
								sarg = &carg;
								slen = 1;
								goto is_string;
							}
							case 's':
							{
								/* String */
								sarg = va_arg(ap, const char *);
								sarg = sarg ? sarg : "(null)";
								slen = prec != -1 ? strnlen(sarg, prec)
												  : strlen(sarg);
								goto is_string;
							}
							case 'm':
							{
								/* glibc error string */
								sarg = strerror(errno);
								slen = strlen(sarg);
								goto is_string;
							}
							is_string:
							{
								char sch;
								int i;

								if (prec != -1 && slen > prec) {
									slen = prec;
								}

								if (width > slen && !(flags & FL_MINUS)) {
									char pad = (flags & FL_ZERO) ? '0' : ' ';
									while (width > slen) {
										EMIT(pad);
										width--;
									}
								}
								
								for (i = slen; i; i--) {
									sch = *sarg++;
									EMIT(sch);
								}
								
								if (width > slen && (flags & FL_MINUS)) {
									while (width > slen) {
										EMIT(' ');
										width--;
									}
								}
								break;
							} /* is_string */
							case 'A':
							{
								base = 16;
								flags |= FL_UPPER;
								goto is_double;
							}
							case 'E':
							case 'G':
							{
								/* currently not supported, fall */
							}
							case 'F':
							{
								base = 10;
								flags |= FL_UPPER;
								goto is_double;
							}
							case 'a':
							{
								base = 16;
								goto is_double;
							}
							case 'e':
							case 'g':
							{
								/* currently not supported, fall */
							}
							case 'f':
							{
								base = 10;
								goto is_double;
							}
							is_double:
							{
								double dval;

								dval = va_arg(ap, double);
								if (isinf(dval))
								{
									if (isgreater(dval, 0.0))
									{
										sarg = flags & FL_UPPER ? "INF" : "inf";
									}
									else
									{
										sarg = flags & FL_UPPER ? "-INF" : "-inf";
									}
									slen = strlen(sarg);
									goto is_string;
								}
								if (isnan(dval))
								{
									sarg = flags & FL_UPPER ? "NAN" : "nan";
									slen = strlen(sarg);
									goto is_string;
								}
								sz = format_double(q, (o < n) ? n - o : 0,
												dval, flags, base, width, prec);
								q += sz;
								o += sz;
								break;
							} /* is_double */
							case 'n':
							{
								/* Output the number of characters written */
								switch (rank)
								{
									case RNK_CHAR:
										*va_arg(ap, signed char *) = o;
										break;
									case RNK_SHORT:
										*va_arg(ap, signed short *) = o;
										break;
									case RNK_INT:
										*va_arg(ap, signed int *) = o;
										break;
									case RNK_LONG:
										*va_arg(ap, signed long *) = o;
										break;
									case RNK_LONGLONG:
										*va_arg(ap, signed long long *) = o;
										break;
								}
								break;
							} /* case 'n' */
							default:
							{
								printfHookHandler_t *handler;

								handler = hooks->get(hooks, (void*)(uintptr_t)ch);
								if (handler) {
									const void *args[ARGS_MAX];
									int i, iargs[ARGS_MAX];
									void *pargs[ARGS_MAX];
									
									printfHookSpec_t spec =  {
										.hash = flags & FL_HASH,
										.plus = flags & FL_PLUS,
										.minus = flags & FL_MINUS,
										.width = width,
									};
									
									printfHookData_t data = {
										.q = q,
										.n = (o < n) ? n - o : 0,
									};
									
									for (i = 0; i < handler->numargs; i++) {
										switch (handler->argtypes[i])
										{
											case PRINTF_HOOK_ARGTYPE_INT:
												iargs[i] = va_arg(ap, int32_t);
												args[i] = &iargs[i];
												break;
											case PRINTF_HOOK_ARGTYPE_POINTER:
												pargs[i] = va_arg(ap, void*);
												args[i] = &pargs[i];
												break;
										}
									}
									sz = handler->hook(&data, &spec, args);
									q += sz;
									o += sz;
								} else {
									EMIT(ch); /* Anything else, including % */
								}
								break;
							}
						} /* switch (ch) */
					} /* default: Output modifiers - terminal sequences */
				} /* case ST_MODIFIERS: switch (ch) */
			} /* case ST_MODIFIERS */
		} /* switch(state) */
	}
}

/**
 * builtin-printf variant of printInHook()
 */
size_t printInHook(printfHookData_t *data, char *fmt, ...)
{
	int written;
	va_list args;

	va_start(args, fmt);
	written = builtin_vsnprintf(data->q, data->n, fmt, args);
	va_end(args);

	if (written > data->n) {
		data->q += data->n;
		data->n = 0;
	} else {
		data->q += written;
		data->n -= written;
	}
	
	return written;
}

