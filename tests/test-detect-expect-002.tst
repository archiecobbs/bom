FLAGS='--detect --expect UTF-16LE'
STDIN='\xef\xbb\xbfblahblah'
STDOUT=''
STDERR='bom: unexpected BOM type UTF-8\n'
EXITVAL='2'
