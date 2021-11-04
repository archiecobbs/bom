# This has a multi-byte sequence that crosses our input buffer boundary
FLAGS='--strip --expect UTF-8 --utf8'
STDIN_BOM='\xef\xbb\xbf'
STDIN_1023=`yes aaaaaaaaaaaaaaa | tr -d \\\\n | head -c 1023`
STDIN_BROKEN_ARROW='\xe2\x86'
STDIN="${STDIN_BOM}${STDIN_1023}${STDIN_BROKEN_ARROW}"
STDOUT="${STDIN_1023}"
STDERR='bom: invalid UTF-8 byte sequence at file offset 1026\n'
EXITVAL='3'
