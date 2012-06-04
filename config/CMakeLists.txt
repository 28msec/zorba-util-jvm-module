# Copyright 2006-2012 The FLWOR Foundation.
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

# We need to be able to install util-jvm and then build module projects
# against that. The "auto-config" approach Zorba provides does not
# support this, so we roll our own here. This should probably be
# re-thought in the future.

SET (UTILJVM_CMAKE_DIR "share/cmake/zorba_util-jvm_module")

# First, install the Use file. Doesn't need to be configured since it
# just contains a macro.
INSTALL (FILES UtilJavaUse.cmake DESTINATION "${UTILJVM_CMAKE_DIR}")

# Anyway, we set all the same variables to what they should be in the 
# install image, and re-config Zorba's ExternalModuleConfig.cmake.in.
SET (ZORBA_PROJECT_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include")
SET (ZORBA_PROJECT_USE_FILE "${CMAKE_INSTALL_PREFIX}/${UTILJVM_CMAKE_DIR}/UtilJavaUse.cmake")
GET_TARGET_PROPERTY(_libpath util-jvm LOCATION)
GET_FILENAME_COMPONENT(_libname "${_libpath}"  NAME)
SET (ZORBA_PROJECT_LIBRARIES "${CMAKE_INSTALL_PREFIX}/lib/${_libname}")

CONFIGURE_FILE("${Zorba_EXTERNALMODULECONFIG_FILE}"
  "${PROJECT_BINARY_DIR}/install/${PROJECT_NAME}Config.cmake" @ONLY)
INSTALL(FILES "${PROJECT_BINARY_DIR}/install/${PROJECT_NAME}Config.cmake"
  DESTINATION "${UTILJVM_CMAKE_DIR}")
