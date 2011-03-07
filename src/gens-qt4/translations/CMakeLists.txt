PROJECT(gens-qt4_translations)
cmake_minimum_required(VERSION 2.6)

# gens-qt4 translations subproject.

# Compile the translations.
SET(TRANSLATION_FILES
	gens-qt4_en.ts
	gens-qt4_en_GB.ts
	gens-qt4_ja.ts
	)
QT4_ADD_TRANSLATION(QM_FILES ${TRANSLATION_FILES})

# Make the destination files available in parent score.
SET(QM_FILES ${QM_FILES} PARENT_SCOPE)
ADD_CUSTOM_TARGET(translations_target DEPENDS ${QM_FILES})

# TODO: Verify install path.
INSTALL(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/translations)
