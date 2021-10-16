#!/bin/bash

# Bail on error
set -e

NCOLS="83"
MANPAGE="bom.1.in"

sed '/man page/q' < README.md > README.md.NEW

printf '```\n' >> README.md.NEW

groff -r LL=${NCOLS}n -r LT=${NCOLS}n -Tlatin1 -man "${MANPAGE}" \
  | sed -r -e 's/.\x08(.)/\1/g' -e 's/[[0-9]+m//g' \
  >> README.md.NEW

printf '```\n' >> README.md.NEW

mv README.md{.NEW,}
