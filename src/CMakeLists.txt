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

DECLARE_ZORBA_MODULE (URI "http://www.zorba-xquery.com/modules/util-jvm"
    VERSION 1.0 FILE "util-jvm.xq" LINK_LIBRARIES "${JAVA_JVM_LIBRARY}")

