# Source: http://public.kitware.com/Bug/bug_view_page.php?bug_id=10069

# This file is used by EnableLanguage in cmGlobalGenerator to
# determine that the selected ASM_NASM "compiler"
# can actually "compile" and link the most basic of programs.   If not, a 
# fatal error is set and cmake stops processing commands and will not generate
# any makefiles or projects.

SET(ASM_DIALECT "_NASM")
INCLUDE(CMakeTestASMCompiler)
SET(ASM_DIALECT)
