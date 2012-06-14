# Copyright 2006-2010 The FLWOR Foundation.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Macro to find a Jar file whose filename possibly includes a version
# number. Some Apache distributions, such as FOP, include dependent jars
# with version numbers, and CMake's FIND_FILE() doesn't support glob
# patterns for the filename. This macro will find .jar files with optional
# patterns like "-1.0" in the basename.
#
# This macro operates the similarly to FIND_FILE(), except it only supports
# NAMES, PATHS, PATH_SUFFIXES, DOC, and NO_CMAKE_PATH. Other differences:
#   1. NAMES is required, and it should be passed the basename of the
#     jarfiles without the version number or ".jar" extension.
#   2. The only search directories will be those on CMAKE_PREFIX_PATH and
#     those specified by PATHS.
#
# For example:
#  ZORBA_FIND_JAR (FOP_JAR NAMES fop PATHS ${XSL_FOP_HOME} PATH_SUFFIXES build)
#
# will find fop.jar, fop-1.0.jar, or fop-23453.1234.jar, in any directory
# on CMAKE_PREFIX_PATH or in XSL_FOP_HOME, or in any subdirectory named
# "build" of any of those directories.
#
# For the moment this macro does not check the version number or enforce
# a particular/minimum version.
MACRO (ZORBA_FIND_JAR JARVARNAME)
  IF (NOT COMMAND PARSE_ARGUMENTS)
    MESSAGE (FATAL_ERROR
      "Please INCLUDE(\${Zorba_USE_FILE}) file "
      "prior to calling ZORBA_FIND_JAR().")
  ENDIF (NOT COMMAND PARSE_ARGUMENTS)

  PARSE_ARGUMENTS (ZFINDJAR "PATHS;NAMES;PATH_SUFFIXES" "DOC"
    "NO_CMAKE_PATH" ${ARGN})

  IF (NOT ZFINDJAR_NAMES)
    MESSAGE (FATAL_ERROR "'NAMES' argument is required for ZORBA_FIND_JAR")
  ENDIF (NOT ZFINDJAR_NAMES)

  # If the value is already cached, don't do anything
  IF (NOT ${JARVARNAME})

    # Form up the set of directories to search in.
    SET (_paths)
    IF (NOT ZFINDJAR_NO_CMAKE_PATH)
      SET (_paths ${CMAKE_PREFIX_PATH})
    ENDIF (NOT ZFINDJAR_NO_CMAKE_PATH)
    IF (ZFINDJAR_PATHS)
      LIST (APPEND _paths ${ZFINDJAR_PATHS})
    ENDIF (ZFINDJAR_PATHS)
    IF (NOT _paths)
      MESSAGE (WARNING "No place to search for ${ZFINDJAR_NAMES} jars! "
	"Set either CMAKE_PREFIX_PATH or pass PATHS")
    ENDIF (NOT _paths)

    # Iterate through each directory looking for each filename
    SET (_jarpath "${JARVARNAME}-NOTFOUND")
    FOREACH (_path ${_paths})
      IF (_jarpath)
	BREAK ()
      ENDIF (_jarpath)

      # Iterate through each potential jarname
      FOREACH (_name ${ZFINDJAR_NAMES})
	IF (_jarpath)
	  BREAK ()
	ENDIF (_jarpath)

	# Iterate through current directory and each suffix
	FOREACH (_dir "" ${ZFINDJAR_PATH_SUFFIXES})
	  # Form up the final full path
	  IF (_dir)
	    SET (_path2 "${_path}/${_dir}")
	  ELSE (_dir)
	    SET (_path2 "${_path}")
	  ENDIF (_dir)

	  # First see if the exact filename exists
	  IF (EXISTS "${_path2}/${_name}.jar")
	    SET (_jarpath "${_path2}/${_name}.jar")
	    BREAK ()
	  ENDIF (EXISTS "${_path2}/${_name}.jar")

	  # Finally, glob for version variants
	  FILE (GLOB _jarfiles "${_path2}/${_name}-*.jar")
	  IF (_jarfiles)
	    LIST (GET _jarfiles 0 _jarpath)
	    BREAK ()
	  ENDIF (_jarfiles)

	ENDFOREACH (_dir)
      ENDFOREACH (_name)
    ENDFOREACH (_path)

    # Cache the results
    SET (${JARVARNAME} "${_jarpath}" CACHE PATH "${ZFINDJAR_DOC}" FORCE)

  ENDIF (NOT ${JARVARNAME})

ENDMACRO (ZORBA_FIND_JAR)

MACRO (ZORBA_FIND_JNI)
  IF (UNIX AND NOT APPLE)
    # setting JAVA_HOME to bypass issues in FindJNI.cmake  
    FIND_FILE(_JAVA_BIN_LOC NAMES java PATHS ENV PATH)
    IF (_JAVA_BIN_LOC)
      GET_FILENAME_COMPONENT(_JAVA_BIN_LOC "${_JAVA_BIN_LOC}" REALPATH) 
      GET_FILENAME_COMPONENT(_JAVA_BIN_DIR ${_JAVA_BIN_LOC} PATH)
      GET_FILENAME_COMPONENT(_JAVA_HOME "${_JAVA_BIN_DIR}/../.." ABSOLUTE)
      MESSAGE(
        STATUS "Generated JAVA_HOME to support FindJNI.cmake: " 
        ${_JAVA_HOME})
      SET(ENV{JAVA_HOME} ${_JAVA_HOME})
    ENDIF()
  ENDIF()

  FIND_PACKAGE(JNI)
ENDMACRO (ZORBA_FIND_JNI)
