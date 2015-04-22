PROJECT(libz80-fuse-tests)

# Generate the test file.
# TODO: Requires POSIX sh and sed.
ADD_CUSTOM_COMMAND(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tests.expected
	COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/generate_expected.sh
		${CMAKE_CURRENT_SOURCE_DIR}/fuse_files/tests.expected
		${CMAKE_CURRENT_BINARY_DIR}/tests.expected
	)

# Build the test program.
ADD_EXECUTABLE(coretest coretest.c)
TARGET_LINK_LIBRARIES(coretest z80)
SET_SOURCE_FILES_PROPERTIES(coretest.c
	PROPERTIES OBJECT_DEPENDS
		${CMAKE_CURRENT_BINARY_DIR}/tests.expected
	)

# Run the test program on the input data.
ADD_TEST(NAME libz80-coretest-generate
	COMMAND coretest
		${CMAKE_CURRENT_SOURCE_DIR}/fuse_files/tests.in
		>${CMAKE_CURRENT_BINARY_DIR}/tests.actual
	)

# Compare the test results.
# TODO: Requires diff and wc.
ADD_TEST(NAME libz80-coretest-compare
	COMMAND diff -q
		${CMAKE_CURRENT_BINARY_DIR}/tests.expected
		${CMAKE_CURRENT_BINARY_DIR}/tests.actual
	)
