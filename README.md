**bom** is a simple UNIX command line utility for dealing with Unicode byte order marks (BOM's).

Unicode byte order marks are "magic number" byte sequences that sometimes appear at the beginning of a file to indicate the file's character encoding. They're sometimes helpful but usually they're just annoying.

You can read more about byte order marks [here](https://en.wikipedia.org/wiki/Byte_order_mark).

**bom** operates in one of the following modes:

  * `bom --detect` Detect which type of byte order mark is present (if any) and print to standard output
  * `bom --strip` Strip off the byte order mark (if any) and output the remainder of the file, optionally also converting to UTF-8
  * `bom --print` Output the byte sequence corresponding to a byte order mark (useful for adding them to files)
  * `bom --list` List the supported byte order mark types

Here is the man page:
```
BOM(1)                      BSD General Commands Manual                      BOM(1)

NAME
     bom -- Decode Unicode byte order mark

SYNOPSIS
     bom --strip [--expect types] [--lenient] [--prefer32] [--utf8] [file]
     bom --detect [--expect types] [--prefer32] [file]
     bom --print type
     bom --list
     bom --help
     bom --version

DESCRIPTION
     bom decodes, verifies, reports, and/or strips the byte order mark (BOM) at the
     start of the specified file, if any.

     When no file is specified, or when file is -, read standard input.

OPTIONS
     -d, --detect
             Report the detected BOM type to standard output and then exit.

             See SUPPORTED BOM TYPES for possible values.

     -e, --expect types
             Expect to find one of the specified BOM types, otherwise exit with an
             error.

             Multiple types may be specified, separated by commas.

             Specifying NONE is acceptable and matches when the file has no (sup-
             ported) BOM.

     -h, --help
             Output command line usage help.

     -l, --lenient
             Silently ignore any illegal byte sequences encountered when converting
             the remainder of the file to UTF-8.

             Without this flag, bom will exit immediately with an error if an ille-
             gal byte sequence is encountered.

             This flag has no effect unless the --utf8 flag is given.

     --list  List the supported BOM types and exit.

     -p, --print type
             Output the byte sequence corresponding to the type byte order mark.

     --prefer32
             Used to disambiguate the byte sequence FF FE 00 00, which can be
             either a UTF-32LE BOM or a UTF-16LE BOM followed by a NUL character.

             Without this flag, UTF-16LE is assumed; with this flag, UTF-32LE is
             assumed.

     -s, --strip
             Strip the BOM, if any, from the beginning of the file and output the
             remainder of the file.

     -u, --utf8
             Convert the remainder of the file to UTF-8, assuming the character
             encoding implied by the detected BOM.

             For files with no (supported) BOM, this flag has no effect and the
             remainder of the file is copied unmodified.

             For files with a UTF-8 BOM, the identity transformation is still
             applied, so (for example) illegal byte sequences will be detected.

     -v, --version
             Output program version and exit.

SUPPORTED BOM TYPES
     The supported BOM types are:

     NONE    No supported BOM was detected.

     UTF-7   A UTF-7 BOM was detected.

     UTF-8   A UTF-8 BOM was detected.

     UTF-16BE
             A UTF-16 (Big Endian) BOM was detected.

     UTF-16LE
             A UTF-16 (Little Endian) BOM was detected.

     UTF-32BE
             A UTF-32 (Big Endian) BOM was detected.

     UTF-32LE
             A UTF-32 (Little Endian) BOM was detected.

     GB18030
             A GB18030 (Chinese National Standard) BOM was detected.

EXAMPLES
     To tell what kind of byte order mark a file has:

           $ bom --detect

     To normalize files with byte order marks into UTF-8, and pass other files
     through unchanged:

           $ bom --strip --utf8

     Same as previous example, but discard illegal byte sequences instead of gener-
     ating an error:

           $ bom --strip --utf8 --lenient

     To verify a properly encoded UTF-8 or UTF-16 file with a byte-order-mark and
     output it as UTF-8:

           $ bom --strip --utf8 --expect UTF-8,UTF-16LE,UTF-16BE

     To just remove any byte order mark and get on with your life:

           $ bom --strip file

RETURN VALUES
     bom exits with one of the following values:

     0       Success.

     1       A general error occurred.

     2       The --expect flag was given but the detected BOM did not match.

     3       An illegal byte sequence was detected (and --lenient was not speci-
             fied).

SEE ALSO
     iconv(1)

     bom: Decode Unicode byte order mark, https://github.com/archiecobbs/bom.

AUTHOR
     Archie L. Cobbs <archie.cobbs@gmail.com>

BSD                               October 14, 2021                              BSD
```
