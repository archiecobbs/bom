# This has a multi-byte sequence that crosses our input buffer boundary
FLAGS='--strip --expect UTF-8 --utf8'
STDIN_BOM='\xef\xbb\xbf'
STDIN_1019=`yes aaaaaaaaaaaaaaa | tr -d \\\\n | head -c 1023`
STDIN_ARROW='\xe2\x86\x92'
STDIN="${STDIN_BOM}${STDIN_1019}${STDIN_ARROW}"
STDOUT="${STDIN_1019}${STDIN_ARROW}"
STDERR=''
EXITVAL='0'
