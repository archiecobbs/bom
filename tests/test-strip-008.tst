# This has a multi-byte sequence that crosses our input buffer boundary
FLAGS='--strip --expect UTF-8 --utf8 --lenient'
STDIN_BOM='\xef\xbb\xbf'
STDIN_1023=`yes aaaaaaaaaaaaaaa | tr -d \\\\n | head -c 1023`
STDIN_BROKEN_ARROW='\xe2\x86'
STDIN_TRAILER='blah-blah'
STDIN="${STDIN_BOM}${STDIN_1023}${STDIN_BROKEN_ARROW}${STDIN_TRAILER}"
STDOUT="${STDIN_1023}${STDIN_TRAILER}"
STDERR=''
EXITVAL='0'
