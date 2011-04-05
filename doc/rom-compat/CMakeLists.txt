###########################################################################
# Gens/GS II Documentation.                                               #
# rom-compat: ROM compatibility lists.                                    #
#                                                                         #
# Copyright (c) 2011 by Pietro Gagliardi.                                 #
# Copyright (c) 2011 by David Korth.                                      #
#                                                                         #
# This program is free software; you can redistribute it and/or modify it #
# under the terms of the GNU General Public License as published by the   #
# Free Software Foundation; either version 2 of the License, or (at your  #
# option) any later version.                                              #
#                                                                         #
# This program is distributed in the hope that it will be useful, but     #
# WITHOUT ANY WARRANTY; without even the implied warranty of              #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           #
# GNU General Public License for more details.                            #
#                                                                         #
# You should have received a copy of the GNU General Public License along #
# with this program; if not, write to the Free Software Foundation, Inc., #
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           #
###########################################################################

PROJECT(rom-compat)
cmake_minimum_required(VERSION 2.6)

# Check if we have an awk interpreter installed.
FIND_PROGRAM(HAVE_GAWK awk)
IF(NOT HAVE_GAWK)
	MESSAGE(WARNING "gawk not found.\nROM compatibility lists will not be processed.")
ELSE(NOT HAVE_GAWK)
	SET(GAWK_BINARY ${HAVE_GAWK})
ENDIF(NOT HAVE_GAWK)

# Source files. (AWK script and CSS)
SET(ROM_COMPAT_AWK rom-compat.awk)
SET(ROM_COMPAT_CSS rom-compat.css)

# Source files. (ROM lists)
SET(ROM_LISTS
	test.txt
	)

#
# Macro to process a list of files using rom-compat.awk.
# out_html_files: Name of variable to store the output filenames in.
# Additional parameters: List of text files to process.
#
MACRO(PROCESS_ROM_COMPAT out_html_files)
	FOREACH(arg ${ARGN})
		SET(PRC_TMP_TXT_FILE "${arg}")
		STRING(REGEX REPLACE "\\.txt$" ".html" PRC_TMP_HTML_FILE "${arg}")
		
		# Prepend directory names.
		SET(PRC_TMP_TXT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${PRC_TMP_TXT_FILE}")
		SET(PRC_TMP_HTML_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PRC_TMP_HTML_FILE}")
		
		# Create the custom command.
		ADD_CUSTOM_COMMAND(
			OUTPUT "${PRC_TMP_HTML_FILE}"
			COMMAND "${GAWK_BINARY}" -f "${CMAKE_CURRENT_SOURCE_DIR}/${ROM_COMPAT_AWK}" "${PRC_TMP_TXT_FILE}" >"${PRC_TMP_HTML_FILE}"
			DEPENDS "${ROM_COMPAT_AWK}" "${arg}"
			)
		
		# Add the HTML file to the output variable.
		SET(${out_html_files} ${${out_html_files}} "${PRC_TMP_HTML_FILE}")
	ENDFOREACH(arg)
	
	# Unset temporary variables.
	UNSET(PRC_TMP_TXT_FILE)
	UNSET(PRC_TMP_HTML_FILE)
ENDMACRO(PROCESS_ROM_COMPAT)

# Process the ROM compatibility lists.
PROCESS_ROM_COMPAT(HTML_FILES ${ROM_LISTS})

# Copy the CSS file to the build directory.
ADD_CUSTOM_COMMAND(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${ROM_COMPAT_CSS}"
	COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${ROM_COMPAT_CSS}" "${CMAKE_CURRENT_BINARY_DIR}/${ROM_COMPAT_CSS}"
	DEPENDS "${ROM_COMPAT_CSS}"
	)

IF(HAVE_GAWK)
	ADD_CUSTOM_TARGET(rom-compat_target ALL
		DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${ROM_COMPAT_CSS}" ${HTML_FILES}
		)
ENDIF(HAVE_GAWK)

