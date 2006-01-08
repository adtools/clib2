/*
 * $Id: stdio_vfscanf.c,v 1.20 2006-01-08 12:04:25 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2006 by Olaf Barthel <olsen (at) sourcery.han.de>
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

/*#define DEBUG*/

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

#if defined(FLOATING_POINT_SUPPORT) && !defined(_MATH_HEADERS_H)
#include "math_headers.h"
#endif /* FLOATING_POINT_SUPPORT && !_MATH_HEADERS_H */

/****************************************************************************/

/*
 * Uncomment this to activate '%lld' support and the like. Better still,
 * define this is in the Makefile!
 */
/*#define USE_64_BIT_INTS*/

/****************************************************************************/

int
vfscanf(FILE *stream, const char *format, va_list arg)
{
	enum parameter_size_t
	{
		parameter_size_byte,
		parameter_size_long,
		parameter_size_short,
		parameter_size_size_t,
		parameter_size_ptrdiff_t,
		parameter_size_long_long,
		parameter_size_long_double,
		parameter_size_intmax_t,
		parameter_size_default
	};

	enum parameter_size_t parameter_size;
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

	if(__check_abort_enabled)
		__check_abort();

	flockfile(stream);

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(stream == NULL || format == NULL)
		{
			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	if(__fgetc_check(stream) < 0)
		goto out;

	/* Just so we can detect errors and tell them apart from
	   an 'end of file' condition. */
	clearerr(stream);

	while((c = (*(unsigned char *)format)) != '\0')
	{
		D(("next format character is '%lc'",c));

		if(isspace(c))
		{
			/* Skip all blank spaces in the stream. */
			format++;

			while((c = __getc(stream)) != EOF)
			{
				if(isspace(c))
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

			if(c == EOF)
			{
				SHOWMSG("end of file");

				/* Hit the end of the stream? */
				if(num_conversions == 0)
					goto out;

				/* Finished... */
				break;
			}

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

				/* Hit the end of the stream. */
				if(num_conversions == 0)
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
			parameter_size = parameter_size_short;

			SHOWMSG("short integer");

			format++;
		}
		else if (c == 'j')
		{
			parameter_size = parameter_size_intmax_t;

			SHOWMSG("largest integer");

			format++;
		}
		else if (c == 'l')
		{
			/* Parameter is a long integer or a double precision floating point value. */
			parameter_size = parameter_size_long;

			SHOWMSG("long integer");

			format++;
		}
		else if (c == 'L')
		{
			/* Parameter is a long double floating point value. */
			parameter_size = parameter_size_long_double;

			SHOWMSG("long double");

			format++;
		}
		else if (c == 't')
		{
			parameter_size = parameter_size_ptrdiff_t;

			SHOWMSG("pointer difference");

			format++;
		}
		else if (c == 'z')
		{
			parameter_size = parameter_size_size_t;

			SHOWMSG("size");

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
			parameter_size = parameter_size_default;

			SHOWMSG("default");
		}

		/* Now for the conversion type. */
		c = (*format);
		if(c == '\0')
			break;

		#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
		{
			D(("c = '%lc'",c));

			/* Check for long long parameters. */
			if(parameter_size == parameter_size_long && c == 'l')
			{
				SHOWMSG("this is a long long parameter");

				parameter_size = parameter_size_long_long;

				format++;

				/* The conversion type follows. */
				c = (*format);
				if(c == '\0')
					break;
			}
		}
		#endif /* __GNUC__ */

		/* Check for byte parameters. */
		if(parameter_size == parameter_size_short && c == 'h')
		{
			parameter_size = parameter_size_byte;

			SHOWMSG("byte-sized integer");

			format++;

			/* The conversion type follows. */
			c = (*format);
			if(c == '\0')
				break;
		}

		switch(c)
		{
			/* It's a pointer. */
			case 'p':

				conversion_type = 'x';
				format++;
				break;

			/* It's a floating point number. */
			case 'a':
			case 'A':
			case 'e':
			case 'E':
			case 'f':
			case 'g':
			case 'G':

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
			case 'X':	/* unsigned integer in hexadecimal format */
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
			while((c = __getc(stream)) != EOF)
			{
				if(isspace(c))
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
						__set_errno(EFAULT);
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
						__set_errno(EFAULT);
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
					/* Bail out if we hit the end of the stream. */
					if(num_conversions == 0)
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
				__long_double_t sum = 0.0;
				__long_double_t new_sum;
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
							__set_errno(EFAULT);
							goto out;
						}
					}
					#endif /* CHECK_FOR_NULL_POINTERS */

					if(parameter_size == parameter_size_default)
					{
						SHOWMSG("short format (float)");

						next_parameter = va_arg(arg,float *);
					}
					else if (parameter_size == parameter_size_long)
					{
						SHOWMSG("long format (double)");

						next_parameter = va_arg(arg,double *);
					}
					else
					{
						SHOWMSG("extended format (long double)");

						next_parameter = va_arg(arg,__long_double_t *);
					}

					assert( next_parameter != NULL );

					#if defined(CHECK_FOR_NULL_POINTERS)
					{
						if(next_parameter == NULL)
						{
							__set_errno(EFAULT);
							goto out;
						}
					}
					#endif /* CHECK_FOR_NULL_POINTERS */

					if(parameter_size == parameter_size_default)
					{
						*((float *)next_parameter) = 0;
					}
					else if (parameter_size == parameter_size_long)
					{
						*((double *)next_parameter) = 0;
					}
					else
					{
						*((__long_double_t *)next_parameter) = 0;
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

				/* Now it gets complicated. We need to pick up a keyword
				   such as "inf", "infinity" or "nan" if it's present.
				   This is tricky because we won't be able to push all
				   the characters back we read during scanning. */
				if(maximum_field_width != 0)
				{
					const char infinity_string[] = "infinity";
					const char infinity_nan[] = "nan";
					char chars_read_so_far[80];
					size_t num_chars_read_so_far = 0;
					size_t infinity_match = 0;
					size_t nan_match = 0;

					while(maximum_field_width != 0 && num_chars_read_so_far < sizeof(chars_read_so_far) && (c = __getc(stream)) != EOF)
					{
						D(("c = '%lc'",c));

						if(tolower(c) == infinity_string[infinity_match])
						{
							SHOWVALUE(infinity_match);

							nan_match = 0;

							chars_read_so_far[num_chars_read_so_far++] = c;
							if(maximum_field_width > 0)
								maximum_field_width--;

							/* Did we match the complete word? */
							infinity_match++;
							if(infinity_match == sizeof(infinity_string)-1)
							{
								SHOWMSG("we have a match for infinity");
								break;
							}
						}
						else if (infinity_match == 3) /* Did we match the "inf" of "infinity"? */
						{
							SHOWVALUE(infinity_match);

							nan_match = 0;

							if(ungetc(c,stream) == EOF)
							{
								SHOWMSG("couldn't push this character back");
								goto out;
							}

							SHOWMSG("we have a match for inf");
							break;
						}
						else if (tolower(c) == infinity_nan[nan_match])
						{
							SHOWVALUE(nan_match);

							infinity_match = 0;

							chars_read_so_far[num_chars_read_so_far++] = c;
							if(maximum_field_width > 0)
								maximum_field_width--;

							/* Did we match the complete word? */
							nan_match++;
							if(nan_match == sizeof(infinity_nan)-1)
							{
								SHOWMSG("we have a match for nan");

								/* Check for the () to follow the nan. */
								if(maximum_field_width != 0 && num_chars_read_so_far < sizeof(chars_read_so_far) && (c = __getc(stream)) != EOF)
								{
									/* Is this the opening parenthesis of the "nan()" keyword? */
									if(c == '(')
									{
										SHOWMSG("there's something following the nan");

										nan_match++;

										if(maximum_field_width > 0)
											maximum_field_width--;

										/* Look for the closing parenthesis. */
										while(maximum_field_width != 0 && (c = __getc(stream)) != EOF)
										{
											nan_match++;

											if(maximum_field_width > 0)
												maximum_field_width--;

											if(c == ')')
												break;
										}
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

								break;
							}
						}
						else
						{
							SHOWMSG("we didn't find a match for infinity/nan");

							nan_match = infinity_match = 0;

							if(ungetc(c,stream) == EOF)
							{
								SHOWMSG("couldn't push this character back");
								goto out;
							}

							maximum_field_width += num_chars_read_so_far;

							/* Let's try our best here... */
							while(num_chars_read_so_far > 0)
								ungetc(chars_read_so_far[--num_chars_read_so_far],stream);

							break;
						}
					}

					SHOWVALUE(infinity_match);

					if(infinity_match >= 3)
					{
						SHOWMSG("infinity");

						sum = __inf();

						total_num_chars_read = num_chars_processed = infinity_match;
					}
					else if (nan_match >= 3)
					{
						SHOWMSG("not a number");

						sum = nan(NULL);

						total_num_chars_read = num_chars_processed = nan_match;
					}
				}

				/* If we didn't find a keyword above, look for digits. */
				if(num_chars_processed == 0)
				{
					int radix = 10;

					/* Check if there's a hex prefix introducing this number. */
					if(maximum_field_width != 0 && (c = __getc(stream)) != EOF)
					{
						if(c == '0')
						{
							/* Use the leading zero as is. */
							total_num_chars_read++;
							num_chars_processed++;

							if(maximum_field_width > 0)
								maximum_field_width--;

							if(maximum_field_width != 0 && (c = __getc(stream)) != EOF)
							{
								if(tolower(c) == 'x')
								{
									SHOWMSG("found the 0x prefix; setting radix to 16");

									/* The floating point number will be encoded
									   in hexadecimal/binary notation. */
									radix = 16;

									total_num_chars_read++;
									num_chars_processed++;

									if(maximum_field_width > 0)
										maximum_field_width--;
								}
								else
								{
									/* Put this back. */
									if(ungetc(c,stream) == EOF)
									{
										SHOWMSG("couldn't push this character back");
										goto out;
									}
								}
							}
						}
						else
						{
							/* Put this back. */
							if(ungetc(c,stream) == EOF)
							{
								SHOWMSG("couldn't push this character back");
								goto out;
							}
						}
					}

					if(maximum_field_width != 0)
					{
						int digit;

						while(maximum_field_width != 0 && (c = __getc(stream)) != EOF)
						{
							/* Is this a digit? */
							if('0' <= c && c <= '9')
								digit = c - '0';
							else if ('a' <= c && c <= 'f')
								digit = c - 'a' + 10;
							else if ('A' <= c && c <= 'F')
								digit = c - 'A' + 10;
							else
								digit = radix;

							if(digit < radix)
							{
								D(("got another digit '%lc'",c));

								new_sum = (radix * sum) + digit;
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

						if(c == EOF && num_chars_processed == 0 && num_conversions == 0)
							goto out;
					}

					if(maximum_field_width != 0)
					{
						SHOWMSG("looking for decimal point");

						c = __getc(stream);
						if(c != EOF)
						{
							__locale_lock();

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

							__locale_unlock();

							if(decimal_point_matches)
							{
								total_num_chars_read++;

								if(maximum_field_width > 0)
									maximum_field_width--;
							}
							else if ((radix == 10 && (c == 'e' || c == 'E')) ||
							         (radix == 16 && (c == 'p' || c == 'P')))
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
							double multiplier = 1.0 / radix;
							int digit;

							SHOWMSG("found a decimal point");

							/* Process all digits following the decimal point. */
							while(maximum_field_width != 0 && (c = __getc(stream)) != EOF)
							{
								/* Is this a digit? */
								if('0' <= c && c <= '9')
									digit = c - '0';
								else if ('a' <= c && c <= 'f')
									digit = c - 'a' + 10;
								else if ('A' <= c && c <= 'F')
									digit = c - 'A' + 10;
								else
									digit = radix;

								if(digit < radix)
								{
									D(("got another digit '%lc'",c));

									if(multiplier != 0.0)
									{
										new_sum = sum + digit * multiplier;
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

										multiplier = multiplier / radix;
									}

									total_num_chars_read++;

									if(maximum_field_width > 0)
										maximum_field_width--;
								}
								else if ((radix == 10 && (c == 'e' || c == 'E')) ||
								         (radix == 16 && (c == 'p' || c == 'P')))
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
							int exponent_radix;

							SHOWMSG("processing exponent");

							/* The exponent may be a binary number. */
							if(radix == 16)
								exponent_radix = 2;
							else
								exponent_radix = 10;

							if(maximum_field_width != 0)
							{
								c = __getc(stream);
								if(c != EOF)
								{
									int digit;

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
											digit = c - '0';
										else
											digit = exponent_radix;

										if(digit < exponent_radix)
										{
											D(("got another digit '%lc'",c));

											new_exponent = (exponent_radix * exponent) + digit;
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
											divisor = pow((double)radix,(double)exponent);
											if(divisor != 0.0)
												sum = sum / divisor;
										}
										else
										{
											/* A positive exponent means multiplication. */
											new_sum = sum * pow((double)radix,(double)exponent);
											if(new_sum >= sum)
												sum = new_sum;
											else
												sum = __get_huge_val();
										}
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

						if(parameter_size == parameter_size_default)
						{
							*((float *)next_parameter) = sum;
						}
						else if (parameter_size == parameter_size_long)
						{
							*((double *)next_parameter) = sum;
						}
						else
						{
							*((__long_double_t *)next_parameter) = sum;
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
							__set_errno(EFAULT);
							goto out;
						}
					}
					#endif /* CHECK_FOR_NULL_POINTERS */

					if(parameter_size == parameter_size_default)
					{
						SHOWMSG("short format (float)");

						next_parameter = va_arg(arg,float *);
					}
					else if (parameter_size == parameter_size_long)
					{
						SHOWMSG("long format (double)");

						next_parameter = va_arg(arg,double *);
					}
					else
					{
						SHOWMSG("extended format (long double)");

						next_parameter = va_arg(arg,__long_double_t *);
					}

					assert( next_parameter != NULL );

					#if defined(CHECK_FOR_NULL_POINTERS)
					{
						if(next_parameter == NULL)
						{
							__set_errno(EFAULT);
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
			int radix;
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
						__set_errno(EFAULT);
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */

				if(parameter_size == parameter_size_short)
				{
					next_parameter = va_arg(arg,short *);
				}
				else if (parameter_size == parameter_size_byte)
				{
					next_parameter = va_arg(arg,char *);
				}
				else
				{
					#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
					{
						if(parameter_size == parameter_size_long_long || parameter_size == parameter_size_intmax_t)
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
						__set_errno(EFAULT);
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */

				if(parameter_size == parameter_size_short)
				{
					*((short *)next_parameter) = 0;
				}
				else if (parameter_size == parameter_size_byte)
				{
					*((char *)next_parameter) = 0;
				}
				else
				{
					#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
					{
						if(parameter_size == parameter_size_long_long || parameter_size == parameter_size_intmax_t)
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

			/* Chose a base according to the conversion to be
			   expected. For the 'x' and 'i' we examine the
			   incoming data rather than commit ourselves to
			   a peculiar data format now. */
			if(conversion_type == 'd' || conversion_type == 'u')
				radix = 10;
			else if (conversion_type == 'o')
				radix = 8;
			else
				radix = 0;

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

						if(radix == 0 && maximum_field_width != 0)
						{
							/* This could be an octal number, the
							 * '0x' prefix or just a zero.
							 */
							c = __getc(stream);

							/* This takes care of the '0x' prefix for hexadecimal
							   numbers ('%x') and also picks the right type of
							   data for the '%i' type. */
							if ((c == 'x' || c == 'X') && (conversion_type == 'x' || conversion_type == 'i'))
							{
								/* It's the hex prefix. */
								radix = 16;

								/* The preceding '0' was part of the
								   hex prefix. So we don't really know
								   the number yet. */
								num_chars_processed--;

								total_num_chars_read++;

								if(maximum_field_width > 0)
									maximum_field_width--;
							}
							else if (isdigit(c) && (conversion_type == 'i')) /* This could be the octal prefix for the '%i' format. */
							{
								/* The preceding '0' was part of the
								   octal prefix. So we don't really know
								   the number yet. */
								num_chars_processed--;

								/* It's an octal number. */
								radix = 8;

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
			if(radix == 0)
			{
				if(conversion_type == 'x')
					radix = 16;
				else
					radix = 10;
			}

			if(maximum_field_width != 0)
			{
				int digit;

				while(maximum_field_width != 0 && (c = __getc(stream)) != EOF)
				{
					/* Is this a digit or hexadecimal value? */
					if('0' <= c && c <= '9')
						digit = c - '0';
					else if ('a' <= c && c <= 'f')
						digit = c - 'a' + 10;
					else if ('A' <= c && c <= 'F')
						digit = c - 'A' + 10;
					else
						digit = radix;

					/* Is this a valid digit? */
					if(digit >= radix)
					{
						SHOWMSG("digit is larger than radix");

						if(ungetc(c,stream) == EOF)
							goto out;

						break;
					}

					next_sum = (radix * sum) + digit;

					#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
					{
						if(parameter_size == parameter_size_long_long || parameter_size == parameter_size_intmax_t)
						{
							if((unsigned long long)next_sum < (unsigned long long)sum) /* overflow? */
							{
								SHOWMSG("overflow!");

								/* Put this back. */
								if(ungetc(c,stream) == EOF)
									goto out;

								break;
							}
						}
						else if ((unsigned int)next_sum < (unsigned int)sum) /* overflow? */
						{
							SHOWMSG("overflow!");

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
							SHOWMSG("overflow!");

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

				if(c == EOF && num_chars_processed == 0 && num_conversions == 0)
					goto out;
			}

			if(num_chars_processed > 0)
			{
				if(NOT assignment_suppressed)
				{
					if(is_negative)
						sum = (-sum);

					if(parameter_size == parameter_size_short)
					{
						*((short *)next_parameter) = sum;
					}
					else if (parameter_size == parameter_size_byte)
					{
						*((char *)next_parameter) = sum;
					}
					else
					{
						#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
						{
							if(parameter_size == parameter_size_long_long || parameter_size == parameter_size_intmax_t)
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
						__set_errno(EFAULT);
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
						__set_errno(EFAULT);
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
				   converted. */
				if(c == EOF && num_chars_processed == 0 && num_conversions == 0)
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
						__set_errno(EFAULT);
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */

				if(parameter_size == parameter_size_short)
				{
					short * short_ptr;

					short_ptr = va_arg(arg,short *);

					assert( short_ptr != NULL );

					#if defined(CHECK_FOR_NULL_POINTERS)
					{
						if(short_ptr == NULL)
						{
							__set_errno(EFAULT);
							goto out;
						}
					}
					#endif /* CHECK_FOR_NULL_POINTERS */

					(*short_ptr) = total_num_chars_read;
				}
				else if (parameter_size == parameter_size_byte)
				{
					char * byte_ptr;

					byte_ptr = va_arg(arg,char *);

					assert( byte_ptr != NULL );

					#if defined(CHECK_FOR_NULL_POINTERS)
					{
						if(byte_ptr == NULL)
						{
							__set_errno(EFAULT);
							goto out;
						}
					}
					#endif /* CHECK_FOR_NULL_POINTERS */

					(*byte_ptr) = total_num_chars_read;
				}
				else
				{
					#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
					{
						if(parameter_size == parameter_size_long_long || parameter_size == parameter_size_intmax_t)
						{
							long long * int_ptr;

							int_ptr = va_arg(arg,long long *);

							assert( int_ptr != NULL );

							#if defined(CHECK_FOR_NULL_POINTERS)
							{
								if(int_ptr == NULL)
								{
									__set_errno(EFAULT);
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
									__set_errno(EFAULT);
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
								__set_errno(EFAULT);
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

				/* Hit the end of the stream. */
				if(num_conversions == 0)
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
			const unsigned char * scanset;
			size_t scanset_length,i;
			int pick;

			if(NOT assignment_suppressed)
			{
				assert( arg != NULL );

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(arg == NULL)
					{
						__set_errno(EFAULT);
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
						__set_errno(EFAULT);
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

			/* Figure out how many characters are in the scanset. */
			scanset = (const unsigned char *)format;

			for(scanset_length = 0 ; scanset[scanset_length] != '\0' && scanset[scanset_length] != ']' ; scanset_length++)
				format++;

			/* We already skipped everything but the right bracket. */
			if((*format) == ']')
				format++;

			/* Now have a look at the specification. We support a non-standard
			   scanf() family feature which permits you to specify ranges of
			   characters rather than spelling out each character included in
			   the range. */
			for(i = 0 ; i < scanset_length ; i++)
			{
				c = scanset[i];

				/* Could this be a range? It's not a range if it
				   is the first or the last character in the
				   specification. */
				if(c == '-' && 0 < i && i < scanset_length - 1)
				{
					int first,last,j;

					/* Pick the first and the last character in
					   the range, e.g. for "%[A-Z]" the first would
					   be the 'A' and the 'Z' would be the last. */
					first	= scanset[i-1];
					last	= scanset[i+1];

					/* Everything in the scanset now 
					   goes into the set. */
					for(j = first ; j <= last ; j++)
						set[j] = pick;

					/* Skip the character which marked the
					   end of the range and resume scanning. */
					i++;

					continue;
				}

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

				if(c == EOF && num_chars_processed == 0 && num_conversions == 0)
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

	result = num_assignments;

 out:

	funlockfile(stream);

	RETURN(result);
	return(result); 
}
