/*
 * $Id: stdio_vfscanf.c,v 1.1.1.1 2004-07-26 16:31:48 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2004 by Olaf Barthel <olsen@sourcery.han.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   - Neither the name of Olaf Barthel nor the names of contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _STDLIB_NULL_POINTER_CHECK_H
#include "stdlib_null_pointer_check.h"
#endif /* _STDLIB_NULL_POINTER_CHECK_H */

/****************************************************************************/

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

#ifndef _LOCALE_HEADERS_H
#include "locale_headers.h"
#endif /* _LOCALE_HEADERS_H */

/****************************************************************************/

/*
 * Uncomment this to activate '%lld' support and the like. Better still,
 * define this is in the Makefile!
 */
/*#define USE_64_BIT_INTS*/

/****************************************************************************/

int
__vfscanf(FILE *stream, const char *format, va_list arg)
{
	enum format_size_t
	{
		format_size_default,
		format_size_long,
		format_size_long_long,
		format_size_short,
		format_size_long_double
	};

	enum format_size_t format_size;
	int total_num_chars_read = 0;
	int num_chars_processed;
	BOOL assignment_suppressed;
	int maximum_field_width;
	int result = EOF;
	int num_conversions = 0;
	int num_assignments = 0;
	int conversion_type;
	int c;

	ENTER();

	assert( stream != NULL && format != NULL );

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(stream == NULL || format == NULL)
		{
			errno = EFAULT;
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	if(__fgetc_check(stream) != OK)
		goto out;

	/* Just so we can detect errors and tell them apart from
	   an 'end of file' condition. */
	clearerr(stream);

	while((c = (*(unsigned char *)format)) != '\0')
	{
		D(("next format character is '%lc'",c));

		if(isspace(c))
		{
			BOOL finished = FALSE;

			/* Skip all blank spaces in the stream. */
			format++;

			while(TRUE)
			{
				c = __getc(stream);
				if(c == EOF)
				{
					/* Hit the end of the stream. Due to an error? */
					if(num_conversions == 0 || ferror(stream))
						goto out;

					finished = TRUE;
					break;
				}
				else if (isspace(c))
				{
					total_num_chars_read++;
				}
				else
				{
					/* End of the white space; we push the last
					 * character read back into the stream.
					 */
					if(ungetc(c,stream) == EOF)
						goto out;

					/* Resume scanning. */
					break;
				}
			}

			if(finished)
				break;

			/* Resume scanning. */
			continue;
		}
		else if (c != '%')
		{
			int d;

			SHOWMSG("next character must match exactly");

			/* Match the next character in the stream. */
			format++;

			d = __getc(stream);
			if(d == EOF)
			{
				SHOWMSG("end of file");

				/* Hit the end of the stream. Due to an error? */
				if(num_conversions == 0 || ferror(stream))
					goto out;

				break;
			}
			else if (c == d)
			{
				SHOWMSG("it matches");

				total_num_chars_read++;
			}
			else
			{
				SHOWMSG("it does not match");

				if(ungetc(d,stream) == EOF)
					goto out;

				break;
			}

			/* Resume scanning. */
			continue;
		}

		format++;

		c = (*format);

		if(c == '*')
		{
			/* If the first letter of the format specification
			 * is an asterisk, no output will be produced for
			 * this parameter.
			 */
			assignment_suppressed = TRUE;

			format++;

			c = (*format);
		}
		else
		{
			assignment_suppressed = FALSE;
		}

		maximum_field_width = -1;

		if('0' <= c && c <= '9')
		{
			int next_sum;
			int sum = 0;

			/* This could be the field width specification. */
			while(TRUE)
			{
				c = (*format);

				if('0' <= c && c <= '9')
				{
					next_sum = (10 * sum) + c - '0';
					if(next_sum < sum) /* overflow? */
						break;

					sum = next_sum;
					format++;
				}
				else
				{
					break;
				}
			}

			maximum_field_width = sum;
		}
		else if (c == '\0')
		{
			/* And that's the end of the string. */
			break;
		}

		c = (*format);

		if(c == 'h')
		{
			/* Parameter is a short integer. */
			format_size = format_size_short;
			format++;
		}
		else if (c == 'l')
		{
			/* Parameter is a long integer or a double precision floating point value. */
			format_size = format_size_long;
			format++;
		}
		else if (c == 'L')
		{
			/* Parameter is a long double floating point value. */
			format_size = format_size_long_double;
			format++;
		}
		else if (c == '\0')
		{
			/* And that's the end of the string. */
			break;
		}
		else
		{
			/* Parameter is a long integer or a single precision floating point value. */
			format_size = format_size_default;
		}

		/* Now for the conversion type. */
		c = (*format);
		if(c == '\0')
			break;

		#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
		{
			/* Check for long long parameters. */
			if(c == 'l')
			{
				if(format_size == format_size_long)
					format_size = format_size_long_long;

				format++;

				/* The conversion type follows. */
				c = (*format);
				if(c == '\0')
					break;
			}
		}
		#endif /* __GNUC__ */

		switch(c)
		{
			/* It's a pointer. */
			case 'p':

				conversion_type = 'x';
				format++;
				break;

			/* It's a floating point number. */
			case 'f':
			case 'e':
			case 'g':

				conversion_type = 'f';
				format++;
				break;

			case 'c':	/* character(s) */
			case 'd':	/* signed integer */
			case 'i':	/* signed integer in decimal, octal or hexadecimal format */
			case 'n':	/* number of characters processed */
			case 'o':	/* integer (octal) */
			case 's':	/* string */
			case 'u':	/* unsigned integer */
			case 'x':	/* unsigned integer in hexadecimal format */
			case '%':	/* the % character */
			case '[':	/* a range of characters */

				conversion_type = c;
				format++;
				break;

			default:

				/* Must be something else */
				continue;
		}

		/* Skip any initial whitespace characters. Exceptions are %c, %n and %[. */
		if(conversion_type != 'c' &&
		   conversion_type != 'n' &&
		   conversion_type != '[')
		{
			BOOL finished = FALSE;

			while(TRUE)
			{
				c = __getc(stream);
				if(c == EOF)
				{
					/* Hit the end of the stream. Due to an error? */
					if(num_conversions == 0 || ferror(stream))
						goto out;

					finished = TRUE;
					break;
				}
				else if (isspace(c))
				{
					total_num_chars_read++;
				}
				else
				{
					/* End of the white space; we push the last
					 * character read back into the stream.
					 */
					if(ungetc(c,stream) == EOF)
						goto out;

					/* Proceed with the conversion operation. */
					break;
				}
			}

			if(finished)
				break;
		}

		num_chars_processed = 0;

		D(("conversion type = '%lc'",c));

		if(conversion_type == 'c')
		{
			char * c_ptr;
			int i;

			/* The maximum field width is actually the number
			 * of characters that should be read. Default is
			 * 1 character.
			 */
			if(maximum_field_width < 0)
				maximum_field_width = 1;

			if(NOT assignment_suppressed)
			{
				assert( arg != NULL );

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(arg == NULL)
					{
						errno = EFAULT;
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */

				c_ptr = va_arg(arg,char *);

				assert( c_ptr != NULL );

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(c_ptr == NULL)
					{
						errno = EFAULT;
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */
			}
			else
			{
				c_ptr = NULL;
			}

			for(i = 0 ; i < maximum_field_width ; i++)
			{
				c = __getc(stream);
				if(c == EOF)
				{
					/* Bail out if we hit the end of the stream or an error occured. */
					if(num_conversions == 0 || ferror(stream))
						goto out;

					break;
				}

				total_num_chars_read++;

				if(NOT assignment_suppressed)
					(*c_ptr++) = c;

				num_chars_processed++;
			}

			/* Did we read as many characters as we wanted? */
			if(num_chars_processed != maximum_field_width)
				break;

			if(NOT assignment_suppressed)
				num_assignments++;

			num_conversions++;
		}
		else if (conversion_type == 'f')
		{
			#if defined(FLOATING_POINT_SUPPORT)
			{
				long double sum = 0.0;
				long double new_sum;
				BOOL is_negative = FALSE;
				BOOL decimal_point_matches = FALSE;
				BOOL have_exponent = FALSE;
				void * next_parameter = NULL;

				/* We boldly try to initialize the parameter to a well-
				   defined value before we begin the conversion. */
				if(NOT assignment_suppressed)
				{
					assert( arg != NULL );

					#if defined(CHECK_FOR_NULL_POINTERS)
					{
						if(arg == NULL)
						{
							errno = EFAULT;
							goto out;
						}
					}
					#endif /* CHECK_FOR_NULL_POINTERS */

					if(format_size == format_size_default)
					{
						SHOWMSG("short format (float)");

						next_parameter = va_arg(arg,float *);
					}
					else if (format_size == format_size_long)
					{
						SHOWMSG("long format (double)");

						next_parameter = va_arg(arg,double *);
					}
					else
					{
						SHOWMSG("extended format (long double)");

						next_parameter = va_arg(arg,long double *);
					}

					assert( next_parameter != NULL );

					#if defined(CHECK_FOR_NULL_POINTERS)
					{
						if(next_parameter == NULL)
						{
							errno = EFAULT;
							goto out;
						}
					}
					#endif /* CHECK_FOR_NULL_POINTERS */

					if(format_size == format_size_default)
					{
						*((float *)next_parameter) = 0;
					}
					else if (format_size == format_size_long)
					{
						*((double *)next_parameter) = 0;
					}
					else
					{
						*((long double *)next_parameter) = 0;
					}
				}

				if(maximum_field_width != 0)
				{
					c = __getc(stream);
					if(c != EOF)
					{
						/* Skip the sign. */
						if(c == '-')
						{
							SHOWMSG("negative floating point number");

							is_negative = 1;

							total_num_chars_read++;

							if(maximum_field_width > 0)
								maximum_field_width--;
						}
						else if (c == '+')
						{
							SHOWMSG("ignoring positive sign");

							total_num_chars_read++;

							if(maximum_field_width > 0)
								maximum_field_width--;
						}
						else
						{
							D(("first character '%lc' is not a sign",c));

							/* It's not a sign; reread this later. */
							if(ungetc(c,stream) == EOF)
							{
								SHOWMSG("couldn't push this character back");
								goto out;
							}
						}
					}
				}

				if(maximum_field_width != 0)
				{
					while(maximum_field_width != 0 && (c = __getc(stream)) != EOF)
					{
						/* Is this a digit? */
						if('0' <= c && c <= '9')
						{
							D(("got another digit '%lc'",c));

							new_sum = (10 * sum) + (c - '0');
							if(new_sum < sum) /* overflow? */
							{
								/* Put this back. */
								if(ungetc(c,stream) == EOF)
								{
									SHOWMSG("couldn't push this character back");
									goto out;
								}

								break;
							}

							sum = new_sum;
						}
						else
						{
							D(("'%lc' is not a digit",c));

							/* It's not a digit; reread this later. */
							if(ungetc(c,stream) == EOF)
							{
								SHOWMSG("couldn't push this character back");
								goto out;
							}

							break;
						}

						total_num_chars_read++;
						num_chars_processed++;

						if(maximum_field_width > 0)
							maximum_field_width--;
					}

					if(c == EOF && ((num_chars_processed == 0 && num_conversions == 0) || ferror(stream)))
						goto out;
				}

				if(maximum_field_width != 0)
				{
					SHOWMSG("looking for decimal point");

					c = __getc(stream);
					if(c != EOF)
					{
						/* Did we find the decimal point? We accept both the
						 * locale configured decimal point and the plain old
						 * dot.
						 */
						if(__locale_table[LC_NUMERIC] != NULL)
						{
							unsigned char * point;

							point = (unsigned char *)__locale_table[LC_NUMERIC]->loc_DecimalPoint;

							if(c == (*point) || c == '.')
							{
								SHOWMSG("found a decimal point");

								decimal_point_matches = TRUE;
							}
							else
							{
								D(("'%lc' is not a decimal point",c));
							}
						}
						else
						{
							if(c == '.')
							{
								SHOWMSG("found a decimal point");

								decimal_point_matches = TRUE;
							}
							else
							{
								D(("'%lc' is not a decimal point",c));
							}
						}

						if(decimal_point_matches)
						{
							total_num_chars_read++;

							if(maximum_field_width > 0)
								maximum_field_width--;
						}
						else if (c == 'e' || c == 'E')
						{
							SHOWMSG("found an exponent specifier");

							total_num_chars_read++;

							have_exponent = TRUE;

							if(maximum_field_width > 0)
								maximum_field_width--;
						}
						else
						{
							if(ungetc(c,stream) == EOF)
							{
								SHOWMSG("couldn't push this character back");
								goto out;
							}
						}
					}

					if(decimal_point_matches)
					{
						double multiplier = 0.1;

						SHOWMSG("found a decimal point");

						/* Process all digits following the decimal point. */
						while(maximum_field_width != 0 && (c = __getc(stream)) != EOF)
						{
							if('0' <= c && c <= '9')
							{
								if(multiplier != 0.0)
								{
									new_sum = sum + (c - '0') * multiplier;
									if(new_sum < sum) /* overflow? */
									{
										SHOWMSG("got an overflow");

										/* Put this back. */
										if(ungetc(c,stream) == EOF)
										{
											SHOWMSG("couldn't push this character back");
											goto out;
										}

										break;
									}

									sum = new_sum;

									multiplier = multiplier / 10.0;
								}

								total_num_chars_read++;

								if(maximum_field_width > 0)
									maximum_field_width--;
							}
							else if (c == 'e' || c == 'E')
							{
								SHOWMSG("found an exponent specifier");

								total_num_chars_read++;

								if(maximum_field_width > 0)
									maximum_field_width--;

								have_exponent = TRUE;
								break;
							}
							else
							{
								if(ungetc(c,stream) == EOF)
								{
									SHOWMSG("couldn't push this character back");
									goto out;
								}

								break;
							}
						}
					}

					if(have_exponent)
					{
						BOOL exponent_is_negative = FALSE;
						int new_exponent;
						int exponent = 0;

						SHOWMSG("processing exponent");

						if(maximum_field_width != 0)
						{
							c = __getc(stream);
							if(c != EOF)
							{
								/* Skip the sign. */
								if(c == '-')
								{
									exponent_is_negative = TRUE;

									total_num_chars_read++;

									if(maximum_field_width > 0)
										maximum_field_width--;
								}
								else if (c == '+')
								{
									total_num_chars_read++;

									if(maximum_field_width > 0)
										maximum_field_width--;
								}
								else
								{
									/* It's not a sign; reread this later. */
									if(ungetc(c,stream) == EOF)
									{
										SHOWMSG("couldn't push this character back");
										goto out;
									}
								}

								while(maximum_field_width != 0 && (c = __getc(stream)) != EOF)
								{
									if('0' <= c && c <= '9')
									{
										new_exponent = (10 * exponent) + (c - '0');
										if(new_exponent < exponent) /* overflow? */
										{
											if(ungetc(c,stream) == EOF)
											{
												SHOWMSG("couldn't push this character back");
												goto out;
											}

											break;
										}

										exponent = new_exponent;

										total_num_chars_read++;

										if(maximum_field_width > 0)
											maximum_field_width--;
									}
									else
									{
										if(ungetc(c,stream) == EOF)
										{
											SHOWMSG("couldn't push this character back");
											goto out;
										}

										break;
									}
								}

								/* If the exponent is valid, scale the number
								 * accordingly.
								 */
								if(exponent != 0)
								{
									if(exponent_is_negative)
									{
										double divisor;

										/* A negative exponent means division. */
										divisor = pow(10.0,(double)exponent);
										if(divisor != 0.0)
											sum = sum / divisor;
									}
									else
									{
										/* A positive exponent means multiplication. */
										new_sum = sum * pow(10.0,(double)exponent);
										if(new_sum >= sum)
											sum = new_sum;
										else
											sum = HUGE_VAL;
									}
								}
							}
						}
					}
				}

				SHOWMSG("conversion finished..");

				if(num_chars_processed > 0)
				{
					if(NOT assignment_suppressed)
					{
						if(is_negative)
							sum = (-sum);

						if(format_size == format_size_default)
						{
							*((float *)next_parameter) = sum;
						}
						else if (format_size == format_size_long)
						{
							*((double *)next_parameter) = sum;
						}
						else
						{
							*((long double *)next_parameter) = sum;
						}

						num_assignments++;
					}

					num_conversions++;
				}
			}
			#else
			{
				if(NOT assignment_suppressed)
				{
					void * next_parameter;

					assert( arg != NULL );

					#if defined(CHECK_FOR_NULL_POINTERS)
					{
						if(arg == NULL)
						{
							errno = EFAULT;
							goto out;
						}
					}
					#endif /* CHECK_FOR_NULL_POINTERS */

					if(format_size == format_size_default)
					{
						SHOWMSG("short format (float)");

						next_parameter = va_arg(arg,float *);
					}
					else if (format_size == format_size_long)
					{
						SHOWMSG("long format (double)");

						next_parameter = va_arg(arg,double *);
					}
					else
					{
						SHOWMSG("extended format (long double)");

						next_parameter = va_arg(arg,long double *);
					}

					assert( next_parameter != NULL );

					#if defined(CHECK_FOR_NULL_POINTERS)
					{
						if(next_parameter == NULL)
						{
							errno = EFAULT;
							goto out;
						}
					}
					#endif /* CHECK_FOR_NULL_POINTERS */
				}
			}
			#endif /* FLOATING_POINT_SUPPORT */
		}
		else if (conversion_type == 'd' ||
		         conversion_type == 'i' ||
		         conversion_type == 'o' ||
		         conversion_type == 'u' ||
		         conversion_type == 'x')
		{
			#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
			long long next_sum;
			long long sum = 0;
			#else
			int next_sum;
			int sum = 0;
			#endif /* __GNUC__ */
			BOOL is_negative = FALSE;
			int base;
			void * next_parameter = NULL;

			/* We boldly try to initialize the parameter to a well-
			   defined value before we begin the conversion. */
			if(NOT assignment_suppressed)
			{
				assert( arg != NULL );

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(arg == NULL)
					{
						errno = EFAULT;
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */

				if(format_size == format_size_short)
				{
					next_parameter = va_arg(arg,short *);
				}
				else
				{
					#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
					{
						if(format_size == format_size_long_long)
							next_parameter = va_arg(arg,long long *);
						else
							next_parameter = va_arg(arg,int *);
					}
					#else
					{
						next_parameter = va_arg(arg,int *);
					}
					#endif /* __GNUC__ */
				}

				assert( next_parameter != NULL );

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(next_parameter == NULL)
					{
						errno = EFAULT;
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */

				if(format_size == format_size_short)
				{
					*((short *)next_parameter) = 0;
				}
				else
				{
					#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
					{
						if(format_size == format_size_long_long)
							*((long long *)next_parameter) = 0;
						else
							*((int *)next_parameter) = 0;
					}
					#else
					{
						*((int *)next_parameter) = 0;
					}
					#endif /* __GNUC__ */
				}
			}

			if(conversion_type == 'd' || conversion_type == 'u')
				base = 10;
			else if (conversion_type == 'o')
				base = 8;
			else
				base = 0;

			if(maximum_field_width != 0)
			{
				c = __getc(stream);
				if(c != EOF)
				{
					/* Skip the sign. */
					if(c == '-')
					{
						is_negative = TRUE;

						total_num_chars_read++;

						if(maximum_field_width > 0)
							maximum_field_width--;
					}
					else if (c == '+')
					{
						total_num_chars_read++;

						if(maximum_field_width > 0)
							maximum_field_width--;
					}
					else if (c == '0')
					{
						/* Keep the leading zero. */
						total_num_chars_read++;
						num_chars_processed++;

						if(maximum_field_width > 0)
							maximum_field_width--;

						if(base == 0 && maximum_field_width != 0)
						{
							/* This could be an octal number, the
							 * '0x' prefix or just a zero.
							 */
							c = __getc(stream);
							if (c == 'x' || c == 'X')
							{
								/* It's the hex prefix. */
								base = 16;

								/* The preceding '0' was part of the
								   hex prefix. So we don't really know
								   the number yet. */
								num_chars_processed--;

								total_num_chars_read++;

								if(maximum_field_width > 0)
									maximum_field_width--;
							}
							else if (isdigit(c))
							{
								/* The preceding '0' was part of the
								   octal prefix. So we don't really know
								   the number yet. */
								num_chars_processed--;

								/* It's an octal number. */
								base = 8;

								/* Process the rest later. */
								if(ungetc(c,stream) == EOF)
									goto out;
							}
							else if (c != EOF)
							{
								/* It's something else; put it back. */
								if(ungetc(c,stream) == EOF)
									goto out;
							}
						}
					}
					else
					{
						if(ungetc(c,stream) == EOF)
							goto out;
					}
				}
			}

			/* Pick a base if none has been chosen yet. */
			if(base == 0)
			{
				if(conversion_type == 'x')
					base = 16;
				else
					base = 10;
			}

			if(maximum_field_width != 0)
			{
				while(maximum_field_width != 0 && (c = __getc(stream)) != EOF)
				{
					/* Is this a digit or hexadecimal value? */
					if(isxdigit(c))
					{
						int digit;

						/* Perform the necessary conversion. */
						if('0' <= c && c <= '9')
							digit = c - '0';
						else if ('a' <= c && c <= 'f')
							digit = c - 'a' + 10;
						else
							digit = c - 'A' + 10;

						/* Is this a valid digit? */
						if(digit > base)
						{
							if(ungetc(c,stream) == EOF)
								goto out;

							break;
						}

						next_sum = (base * sum) + digit;

						#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
						{
							if(format_size == format_size_long_long && (unsigned long long)next_sum < (unsigned long long)sum) /* overflow? */
							{
								/* Put this back. */
								if(ungetc(c,stream) == EOF)
									goto out;

								break;
							}
							else if ((unsigned int)next_sum < (unsigned int)sum) /* overflow? */
							{
								/* Put this back. */
								if(ungetc(c,stream) == EOF)
									goto out;

								break;
							}
						}
						#else
						{
							if((unsigned int)next_sum < (unsigned int)sum) /* overflow? */
							{
								/* Put this back. */
								if(ungetc(c,stream) == EOF)
									goto out;

								break;
							}
						}
						#endif /* __GNUC__ */

						total_num_chars_read++;
						num_chars_processed++;

						sum = next_sum;

						if(maximum_field_width > 0)
							maximum_field_width--;
					}
					else
					{
						/* It's something else; reread this later. */
						if(ungetc(c,stream) == EOF)
							goto out;

						break;
					}
				}

				if(c == EOF && ((num_chars_processed == 0 && num_conversions == 0) || ferror(stream)))
					goto out;
			}

			if(num_chars_processed > 0)
			{
				if(NOT assignment_suppressed)
				{
					if(is_negative)
						sum = (-sum);

					if(format_size == format_size_short)
					{
						*((short *)next_parameter) = sum;
					}
					else
					{
						#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
						{
							if(format_size == format_size_long_long)
								*((long long *)next_parameter) = sum;
							else
								*((int *)next_parameter) = sum;
						}
						#else
						{
							*((int *)next_parameter) = sum;
						}
						#endif /* __GNUC__ */
					}

					num_assignments++;
				}

				num_conversions++;
			}
		}
		else if (conversion_type == 's')
		{
			char * s_ptr;

			if(NOT assignment_suppressed)
			{
				assert( arg != NULL );

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(arg == NULL)
					{
						errno = EFAULT;
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */

				s_ptr = va_arg(arg,char *);

				assert( s_ptr != NULL );

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(s_ptr == NULL)
					{
						errno = EFAULT;
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */
			}
			else
			{
				s_ptr = NULL;
			}

			/* Try to NUL terminate this even in case of failure. */
			if(NOT assignment_suppressed)
				(*s_ptr) = '\0';

			if(maximum_field_width != 0)
			{
				while(maximum_field_width != 0 && (c = __getc(stream)) != EOF)
				{
					/* Blank spaces stop the conversion process. */
					if(isspace(c))
					{
						if(ungetc(c,stream) == EOF)
							goto out;

						break;
					}

					total_num_chars_read++;
					num_chars_processed++;

					if(NOT assignment_suppressed)
					{
						(*s_ptr++)	= c;
						(*s_ptr)	= '\0'; /* Try to NUL terminate this even in case of failure. */
					}

					if(maximum_field_width > 0)
						maximum_field_width--;
				}

				/* The conversion is considered to have failed if an EOF was
				   encountered before any non-whitespace characters could be
				   converted. We also bail out if we hit an error. */
				if(c == EOF && (num_chars_processed == 0 || ferror(stream)))
					goto out;
			}

			if(NOT assignment_suppressed)
				num_assignments++;

			num_conversions++;
		}
		else if (conversion_type == 'n')
		{
			if(NOT assignment_suppressed)
			{
				assert( arg != NULL );

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(arg == NULL)
					{
						errno = EFAULT;
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */

				if(format_size == format_size_short)
				{
					short * short_ptr;

					short_ptr = va_arg(arg,short *);

					assert( short_ptr != NULL );

					#if defined(CHECK_FOR_NULL_POINTERS)
					{
						if(short_ptr == NULL)
						{
							errno = EFAULT;
							goto out;
						}
					}
					#endif /* CHECK_FOR_NULL_POINTERS */

					(*short_ptr) = total_num_chars_read;
				}
				else
				{
					#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
					{
						if(format_size == format_size_long_long)
						{
							long long * int_ptr;

							int_ptr = va_arg(arg,long long *);

							assert( int_ptr != NULL );

							#if defined(CHECK_FOR_NULL_POINTERS)
							{
								if(int_ptr == NULL)
								{
									errno = EFAULT;
									goto out;
								}
							}
							#endif /* CHECK_FOR_NULL_POINTERS */

							(*int_ptr) = total_num_chars_read;
						}
						else
						{
							int * int_ptr;

							int_ptr = va_arg(arg,int *);

							assert( int_ptr != NULL );

							#if defined(CHECK_FOR_NULL_POINTERS)
							{
								if(int_ptr == NULL)
								{
									errno = EFAULT;
									goto out;
								}
							}
							#endif /* CHECK_FOR_NULL_POINTERS */

							(*int_ptr) = total_num_chars_read;
						}
					}
					#else
					{
						int * int_ptr;

						int_ptr = va_arg(arg,int *);

						assert( int_ptr != NULL );

						#if defined(CHECK_FOR_NULL_POINTERS)
						{
							if(int_ptr == NULL)
							{
								errno = EFAULT;
								goto out;
							}
						}
						#endif /* CHECK_FOR_NULL_POINTERS */

						(*int_ptr) = total_num_chars_read;
					}
					#endif /* __GNUC__ */
				}
			}
		}
		else if (c == '%')
		{
			int d;

			d = __getc(stream);
			if(d == EOF)
			{
				SHOWMSG("end of file");

				/* Hit the end of the stream. Due to an error? */
				if(num_conversions == 0 || ferror(stream))
					goto out;

				break;
			}
			else if (c == d)
			{
				total_num_chars_read++;
			}
			else
			{
				/* This is not what we expected. We're finished. */
				if(ungetc(d,stream) == EOF)
					goto out;

				break;
			}
		}
		else if (c == '[')
		{
			char * s_ptr;
			char set[256];
			int pick;

			if(NOT assignment_suppressed)
			{
				assert( arg != NULL );

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(arg == NULL)
					{
						errno = EFAULT;
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */

				s_ptr = va_arg(arg,char *);

				assert( s_ptr != NULL );

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(s_ptr == NULL)
					{
						errno = EFAULT;
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */
			}
			else
			{
				s_ptr = NULL;
			}

			c = (*format);
			if(c == '^')
			{
				/* Accept only characters which are not found in the range. */
				pick = 0;

				format++;
			}
			else
			{
				/* Accept only characters which are found in the range. */
				pick = 1;
			}

			memset(set,1 - pick,sizeof(set));

			/* If the ']' character introduces the range, then
			 * it is not considered the range termination character.
			 */
			c = (*format);
			if(c == ']')
			{
				set[c] = pick;

				format++;
			}

			/* Collect the other characters to form the range. */
			while((c = (*(unsigned char *)format)) != '\0')
			{
				format++;

				/* This would end the range. */
				if(c == ']')
					break;

				assert( 0 <= c && c <= 255 );

				set[c] = pick;
			}

			/* Try to NUL terminate this even in case of failure. */
			if(NOT assignment_suppressed)
				(*s_ptr) = '\0';

			if(maximum_field_width != 0)
			{
				while(maximum_field_width != 0 && (c = __getc(stream)) != EOF)
				{
					assert( 0 <= c && c <= 255 );

					/* If the character is not part of the
					 * range we stop further processing.
					 */
					if(set[c] == 0)
					{
						if(ungetc(c,stream) == EOF)
							goto out;

						break;
					}

					if(NOT assignment_suppressed)
					{
						(*s_ptr++)	= c;
						(*s_ptr)	= '\0'; /* Try to NUL terminate this even in case of failure. */
					}

					total_num_chars_read++;
					num_chars_processed++;

					if(maximum_field_width > 0)
						maximum_field_width--;
				}

				if(c == EOF && ((num_chars_processed == 0 && num_conversions == 0) || ferror(stream)))
					goto out;
			}

			if(num_chars_processed > 0)
			{
				if(NOT assignment_suppressed)
					num_assignments++;

				num_conversions++;
			}
		}
	}

	if(num_conversions > 0)
		result = num_assignments;

 out:

	RETURN(result);
	return(result); 
}
