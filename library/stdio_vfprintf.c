/*
 * $Id: stdio_vfprintf.c,v 1.25 2006-11-13 09:32:28 obarthel Exp $
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
#ifndef _STDLIB_NULL_POINTER_CHECK_H
#include "stdlib_null_pointer_check.h"
#endif /* _STDLIB_NULL_POINTER_CHECK_H */

/****************************************************************************/

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

/*
 * Uncomment this to activate '%lld' support and the like. Better still,
 * define this is in the Makefile!
 */
/*#define USE_64_BIT_INTS*/

/****************************************************************************/

/* Data conversion flags for vfprintf() below. */
#define FORMATF_LeftJustified		(1<<0)	/* Output must be left justified */
#define FORMATF_ProduceSign			(1<<1)	/* Numbers always begin with a leading
											   sign character */
#define FORMATF_ProduceSpace		(1<<2)	/* Numbers always begin with a '-'
											   character or a blank space */
#define FORMATF_AlternateConversion	(1<<3)	/* Use alternative conversion format */
#define FORMATF_CapitalLetters		(1<<4)	/* Output must use upper case characters */
#define FORMATF_IsNegative			(1<<5)	/* Number is negative */
#define FORMATF_HexPrefix			(1<<6)	/* Prepend '0x' to the output */
#define FORMATF_ZeroPrefix			(1<<7)	/* Prepend '0' to the output */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

#define max(a,b) ((a) > (b) ? (a) : (b))

/****************************************************************************/

