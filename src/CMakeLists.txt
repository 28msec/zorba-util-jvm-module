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

INCLUDE (CMakeJavaInformation )
MESSAGE(STATUS "Using Java API")
EXECUTE_PROCESS (
    COMMAND ${JAVA_RUNTIME} -version
    RESULT_VARIABLE JAVA_VERSION
    OUTPUT_QUIET
    ERROR_QUIET)
    
MESSAGE (STATUS "JNI found at    :${JAVA_INCLUDE_PATH}")
MESSAGE (STATUS "Java Library at :${JAVA_JVM_LIBRARY}")

INCLUDE_DIRECTORIES (${JAVA_INCLUDE_PATH} ${JAVA_INCLUDE_PATH2})
INCLUDE_DIRECTORIES (${JAVA_INCLUDE_PATH})

# For now, we still have a module, but it actually does nothing at all.
DECLARE_ZORBA_MODULE (URI "http://www.zorba-xquery.com/modules/util-jvm"
    VERSION 1.0 FILE "util-jvm.xq")

# The important stuff is the library, which we install in Zorba's
# default lib directory.
ADD_LIBRARY(util-jvm SHARED util-jvm.cpp JavaVMSingleton.cpp)
TARGET_LINK_LIBRARIES(util-jvm "${JAVA_JVM_LIBRARY}" zorba_${ZORBA_STORE_NAME})
INSTALL(TARGETS util-jvm 
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib${LIB_SUFFIX}
  ARCHIVE DESTINATION lib)

# MAC OS X only property
# this is required to make sure that the library has the correct install name after
# installation such that dependent modules always find it
SET_TARGET_PROPERTIES(util-jvm PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib" INSTALL_RPATH_USE_LINK_PATH TRUE)

# Set this in the parent scope so it will be put into our Config.cmake file.
# As with many other things, this doesn't work when the module is installed.
SET(ZORBA_PROJECT_LIBRARIES util-jvm PARENT_SCOPE)
