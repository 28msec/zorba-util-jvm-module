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

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT (zorba_util-jvm_module)

IF (ZORBA_SUPPRESS_JAVA)
  MESSAGE (STATUS "ZORBA_SUPPRESS_JAVA is true - not searching for Java")
ELSE (ZORBA_SUPPRESS_JAVA)

  MESSAGE(STATUS "")
  MESSAGE(STATUS "--- Module:   util-jvm    ---")

  MESSAGE (STATUS "Looking for JNI")
  FIND_PACKAGE(JNI)

  MESSAGE (STATUS "Looking for Java")
  FIND_PACKAGE(Java)

  IF (JNI_FOUND)
      SET (UTIL_JVM_VERSION 1.0)

      #ENABLE_TESTING ()
      #INCLUDE (CTest)

      SET_CMAKE_MODULE_PATH ()

      FIND_PACKAGE (Zorba REQUIRED HINTS "${ZORBA_BUILD_DIR}")
      INCLUDE ("${Zorba_USE_FILE}")

      # Export this include directory by setting ZORBA_PROJECT_INCLUDE_DIRS
      SET (ZORBA_PROJECT_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include")
      INCLUDE_DIRECTORIES ("${ZORBA_PROJECT_INCLUDE_DIRS}")

      ADD_SUBDIRECTORY ("src")
      #ADD_TEST_DIRECTORY("${PROJECT_SOURCE_DIR}/test")
      DONE_DECLARING_ZORBA_URIS ()

  ELSE (JNI_FOUND)
    MESSAGE(STATUS "Building Zorba without util-jvm module.")
  ENDIF (JNI_FOUND)
  MESSAGE(STATUS "---")
  MESSAGE(STATUS "")
ENDIF(ZORBA_SUPPRESS_JAVA)

