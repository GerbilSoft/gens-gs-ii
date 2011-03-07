#!/bin/sh
# Qt lupdate script for gens-qt4.
# Run this script to update the translations from the source code.
# NOTE: This script must be run from the translations/ directory!

# Source file extensions.
SRC_EXTS="h,hpp,c,cpp,m,mm,ui"

# Process the translation files.
for TS_FILE in *.ts; do
	lupdate -extensions "$SRC_EXTS" ../ -ts "$TS_FILE"
done
