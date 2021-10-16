# The input is truncated after 2/3 of a rightwards arrow U2192 -> e2 86 92
FLAGS='--strip --expect UTF-8 --utf8 --lenient'
STDIN='\xef\xbb\xbfpartial arrow: \xe2\x86'
STDOUT='partial arrow: '
STDERR=''
EXITVAL='0'