STATIC int
get_num_leading_digits(__long_double_t v,int radix)
{
	int num_digits;

	SHOWVALUE(radix);

	if(v < radix)
	{
		num_digits = 1;
	}
	else
	{
		/* For some reason log() can crash on GCC 68k... */
		#if (!defined(__GNUC__) || defined(__PPC__))
		{
			num_digits = 1 + floor(log(v) / log((double)radix));
		}
		#else
		{
			/* Perform the conversion one digit at a time... */
			num_digits = 0;

			while(floor(v) > 0.0)
			{
				num_digits++;

				v /= radix;
			}
		}
		#endif /* __GNUC__ && !__PPC__ */
	}

	return(num_digits);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */

/****************************************************************************/

int
vfprintf(FILE * stream,const char * format, va_list arg)
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

	struct iob * iob = (struct iob *)stream;
	int format_flags;
	char fill_character;
	int minimum_field_width;
	int precision;
	enum parameter_size_t parameter_size;
	char conversion_type;
	char buffer[80];
	int buffer_mode;
	char *output_buffer;
	int output_len;
	const char *prefix;
	char prefix_buffer[8];
	int result = EOF;
	int len = 0;
	int c;

 #if defined(FLOATING_POINT_SUPPORT)
 	char * internal_buffer = NULL;
 	size_t internal_buffer_size = 0;
	char trail_string[8];
	int trail_string_len;
	int num_trailing_zeroes;
 #endif /* FLOATING_POINT_SUPPORT */

	ENTER();

	SHOWSTRING(format);

	assert( stream != NULL && format != NULL && arg != NULL );

	if(__check_abort_enabled)
		__check_abort();

	flockfile(stream);

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(stream == NULL || format == NULL)
		{
			SHOWMSG("invalid parameters");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	/* If no buffering is specified but a buffer was allocated, switch to
	   line buffering. This is intended to help 'stderr' and others. */
	buffer_mode = (iob->iob_Flags & IOBF_BUFFER_MODE);
	if(buffer_mode == IOBF_BUFFER_MODE_NONE)
		buffer_mode = IOBF_BUFFER_MODE_LINE;

	assert( FLAG_IS_SET(iob->iob_Flags,IOBF_IN_USE) );
	assert( iob->iob_BufferSize > 0 );

	if(__fputc_check(stream) < 0)
		goto out;

	while((c = (*format++)) != '\0')
	{
		/* I this isn't a % charater, copy the input to the output. */
		if(c != '%')
		{
			if(__putc(c,stream,buffer_mode) == EOF)
				goto out;

			len++;

			continue;
		}

		format_flags	= 0;
		fill_character	= ' ';

		/* Collect the flags: left justification, sign, space,
		 * alternate format, fill character.
		 */
		while(TRUE)
		{
			c = (*format);

			if(c == '-')
			{
				SHOWMSG("minus");

				SET_FLAG(format_flags,FORMATF_LeftJustified);
				format++;
			}
			else if (c == '+')
			{
				SHOWMSG("plus");

				SET_FLAG(format_flags,FORMATF_ProduceSign);
				format++;
			}
			else if (c == ' ')
			{
				SHOWMSG("space");

				SET_FLAG(format_flags,FORMATF_ProduceSpace);
				format++;
			}
			else if (c == '#')
			{
				SHOWMSG("alternate");

				SET_FLAG(format_flags,FORMATF_AlternateConversion);
				format++;
			}
			else if (c == '0')
			{
				SHOWMSG("leading zeroes");

				fill_character = '0';
				format++;
			}
			else
			{
				break;
			}
		}

		if(c == '\0')
			break;

		/* Now for the field width. */
		minimum_field_width = 0;

		while(TRUE)
		{
			c = (*format);

			if(c == '*')
			{
				SHOWMSG("use field width (stack)");

				/* The field width is stored on the stack. */

				assert(arg != NULL);

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(arg == NULL)
					{
						__set_errno(EFAULT);
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */

				minimum_field_width = va_arg(arg,int);
				if(minimum_field_width < 0)
					minimum_field_width = 0;

				format++;
			}
			else if ('0' <= c && c <= '9')
			{
				int next_sum;
				int sum = 0;

				SHOWMSG("use field width (string)");

				/* Process the field width. */
				while(TRUE)
				{
					c = (*format);

					if('0' <= c && c <= '9')
					{
						D(("digit = %lc",c));

						next_sum = (10 * sum) + c - '0';
						if(next_sum < sum) /* overflow? */
						{
							SHOWMSG("overflow");
							break;
						}

						SHOWVALUE(sum);

						sum = next_sum;
						format++;
					}
					else
					{
						break;
					}
				}

				minimum_field_width = sum;

				SHOWVALUE(minimum_field_width);
			}
			else
			{
				break;
			}
		}

		/* End of the format string? */
		if(c == '\0')
			break;

		precision = -1;

		/* Was a precision specified? */
		if((*format) == '.')
		{
			SHOWMSG("precision required");

			format++;

			c = (*format);
			if(c == '*')
			{
				SHOWMSG("use stack");

				/* The precision is stored on the stack. */
				assert(arg != NULL);

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(arg == NULL)
					{
						__set_errno(EFAULT);
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */

				precision = va_arg(arg,int);
				if(precision < 0)
					precision = 0;

				format++;
			}
			else if ('0' <= c && c <= '9')
			{
				int next_sum;
				int sum = 0;

				SHOWMSG("use string");

				/* Process the precision. */
				while(TRUE)
				{
					c = (*format);

					if('0' <= c && c <= '9')
					{
						D(("digit = %lc",c));

						next_sum = (10 * sum) + c - '0';
						if(next_sum < sum) /* overflow? */
						{
							SHOWMSG("overflow");
							break;
						}

						SHOWVALUE(sum);

						sum = next_sum;
						format++;
					}
					else
					{
						break;
					}
				}

				precision = sum;
			}
			else
			{
				/* Anything else results in the precision
				 * value to be set to 0.
				 */
				precision = 0;
			}

			SHOWVALUE(precision);
		}

		/* Now for the size modifier, if any. */
		c = (*format);

		/* End of the format string? */
		if(c == '\0')
			break;

		if(c == 'l')
		{
			SHOWMSG("format size = long");

			parameter_size = parameter_size_long;
			format++;
		}
		else if (c == 'L')
		{
			SHOWMSG("format size = long double");

			parameter_size = parameter_size_long_double;
			format++;
		}
		else if (c == 'h')
		{
			SHOWMSG("format size = short");

			parameter_size = parameter_size_short;
			format++;
		}
		else if (c == 'j')
		{
			SHOWMSG("format size = intmax_t");

			parameter_size = parameter_size_intmax_t;
			format++;
		}
		else if (c == 't')
		{
			SHOWMSG("format size = ptrdiff_t");

			parameter_size = parameter_size_ptrdiff_t;
			format++;
		}
		else if (c == 'z')
		{
			SHOWMSG("format size = size_t");

			parameter_size = parameter_size_size_t;
			format++;
		}
		else
		{
			SHOWMSG("format size = default");

			/* The default is to assume 32 bit parameters. */
			parameter_size = parameter_size_default;
		}

		/* Finally, the conversion_type type. */
		c = (*format);

		/* End of the format string? */
		if(c == '\0')
			break;

		/* Check for byte parameters. */
		if(parameter_size == parameter_size_short && c == 'h')
		{
			SHOWMSG("format size = byte");

			parameter_size = parameter_size_byte;

			format++;

			/* The conversion_type type follows. */
			c = (*format);

			/* End of the format string? */
			if(c == '\0')
				break;
		}

		#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
		{
			/* Check for long long parameters. */
			if(parameter_size == parameter_size_long && c == 'l')
			{
				parameter_size = parameter_size_long_long;

				format++;

				/* The conversion_type type follows. */
				c = (*format);

				/* End of the format string? */
				if(c == '\0')
					break;
			}
		}
		#endif /* __GNUC__ */

		#if defined(FLOATING_POINT_SUPPORT)
		{
			trail_string[0] = '\0';
			num_trailing_zeroes = 0;
		}
		#endif /* FLOATING_POINT_SUPPORT */

		D(("conversion_type (preliminary) = %lc",c));

		switch(c)
		{
			/* signed integer */
			case 'i':

				conversion_type = 'd';

				format++;
				break;

			/* unsigned integer (hexadecimal notation) */
			case 'X':

				SET_FLAG(format_flags,FORMATF_CapitalLetters);

				conversion_type = 'x';

				format++;
				break;

			/* floating point number (exponential notation) */
			case 'E':

				SET_FLAG(format_flags,FORMATF_CapitalLetters);

				conversion_type = 'e';

				format++;
				break;

			/* floating point number (hexadecimal digits; exponential notation) */
			case 'A':

				SET_FLAG(format_flags,FORMATF_CapitalLetters);

				conversion_type = 'a';

				format++;
				break;

			/* floating point number (plain or exponential notation) */
			case 'G':

				SET_FLAG(format_flags,FORMATF_CapitalLetters);

				conversion_type = 'g';

				format++;
				break;

			/* pointer (hexadecimal notation, eight digits, '0x' prefix) */
			case 'p':

				conversion_type = 'x';

				SET_FLAG(format_flags,FORMATF_HexPrefix);

				fill_character = '0';
				minimum_field_width = 8;

				format++;
				break;

			case 'a':	/* floating point number (hexadecimal digits; exponential notation) */
			case 'c':	/* character */
			case 'd':	/* signed integer */
			case 'f':	/* floating point number */
			case 'e':	/* floating point number (exponential notation) */
			case 'g':	/* floating point number (plain or exponential notation) */
			case 's':	/* string */
			case '%':	/* % character */
			case 'o':	/* unsigned integer (octal notation) */
			case 'x':	/* unsigned integer (hexadecimal notation) */
			case 'u':	/* unsigned integer */
			case 'n':	/* number of characters written */
			default:	/* anything else (% works as escape character) */

				conversion_type = c;
				format++;
				break;
		}

		D(("conversion_type (final) = %lc",conversion_type));

		output_buffer = &buffer[sizeof(buffer)-1];
		(*output_buffer) = '\0';

		output_len = 0;

		if(conversion_type == 'c')
		{
			int ch;

			SHOWMSG("character");

			assert(arg != NULL);

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
				/* Parameter is a short integer which
				 * must be cast to a long integer before
				 * it can be used.
				 */
				short short_integer;

				short_integer = (short)va_arg(arg, int);

				ch = short_integer;
			}
			else if (parameter_size == parameter_size_byte)
			{
				/* Parameter is a byte-sized integer which
				 * must be cast to a long integer before
				 * it can be used.
				 */
				char byte_integer;

				byte_integer = (char)va_arg(arg, int);

				ch = byte_integer;
			}
			else
			{
				#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
				{
					if(parameter_size == parameter_size_long_long || parameter_size == parameter_size_intmax_t)
						ch = va_arg(arg, long long);
					else
						ch = va_arg(arg, int);
				}
				#else
				{
					ch = va_arg(arg, int);
				}
				#endif /* __GNUC__ */
			}

			D(("output = %lc",ch));

			output_buffer--;
			output_len++;

			(*output_buffer) = ch;

			CLEAR_FLAG(format_flags,FORMATF_ProduceSign);
			CLEAR_FLAG(format_flags,FORMATF_ProduceSpace);
		}
		else if (conversion_type == 'a' ||
		         conversion_type == 'e' ||
		         conversion_type == 'f' ||
		         conversion_type == 'g')
		{
			SHOWMSG("floating point format");

			assert(arg != NULL);

			#if defined(CHECK_FOR_NULL_POINTERS)
			{
				if(arg == NULL)
				{
					__set_errno(EFAULT);
					goto out;
				}
			}
			#endif /* CHECK_FOR_NULL_POINTERS */

			#if defined(FLOATING_POINT_SUPPORT)
			{
				const char *	buffer_stop		= &buffer[sizeof(buffer)-1];
				char *			buffer_start	= buffer;

				const char * digit_encoding;
				__long_double_t v;
				int radix;

				if(conversion_type == 'a')
				{
					SET_FLAG(format_flags,FORMATF_HexPrefix);

					radix = 16;
				}
				else
				{
					radix = 10;
				}

				if(FLAG_IS_SET(format_flags,FORMATF_CapitalLetters))
					digit_encoding = "0123456789ABCDEF";
				else
					digit_encoding = "0123456789abcdef";

				output_buffer = buffer_start;

				D(("sizeof(long double) == %ld",sizeof(v)));

				if(parameter_size == parameter_size_long_double)
					v = va_arg(arg, __long_double_t);
				else
					v = va_arg(arg, double);

				if(isinf(v))
				{
					SHOWMSG("infinity");

					strcpy(output_buffer,"inf");
					output_len = 3;

					if(v < 0)
						SET_FLAG(format_flags,FORMATF_IsNegative);

					fill_character = ' ';
				}
				else if (isnan(v))
				{
					SHOWMSG("not a number");

					strcpy(output_buffer,"nan");
					output_len = 3;

					if(FLAG_IS_SET(format_flags,FORMATF_ProduceSign))
					{
						SET_FLAG(format_flags,FORMATF_ProduceSpace);

						CLEAR_FLAG(format_flags,FORMATF_ProduceSign);
					}

					fill_character = ' ';
				}
				else
				{
					BOOL strip_trailing_zeroes = FALSE;
					__long_double_t roundoff_fudge = 0.0;
					int num_output_characters;
					int num_leading_digits;
					int max_digits = -1;
					int exponent = 0;
					int digit;
					int i;

					/* This takes care of the sign. */
					if(v < 0.0)
					{
						SHOWMSG("negative number");

						SET_FLAG(format_flags,FORMATF_IsNegative);
						v = (-v);
					}

					D(("sizeof(v) == %ld",sizeof(v)));

					#if DEBUG
					{
						unsigned long n[2];

						memcpy(n,&v,sizeof(n));

						D(("v = 0x%08lx%08lx",n[0],n[1]));
					}
					#endif /* DEBUG */

					/* Default precision is 6 digits. */
					if(precision < 0)
					{
						SHOWMSG("no precision specified, using six digits");
						precision = 6;
					}

					/* Figure out whether 'e' or 'f' format should be used. */
					if(conversion_type == 'g' || conversion_type == 'e' || conversion_type == 'a')
					{
						__long_double_t local_v = v;
						int local_exponent = 0;

						/* Put one single digit in front of the decimal point. */
						while(local_v != 0.0 && local_v < 1.0)
						{
							local_v *= radix;
							local_exponent--;
						}

						while(local_v >= radix)
						{
							local_v /= radix;
							local_exponent++;
						}

						if(conversion_type == 'g')
						{
							/* If the precision is < 1, then it defaults to 1. */
							if(precision < 1)
								precision = 1;

							SHOWVALUE(exponent);
							SHOWVALUE(precision);

							/* If the exponent is < -4 or greater than or equal to
							 * the precision, we switch to 'e' or 'f' format,
							 * respectively.
							 */
							if((local_exponent < -4) || local_exponent >= precision)
								conversion_type = 'e';
							else
								conversion_type = 'f';

							max_digits = precision;

							strip_trailing_zeroes = TRUE;
						}

						if(conversion_type == 'e' || conversion_type == 'a')
						{
							v			= local_v;
							exponent	= local_exponent;
						}
					}

					D(("conversion_type is now %lc",conversion_type));

					/* If necessary, perform rounding after the
					   last digit to be displayed. */
					if(max_digits > 0)
					{
						int roundoff_position;

						if(v >= 1.0)
							roundoff_position = max_digits - get_num_leading_digits(v,radix);
						else
							roundoff_position = max_digits;

						if(roundoff_position >= 0)
							roundoff_fudge = pow((double)radix,(double)(roundoff_position + 1));
					}
					else
					{
						roundoff_fudge = pow((double)radix,(double)(precision + 1));
					}

					if(roundoff_fudge > 0.0)
						v += 5.0 / roundoff_fudge;

					/* The rounding may have caused an overflow, putting
					   two digits in front of the decimal point. This
					   needs to be corrected. */
					if(conversion_type == 'e' || conversion_type == 'a')
					{
						while(v >= radix)
						{
							v /= radix;
							exponent++;
						}
					}

					SHOWMSG("integral part");

					num_leading_digits = get_num_leading_digits(v,radix);

					SHOWVALUE(num_leading_digits);

					/* Figure out how much room the number will need in order
					   to be stored. */
					num_output_characters =
						1 +									/* sign */
						num_leading_digits +				/* integral part */
						1 +									/* decimal point */
						max(0,max(precision,max_digits)) +	/* fractional part */
						1 +									/* 'e' or 'p' */
						1 +									/* sign of the exponent */
						32 +								/* exponent */
						1;									/* NUL termination */

					/* Can we store that much? */
					if((size_t)num_output_characters > sizeof(buffer))
					{
						if((size_t)num_output_characters > internal_buffer_size)
						{
							char * new_internal_buffer;

							/* Try to (re-)allocate a larger output buffer. */
							new_internal_buffer = realloc(internal_buffer,(size_t)num_output_characters);
							if(new_internal_buffer == NULL)
							{
								__set_errno(ENOMEM);
								goto out;
							}

							internal_buffer			= new_internal_buffer;
							internal_buffer_size	= (size_t)num_output_characters;
						}

						buffer_start	= internal_buffer;
						buffer_stop		= &internal_buffer[internal_buffer_size - 1];
						output_buffer   = buffer_start;
					}

					if(v >= 1.0)
					{
						/* 'Normalize' the number so that we have a zero in
						   front of the mantissa. We can't lose here: we
						   simply scale the value without any loss of
						   precision (we just change the floating point
						   exponent). */
						v /= pow((double)radix,(double)num_leading_digits);

						for(i = 0 ; (max_digits != 0) && (i < num_leading_digits) && (output_buffer < buffer_stop) ; i++)
						{
							v *= radix;

							digit = floor(v);

							D(("next digit = %lc (digit = %ld)",digit_encoding[digit],digit));

							assert( 0 <= digit && digit < radix );

							(*output_buffer++) = digit_encoding[digit];

							v -= digit;

							if(max_digits > 0)
								max_digits--;
						}
					}
					else
					{
						/* NOTE: any 'significant' digits (for %g conversion)
						         will follow the decimal point. */
						(*output_buffer++) = '0';
					}

					/* Now for the fractional part. */
					if(precision > 0)
					{
						SHOWMSG("mantissa");

						if((max_digits != 0) && (output_buffer < buffer_stop))
						{
							(*output_buffer++) = '.';

							for(i = 0 ; (max_digits != 0) && (i < precision) && (output_buffer < buffer_stop) ; i++)
							{
								v *= radix;

								digit = floor(v);

								D(("next digit = %lc",digit_encoding[digit]));

								assert( 0 <= digit && digit < radix );

								(*output_buffer++) = digit_encoding[digit];

								v -= digit;

								if(max_digits > 0)
									max_digits--;
							}

							if(i < precision && max_digits != 0 && NOT strip_trailing_zeroes)
							{
								num_trailing_zeroes = precision - i;

								if(max_digits > 0 && max_digits < num_trailing_zeroes)
									num_trailing_zeroes = max_digits;
							}

							/* Strip trailing digits and decimal point
							 * unless we shouldn't.
							 */
							if(strip_trailing_zeroes && FLAG_IS_CLEAR(format_flags,FORMATF_AlternateConversion))
							{
								SHOWMSG("strip trailing zeroes and comma");

								while(output_buffer > buffer_start+1 && output_buffer[-1] == '0')
									output_buffer--;

								if(output_buffer > buffer_start && output_buffer[-1] == '.')
									output_buffer--;
							}
						}
					}
					else
					{
						/* Precision is zero; if the alternative conversion flag
						 * is specified, add the lonely decimal point.
						 */
						if(FLAG_IS_SET(format_flags,FORMATF_AlternateConversion))
						{
							if(output_buffer < buffer_stop)
								(*output_buffer++) = '.';
						}
					}

					#if DEBUG
					{
						(*output_buffer) = '\0';

						output_len = output_buffer - buffer_start;

						D(("length = %ld, output_buffer = '%s'",output_len,buffer_start));
					}
					#endif /* DEBUG */

					if(conversion_type == 'e' || conversion_type == 'a')
					{
						char exponent_string[40];
						size_t exponent_string_len,j;
						int exponent_sign;

						/* For the '%a' conversion the exponent is given in
						   binary notation rather than decimal. */
						if(conversion_type == 'a')
							radix = 2;

						/* Build the exponent string in reverse order. */
						exponent_string_len = 0;

						if(exponent < 0)
						{
							exponent_sign = -1;

							exponent = (-exponent);
						}
						else
						{
							exponent_sign = 1;
						}

						while(exponent > 0 && exponent_string_len < sizeof(exponent_string))
						{
							exponent_string[exponent_string_len++] = '0' + (exponent % radix);

							exponent /= radix;
						}

						/* Minimum length of the exponent is two digits. */
						while(exponent_string_len < 2)
							exponent_string[exponent_string_len++] = '0';

						if(exponent_string_len < sizeof(exponent_string)-1)
						{
							if(exponent_sign < 0)
								exponent_string[exponent_string_len++] = '-';
							else
								exponent_string[exponent_string_len++] = '+';

							if(conversion_type == 'a')
							{
								if(FLAG_IS_SET(format_flags,FORMATF_CapitalLetters))
									exponent_string[exponent_string_len++] = 'P';
								else
									exponent_string[exponent_string_len++] = 'p';
							}
							else
							{
								if(FLAG_IS_SET(format_flags,FORMATF_CapitalLetters))
									exponent_string[exponent_string_len++] = 'E';
								else
									exponent_string[exponent_string_len++] = 'e';
							}
						}

						/* Add the exponent string in reverse order. */
						for(j = 0 ; exponent_string_len > 0 ; j++)
							trail_string[j] = exponent_string[--exponent_string_len];

						trail_string[j] = '\0';
					}

					(*output_buffer) = '\0';

					output_len = output_buffer - buffer_start;
					output_buffer = buffer_start;

					D(("length = %ld, output_buffer = '%s'",output_len,output_buffer));
				}
			}
			#else
			{
				/* Remove the parameter from the argument vector and
				   don't produce any output. */
				if(parameter_size == parameter_size_long_double)
					(void)va_arg(arg, __long_double_t);
				else
					(void)va_arg(arg, double);

				minimum_field_width = 0;
				format_flags = 0;
			}
			#endif /* FLOATING_POINT_SUPPORT */
		}
		else if (conversion_type == 'd' ||
		         conversion_type == 'o' ||
		         conversion_type == 'u' ||
		         conversion_type == 'x')
		{
			#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
			unsigned long long v;
			#else
			unsigned int v;
			#endif /* __GNUC__ */

			assert(arg != NULL);

			#if defined(CHECK_FOR_NULL_POINTERS)
			{
				if(arg == NULL)
				{
					__set_errno(EFAULT);
					goto out;
				}
			}
			#endif /* CHECK_FOR_NULL_POINTERS */

			if(conversion_type == 'd')
			{
				#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
				long long sv;
				#else
				int sv;
				#endif /* __GNUC__ */

				SHOWMSG("signed integer");

				if(parameter_size == parameter_size_short)
				{
					/* Parameter is a short integer which
					 * must be cast to a long integer before
					 * it can be used.
					 */
					short short_integer;

					short_integer = (short)va_arg(arg, int);

					sv = short_integer;
				}
				else if (parameter_size == parameter_size_byte)
				{
					/* Parameter is a byte-sized integer which
					 * must be cast to a long integer before
					 * it can be used.
					 */
					short byte_integer;

					byte_integer = (char)va_arg(arg, int);

					sv = byte_integer;
				}
				else
				{
					#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
					{
						if(parameter_size == parameter_size_long_long || parameter_size == parameter_size_intmax_t)
							sv = va_arg(arg, long long);
						else
							sv = va_arg(arg, int);
					}
					#else
					{
						sv = va_arg(arg, int);
					}
					#endif /* __GNUC__ */
				}

				if(sv < 0)
				{
					SHOWMSG("negative number");

					SET_FLAG(format_flags,FORMATF_IsNegative);

					v = (-sv);
				}
				else
				{
					v = sv;
				}
			}
			else
			{
				if(conversion_type == 'o')
					SHOWMSG("unsigned integer (octal notation)");
				else if (conversion_type == 'x')
					SHOWMSG("unsigned integer (hexadecimal notation)");
				else
					SHOWMSG("unsigned integer");

				if(parameter_size == parameter_size_short)
				{
					/* Parameter is a short integer which
					 * must be cast to a long integer before
					 * it can be used.
					 */
					unsigned short short_integer;

					short_integer = (unsigned short)va_arg(arg, unsigned int);

					v = short_integer;
				}
				else if (parameter_size == parameter_size_byte)
				{
					/* Parameter is a byte-sized integer which
					 * must be cast to a long integer before
					 * it can be used.
					 */
					unsigned char byte_integer;

					byte_integer = (unsigned char)va_arg(arg, unsigned int);

					v = byte_integer;
				}
				else
				{
					#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
					{
						if(parameter_size == parameter_size_long_long || parameter_size == parameter_size_intmax_t)
							v = va_arg(arg, unsigned long long);
						else
							v = va_arg(arg, unsigned int);
					}
					#else
					{
						v = va_arg(arg, unsigned int);
					}
					#endif /* __GNUC__ */
				}

				CLEAR_FLAG(format_flags,FORMATF_ProduceSign);
				CLEAR_FLAG(format_flags,FORMATF_ProduceSpace);
			}

			/* Don't print anything if the precision is 0 and the number
			   itself is 0. */
			if(v != 0 || precision != 0)
			{
				const char * digit_encoding;
				int radix;

				/* Only add the zero (%o) or hex (%x) prefix if the value to
				   be converted is non-zero. */
				if(FLAG_IS_SET(format_flags,FORMATF_AlternateConversion) && v != 0)
				{
					if(conversion_type == 'o')
						SET_FLAG(format_flags,FORMATF_ZeroPrefix);
					else if (conversion_type == 'x')
						SET_FLAG(format_flags,FORMATF_HexPrefix);
				}

				if(conversion_type == 'o')
					radix = 8;
				else if (conversion_type == 'x')
					radix = 16;
				else
					radix = 10;

				if(FLAG_IS_SET(format_flags,FORMATF_CapitalLetters))
					digit_encoding = "0123456789ABCDEF";
				else
					digit_encoding = "0123456789abcdef";

				do
				{
					output_buffer--;
					output_len++;

					(*output_buffer) = digit_encoding[v % radix];
					v /= radix;
				}
				while(v > 0 && buffer < output_buffer);

				while(output_len < precision && output_buffer > buffer)
				{
					output_buffer--;
					output_len++;

					(*output_buffer) = '0';
				}
			}
		}
		else if (conversion_type == 's')
		{
			SHOWMSG("string");

			assert(arg != NULL);

			#if defined(CHECK_FOR_NULL_POINTERS)
			{
				if(arg == NULL)
				{
					__set_errno(EFAULT);
					goto out;
				}
			}
			#endif /* CHECK_FOR_NULL_POINTERS */

			output_buffer = va_arg(arg, char *);

			#if defined(NDEBUG)
			{
				if(output_buffer == NULL)
				{
					output_buffer = buffer;
					strcpy(output_buffer,"");
				}
			}
			#else
			{
				if(output_buffer == NULL)
				{
					output_buffer = buffer;
					strcpy(output_buffer,"*NULL POINTER*");
				}
			}
			#endif /* NDEBUG */

			if(precision < 0)
			{
				output_len = strlen(output_buffer);
			}
			else
			{
				int i;

				output_len = precision;

				for(i = 0 ; i < precision ; i++)
				{
					if(output_buffer[i] == '\0')
					{
						output_len = i;
						break;
					}
				}
			}

			D(("string = '%s', length = %ld",output_buffer,output_len));

			CLEAR_FLAG(format_flags,FORMATF_ProduceSign);
			CLEAR_FLAG(format_flags,FORMATF_ProduceSpace);
		}
		else if (conversion_type == 'n')
		{
			SHOWMSG("number of characters");

			assert(arg != NULL);

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

				short_ptr = va_arg(arg, short *);

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

				(*short_ptr) = len;
			}
			else if (parameter_size == parameter_size_byte)
			{
				char * byte_ptr;

				byte_ptr = va_arg(arg, char *);

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

				(*byte_ptr) = len;
			}
			else
			{
				#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
				{
					if(parameter_size == parameter_size_long_long || parameter_size == parameter_size_intmax_t)
					{
						long long * int_ptr;

						int_ptr = va_arg(arg, long long *);

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

						(*int_ptr) = len;
					}
					else
					{
						int * int_ptr;

						int_ptr = va_arg(arg, int *);

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

						(*int_ptr) = len;
					}
				}
				#else
				{
					int * int_ptr;

					int_ptr = va_arg(arg, int *);

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

					(*int_ptr) = len;
				}
				#endif /* __GNUC__ */
			}

			continue;
		}
		else
		{
			SHOWMSG("anything else");

			/* Just store the conversion_type character. */
			if(__putc(conversion_type,stream,buffer_mode) == EOF)
				goto out;

			len++;

			continue;
		}

		/* Get ready to prefix a sign character, if required. */
		if(FLAG_IS_SET(format_flags,FORMATF_IsNegative))
			prefix = "-";
		else if (FLAG_IS_SET(format_flags,FORMATF_ProduceSign))
			prefix = "+";
		else if (FLAG_IS_SET(format_flags,FORMATF_ProduceSpace))
			prefix = " ";
		else if (FLAG_IS_SET(format_flags,FORMATF_ZeroPrefix))
			prefix = "0";
		else
			prefix = NULL;

		if(FLAG_IS_SET(format_flags,FORMATF_HexPrefix))
		{
			strcpy(prefix_buffer,(prefix != NULL ? prefix : ""));
			strcat(prefix_buffer,FLAG_IS_SET(format_flags,FORMATF_CapitalLetters) ? "0X" : "0x");

			prefix = prefix_buffer;
		}

		if(FLAG_IS_SET(format_flags,FORMATF_LeftJustified))
		{
			int i;

			if(prefix != NULL)
			{
				for(i = 0 ; prefix[i] != '\0' ; i++)
				{
					/* One less character to fill the output with. */
					minimum_field_width--;

					if(__putc(prefix[i],stream,buffer_mode) == EOF)
						goto out;

					len++;
				}
			}

			for(i = 0 ; i < output_len ; i++)
			{
				if(__putc(output_buffer[i],stream,buffer_mode) == EOF)
					goto out;

				len++;
			}

			#if defined(FLOATING_POINT_SUPPORT)
			{
				for(i = 0 ; i < num_trailing_zeroes ; i++)
				{
					if(__putc('0',stream,buffer_mode) == EOF)
						goto out;

					output_len++;
					len++;
				}

				for(i = 0 ; trail_string[i] != '\0' ; i++)
				{
					if(__putc(trail_string[i],stream,buffer_mode) == EOF)
						goto out;

					output_len++;
					len++;
				}
			}
			#endif /* FLOATING_POINT_SUPPORT */

			for(i = output_len ; i < minimum_field_width ; i++)
			{
				/* Left justified output defaults to use the blank
				   space as the fill character. */
				if(__putc(' ',stream,buffer_mode) == EOF)
					goto out;

				len++;
			}
		}
		else
		{
			int i;

			/* If we have to add the prefix later, make sure that
			   we don't add too many fill characters in front of
			   it now. */
			if(prefix != NULL)
			{
				for(i = 0 ; prefix[i] != '\0' ; i++)
				{
					/* One less character to fill the output with. */
					minimum_field_width--;

					if(fill_character == '0')
					{
						if(__putc(prefix[i],stream,buffer_mode) == EOF)
							goto out;

						len++;
					}
				}

				/* That takes care of the sign. */
				if(fill_character == '0')
					prefix = NULL;
			}

			#if defined(FLOATING_POINT_SUPPORT)
			{
				trail_string_len = strlen(trail_string);

				minimum_field_width -= num_trailing_zeroes + trail_string_len;
			}
			#endif /* FLOATING_POINT_SUPPORT */
			
			for(i = output_len ; i < minimum_field_width ; i++)
			{
				if(__putc(fill_character,stream,buffer_mode) == EOF)
					goto out;

				len++;
			}

			/* If we still have a sign character to add, do it here. */
			if(prefix != NULL)
			{
				for(i = 0 ; prefix[i] != '\0' ; i++)
				{
					if(__putc(prefix[i],stream,buffer_mode) == EOF)
						goto out;

					len++;
				}
			}


			for(i = 0 ; i < output_len ; i++)
			{
				if(__putc(output_buffer[i],stream,buffer_mode) == EOF)
					goto out;

				len++;
			}

			#if defined(FLOATING_POINT_SUPPORT)
			{
				for(i = 0 ; i < num_trailing_zeroes ; i++)
				{
					if(__putc('0',stream,buffer_mode) == EOF)
						goto out;

					len++;
				}

				for(i = 0 ; i < trail_string_len ; i++)
				{
					if(__putc(trail_string[i],stream,buffer_mode) == EOF)
						goto out;

					len++;
				}
			}
			#endif /* FLOATING_POINT_SUPPORT */
		}
	}

	if(FLAG_IS_CLEAR(iob->iob_Flags,IOBF_NO_NUL))
	{
		if(__putc('\0',stream,buffer_mode) == EOF)
			goto out;
	}

	result = len;

 out:

	#if defined(FLOATING_POINT_SUPPORT)
	{
		if(internal_buffer != NULL && internal_buffer_size > 0)
			free(internal_buffer);
	}
	#endif /* FLOATING_POINT_SUPPORT */

	/* Note: if buffering is disabled for this stream, then we still
	   may have buffered data around, queued to be printed right now.
	   This is intended to improve performance as it takes more effort
	   to write a single character to a file than to write a bunch. */
	if(result != EOF && (iob->iob_Flags & IOBF_BUFFER_MODE) == IOBF_BUFFER_MODE_NONE)
	{
		if(__iob_write_buffer_is_valid(iob) && __flush_iob_write_buffer(iob) < 0)
			result = EOF;
	}

	funlockfile(stream);

	RETURN(result);
	return(result);
}
