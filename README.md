# clib2 – An ISO 'C' (1994) compliant runtime library for AmigaOS

[![Build](https://github.com/adtools/clib2/workflows/CI/badge.svg)](https://github.com/adtools/clib2/actions)
[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

## What is this?

This is my attempt to get Samba 2.2.x ported to the Amiga. My first Amiga port required *SAS/C* and a number of strange tricks had to be pulled to get it to support the kind of environment Samba needed. But with the introduction of Samba 2.2.x many of those tricks did not work any more, which is why I decided to attack the problem at the root, namely the runtime library.

Because it was no longer possible to build Samba with *SAS/C* on the new Amiga platform, the idea came up to move development to the GNU 'C' compiler. This turned out to be a challenge due to its somewhat underdeveloped runtime library and header files. Eventually, I decided to rewrite that library from scratch.


## What does it do?

Using *'C' - A reference manual* (4th edition) as a reference I wrote a set of header files, then proceeded to implement each single function referenced in them. With few exceptions in the area of wide character support, the result should be a feature complete implementation of the ISO 'C' (1994) runtime library. The library was subsequently updated to offer functionality defined in *ISO/IEC 9899:1999*, also known as *C99*.

Because Samba needs a few POSIX-like routines to be supported, the library functionality is complemented by a set of routines described in *Advanced programming in the Unix environment*.

This is not a portable implementation of the library in the sense that you could move it from one 'C' compiler on one operating system to another. This is an Amiga specific implementation.

The library supports floating point math, which, for the 68k platform, is limited to IEEE single and double precision or M68881 inline math. There is no support for the fast floating point (FFP) format or exclusive IEEE single precision. You either get double precision (IEEE math) or extended precision (M68881 inline math). What it is that you get is determined at compile time. Use the `IEEE_FLOATING_POINT_SUPPORT` preprocessor symbol to activate IEEE math code and the `M68881_FLOATING_POINT_SUPPORT` symbol for M68881 inline math.

For the PowerPC platform, the library uses code borrowed from <a href="http://www.netlib.org/fdlibm/">fdlibm 5.3</a>, which is a portable library of arithmetic functions developed by Sun Microsystems which, for example, is also used within the Java platform.

## What does it not do?

This library is a departure from the typical 'C' runtime environments of the past which had to run on all AmigaOS releases, down to *Kickstart 1.1*. This very library was designed to take advantage of the routines available since *Kickstart 2.04* was introduced and virtually nobody ever put to use. This helps to cut the code size, and it also helps to keep bugs out of the library by falling back onto well-tested implementations. However, the catch is that the code won't run under *Kickstart 1.3* and below. But then these operating system releases have been obsolete for more than a decade, and you can always go back to a compiler environment which supports them.

There is very little support for `amiga.lib` functionality. There is `NewList()`, `HookEntry()`, `CallHook()`, `CallHookA()`, the `DoMethod()` family, the *RexxVars* family, but that's all. If you need more, you would have to implement it yourself. Put another way, if you absolutely need functionality that is only found in `amiga.lib`, you really shouldn't need it in the first place.


## Where does the source code come from?

I originally thought that it might be helpful to piece this library together from various sources, such as the BSD libc. It turned out that this code was so 'portable' that it became much more complex than it ought to be. Also, some side-effects were present which considerably changed the behaviour of the library. For example, the BSD libc uses `bcopy()` as an alias for `memcpy()`, and unlike `memcpy()` is documented to, `bcopy()` supports overlapping copies.

Eventually, I wrote virtually all the code myself, borrowing algorithmic ideas from the BSD libc and the *Manx Aztec 'C'* runtime library. Because I don't know much about the environment *GCC* expects, I borrowed code snippets from *libnix*, which was written by Matthias Fleischer and Gunther Nikl. This in particular concerns the integer and floating point math support, the `setjmp`/`longjmp` routines and the startup code. The M68881 inline math code comes from the `<math-68881.h>` file written by Matthew Self `(self [at] bayes.arc.nasa.gov)`.


## Limitations and caveats

There is hardly any documentation on the code I wrote. In part this is due to the fact that the code itself is very simple in design. It should speak for itself. However, to make a usable runtime library you have to have a user documentation as in *man pages* or *AutoDocs*. We will eventually have to have *AutoDocs* for this library.

The exception handling in the math code is not particularly effective. For one part this is due to the fact that there is no exception handler installed by the runtime library when it starts up which could catch and process the error conditions the CPU or FPU generates. The idea was to provide for a portable runtime library with little to no assembly language involved. To make the exception handling complete, such code would be necessary.

The library currently builds under *SAS/C*, but because the 'normal' program startup code is not utilized, the base relative (A4) addressing does not work. If you are going to test it, use the `data=faronly` option to compile the library and the programs.

Different build *Makefiles* are supplied for use with *GCC*. There is a `GNUmakefile.68k` for the 68k platform and a `GNUmakefile.os4` for the AmigaOS4 PowerPC version.

### Floating point math and functions (`scanf()`, `printf()`, etc.)

The plain `libc.a`, which your software would be linked against by default, does not contain any floating point support code. This means, for example, that `printf("%f",...)` will not produce the desired output and that `scanf("%f",...)` may not read any data at all. If your program needs functions such as these or `atod()` then you must link against `libm.a` or the equivalent.

To link the floating point support code with your software, use the `-lm` compiler option. 
**Careful!** The order in which you specify the libraries to link against is important here. Thus, `gcc -o test test.c -lm -lc` would correctly link the program `test` against the proper floating point math library, but `gcc -o test test.c -lc -lm` would not.

### The thread-safe library

Thread-safety does not imply that you can have multiple callers access and close the same file. There is no resource tracking to that degree yet. All that the thread-safety tries to afford you is not to get into big trouble if simultaneous and overlapping accesses to files, memory allocation and other resources are taking place.

The library code is supposed to be thread-safe if built with the `__THREAD_SAFE` preprocessor symbol defined. Note that 'thread-safe' does **not** mean 'reentrant'. Multiple callers for certain library functions are permitted, but not for all of them. For example, `mkdtemp()` is not thread-safe, and neither is `rand()` or `localtime()`. But as per *POSIX 1003.1c-1995* there are thread-safe variants of `rand()` and `localtime()` called `rand_r()`, `localtime_r()`, and others.

The use of the socket I/O functions is problematic because the underlying `bsdsocket.library` API is not supposed to be used by any process other than the one which opened it. While one TCP/IP stack (my own "Roadshow") allows you to share the library base among different processes, if so configured, it is the exception. No other TCP/IP stack available for the Amiga robustly supports a similar feature. If the TCP/IP stack supports this feature, then the global variable `__can_share_socket_library_base` will be set to a non-zero value.

Errors reported by the socket I/O functions which modify the global variables `errno` and `h_errno` may be directed to call the `__set_errno()` and `__set_h_errno()` functions instead, if the TCP/IP stack supports this feature. The global variable `__thread_safe_errno_h_errno` will be set to a non-zero value if it does.

A much more serious problem resides with the `exit()`, `abort()`, `assert()` and `raise()` functions, and how the `SIGINT` signal is processed. In the thread-safe library only the `main()` function may directly or indirectly call the `exit()` function. No child process may do so, since this would wreck its stack context, crashing it instantly; the main program would be very likely to crash, too, because `exit()` will clean up after all memory allocations and files currently in use. Functions such as `abort()` and `raise()` may call the `exit()` function indirectly. And the `raise()` function may be invoked as part of the `Control+C` checking. You should make sure that the signal handling does not affect any child processes. This can be done by replacing the `__check_abort()` function or by disabling `SIGINT` processing altogether, such as through a `signal(SIGINT,SIG_IGN)` call.

Also take care with file I/O involving the `stdin`/`stdout`/`stderr` streams; read/write operations on these streams will be mapped to the `Input()`/`Output()`/`ErrorOutput()` file handles of the process performing these operations. Since only this small set of operations is mapped, functions such as `fcntl()` or `select()` will not work on the `stdin`/`stdout`/`stderr` streams and the corresponding file descriptors `STDIN_FILENO`/`STDOUT_FILENO`/`STDERR_FILENO`. It is therefore strongly recommended to use the thread-safe library only for applications which can cope with the limitations described above.

### Using gmon (PowerPC only)

To use profiling, two steps are required. First of all, your program must be compiled with the *GCC* command line option `-pg`. This instructs the compiler to generate special profiling code in the prologue and epilogue of each function. Additionally, the program must be linked against `libprofile.a`. To do this, either manually add `-lprofile` to the linker command line, or modify the specs file as follows. Find the lines that look like this (it may actually differ slightly from your specs file, but the important thing is that the line before the line to be modified reads `lib:`):

```
lib:
--start-group -lc --end-group
```

You will have to modify this to look like this:

```
lib:
%{pg: -lprofile} --start-group -lc --end-group
```

Normally, the `specs` file is located at the compiler's installation directory. For cross-compilers, this is `/usr/local/amiga/lib/gcc/ppc-amigaos/*compiler-version*/specs`. For a native compiler, it's in `gcc:lib/gcc/ppc-amigaos/*compiler-version*/specs`. Most likely, your compiler will already have this added to it's `specs` file.

Profiling makes use of a special PowerPC facility called the *Performance Monitor*. It allows to "mark" tasks and count only during while a marked task is running. This allows performance analysis to be made independent of the actual system load. The *Performance Monitor* is available on all PowerPC models supported by AmigaOS 4 except for the *603e*, and embedded versions of the PowerPC like the *405* and *440* series. Consult the manual of the appropriate chip for more information.

### Implementation defined behaviour

#### 'C' language

##### Environment

The `main(int argc,char **argv);` function may be called with an `argc` value of 0, in which case the `argv` variable will contain a pointer to the Amiga Workbench startup message, which is of type `struct WBStartup *`, and is defined in the Amiga system header file `<workbench/startup.h>`.

##### Characters

The current locale is derived from the current Amiga operating system locale settings. The `setlocale("")` function call will choose the current Amiga operating system locale settings. Any other name passed to the `setlocale()` function, with the exception of `"C"`, which selects the 'C' locale, must be a locale name, as used by the Amiga operating system function `OpenLocale()` in `locale.library`.

##### Floating-point

The 68k version of *clib2* supports single and double precision floating point numbers, according to the *IEEE 754* standard. The software floating point number support is built upon the Amiga operating system libraries `mathieeesingbas.library`, `mathieeedoubbas.library` and `mathieeedoubtrans.library`. The hardware floating point number support uses the M68881/M68882/M68040/M68060 floating point unit instead.

The PowerPC version of *clib2* supports only double precision floating point numbers, according to the *IEEE 754* standard, because that is exactly what the PowerPC CPU supports. Single precision numbers may be implicitly converted to double precision numbers. This also means that the *C99* data type `long double` is identical to the `double` data type. Because there is no difference between these two, the library omits support for *C99* functions specifically designed to operate on `long double` data types, such as `rintl()`.

Both the 68k and the PowerPC versions of *clib2* may call software floating point support routines in order to perform double and single precision operations that go beyond simple addition and multiplication, such as `sqrt()`. These functions come from Sun Microsystems <a href="http://www.netlib.org/fdlibm/">fdlibm 5.3</a> library.

Unless your software is linked against `libm.a` no floating point functions will be available to it, possibly causing a linker error. When using the GNU 'C' compiler, you will want to add the option `-lm -lc` to the linker command line.

The exception handling is currently entirely out of control of the developer and solely subject to the rules imposed by the operating system itself.

The `fmod()` function returns the value of the `x` parameter and sets `errno` to `EDOM` if the `y` parameter value is 0.

#### Library functions

##### `NULL`

The `NULL` pointer constant is defined in the `<stddef.h>` header and will expand to `((void *)0L)` if the 'C' compiler is used. For a C++ compiler the constant will expand to `0L` instead.

##### `assert()` diagnostic messages

The diagnostic messages printed by the `assert()` function take the following form:

> `[*program name*] *file*:*line*: failed assertion "*condition*".`

where:

Label | Meaning
----- | -------
program name | Optional program name; if the program name is not yet known, then the entire text enclosed in square brackets will be omitted.
file | The value of the `__FILE__` symbol at the location of the `assert()` call.
line | The value of the `__LINE__` symbol at the location of the `assert()` call.
condition | The condition passed to the `assert()` function.

If available, the diagnostic messages will be sent to `stderr`.

If the program was launched from Workbench or if the global variable `__no_standard_io` is set to a non-zero value, then the assertion failure message will not be displayed in the shell window, but in a requester window. The diagnostic message shown in this window will take the following form:

> `Assertion of condition "*condition*" failed in file "*file*", line *line*.`

The name of the program, if it is know at that time, will be displayed in the requester window title.

##### Signal handling

Only the minimum of required signals are supported by this library. These are `SIGABRT`, `SIGFPE`, `SIGILL`, `SIGINT`, `SIGSEGV` and `SIGTERM`.

As of this writing `SIGFPE` is never called by the floating point library functions.

The `Ctrl+C` event is translated into `SIGINT`. Signal delivery may be delayed until a library function which polls for the signal examines it. This means, for example, that a runaway program caught in an infinite loop cannot be aborted by sending it a `Ctrl+C` event unless special code is added which tests for the presence of the signal and calls the `__check_abort()` function on its own accord.

Processing of the `Ctrl+C` event involves the internal `__check_abort()` function which polls for the presence of the event and which will call `raise(SIGINT);`. The `__check_abort()` function may be replaced by user code.

##### Files

No new line characters are written unless specifically requested.

Space characters in a text stream before a new line character are read in and are not discarded.

When data is read from a file, the last character does not have to be a new line character.

No `NUL` byte will be appended to data written to a binary stream.

There is no difference between text and binary streams.

Writing to a text or binary stream does not truncate the associated file. A stream may be truncated by the initial `fopen()` call if the `mode` parameter starts with the letter `w`.

The file position indicator is initially set to the end of an append mode stream.

##### `printf()` family

The `%p` conversion is the hexadecimal representation of the pointer, and it is preceded by the string `0x`.

The `%a`, `%e`, `%f`, `%g`, `%A`, `%E`, `%F` and `%G` specifiers will produce the string `inf` for infinity.

##### `scanf()` family

The input for the `%p` conversion must be a hexadecimal number, preceded by either the string `0x` or `0X`.

In the `%[` conversion a `-` (dash) character that is neither the first nor the last character in the scanset indicates that a subrange of characters should be used. Thus `%[a-d]` is equivalent to `%[abcd]`.

The period (.) is the decimal-point character. The locale specific decimal-point character is accepted as an alternative to the period (.).

##### `malloc()`, `realloc()` and `calloc()`

In the standard `libc.a` implementation any request to allocate 0 (zero) bytes will fail. A result value of `NULL` will be returned and the global `errno` variable will be set to `EINVAL`.

In the `libunix.a` implementation a request to allocate 0 (zero) bytes will result in an allocation of at least 4 bytes, which will be set to zero. Each zero length allocation will return a different memory address.

##### `rename()`

In the standard `libc.a` implementation the `rename()` function will fail if there already is a file or directory by the new name to be used.

In the `libunix.a` implementation the `rename()` function will delete any existing file or directory by the new name.

##### `remove()`

In the standard `libc.a` implementation the `remove()` function will fail if the file is protected by deletion or currently in use.

In the `libunix.a` implementation the `remove()` function will remove the file when the program exits or the file is closed.

##### `abort()`

The `abort()` function will flush all buffered files, close all the files currently open and delete temporary files.

##### `exit()` and `_Exit()`

The value passed to the `exit()` function will be passed to the Amiga operating system. The value of `EXIT_FAILURE` is equivalent to `RETURN_FAIL` as defined in the Amiga system header file `<dos/dos.h>`; this value maps to the number 20. The value of `EXIT_SUCCESS` is equivalent to `RETURN_OK` as defined in the Amiga system header file `<dos/dos.h>`; this value maps to the number 0.

The `_Exit()` function will flush all buffered files, close all the files currently open and delete temporary files.

##### `getenv()`

Environment data is retrieved from the global Amiga operating system environment storage area through the `dos.library/GetEnv()` function. Global variables are stored in files in the `ENV:` directory.

##### `system()`

If the `command` parameter is not NULL and the `system()` function returns, then the result will be equivalent to the exit code of the program invoked, or -1 if the program could not be started. This follows the behaviour of the Amiga operating system function `dos.library/System()`. A return value of 0 typically indicates successful execution and a value > 0  typically indicates failure.

##### Time

The default time zone is derived from the Amiga operating system locale settings and takes the form `GMT+*hh*` or `GMT-*hh*`, respectively, in which *hh* stands for the difference between the local time zone and Greenwich Mean Time (actually, this is not GMT but UTC).

The `clock_t` and `time_t` types are unsigned 32 bit integers. The `time_t` epoch starts with midnight January 1st, 1970.

Daylight savings time is not supported.

The reference point used by the `clock()` function is the time when the program was started.

#### Locale specific behaviour

The direction of printing is from left to right.

The period (.) is the decimal-point character.

The `strftime()` behaviour follows the Amiga operating system locale settings. If the 'C' locale is in effect, then the output generated by the `%Z` takes the form `GMT+*hh*` or `GMT-*hh*`, respectively, in which *hh* stands for the difference between the local time zone and Greenwich Mean Time (this is really UTC).

## Conventions and design issues

You will have noticed the 330+ files in this directory. This is not the best way to organize a runtime library, but at least all the bits and pieces are in plain sight. Each file stands for the one or two routines it contains. The name indicates what routine(s) that might be. Each file name is prefixed by the name of the header file in which the corresponding routine is defined. So, for example, you will find that `unistd_lchown.c` contains the definition of the `lchown()` routine, which has its prototype defined in the `<unistd.h>` header file.

Internal function and variables which need to be visible across several modules have names prefixed with two underscores, as in `__stdio_init()`.

By default all library routines follow the ISO 'C' conventions in that where implementation defined behaviour is permitted, the AmigaOS rules are followed. For example, `unlink()` will by default operate like `DeleteFile()` and `rename()` will return with an error code set if the name of the file/directory to be renamed would collide with an existing directory entry.


## The startup code

There are three program startup files provided. The most simplistic is in `startup.c` which I use for *SAS/C*. It just invokes the setup routine which eventually calls `main()` and drops straight into `exit()`.

The `ncrt0.S` file was adapted from the *libnix* startup code which sets up the base relative data area, if necessary (the `SMALL_DATA` preprocessor symbol must be defined).

The `nrcrt0.S` file was adapted from *libnix* startup code, too, and sets up the base relative data area for programs to be made resident. Note that the `geta4()` stub is missing here; it wouldn't work in a resident program anyway.

The `ncrt0.S` and `nrcrt0.S` files are considerably smaller and less complex than the *libnix* code they are based on. This is because in this library design all the more complex tasks are performed in the `stdlib_main.c` file rather than in assembly language.


## Documentation

Well, you're reading it. There isn't anything much yet. You can consult the book *'C' - A reference manual* and you could look at the
<a href="http://www.opengroup.org/onlinepubs/007904975">Open Group's Single Unix
Specification</a>.

It is recommended to browse the contents of the `include` directory. The header files contain information on library behaviour and not just data type and function prototype definitions. Specifically, the `<dos.h>` header file contains documentation about special libraries and global variables which may be used or replaced by user code.


## Legal status

Because this library is in part based upon free software it would be uncourteous not to make it free software itself. The BSD license would probably be appropriate here.

The PowerPC math library is based in part on work by Sun Microsystems:

<pre>
====================================================
Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.

Developed at SunPro, a Sun Microsystems, Inc. business.
Permission to use, copy, modify, and distribute this
software is freely granted, provided that this notice
is preserved.
====================================================
</pre>

## Contacting the author

The basic work was done by Olaf Barthel during two weeks in July 2002. You can reach me at:

    Olaf Barthel
    Gneisenaustr. 43
    D-31275 Lehrte

Or via e-mail:

    obarthel [at] gmx.net
