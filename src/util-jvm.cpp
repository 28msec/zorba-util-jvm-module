/*
 * Copyright 2006-2008 The FLWOR Foundation.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <zorba/diagnostic_list.h>
#include <zorba/empty_sequence.h>
#include <zorba/external_module.h>
#include <zorba/function.h>
#include <zorba/item_factory.h>
#include <zorba/serializer.h>
#include <zorba/singleton_item_sequence.h>
#include <zorba/user_exception.h>
#include <zorba/util/base64_util.h>
#include <zorba/vector_item_sequence.h>
#include <zorba/zorba.h>

#include "JavaVMSingleton.h"

#define UTILJVM_MODULE_NAMESPACE "http://www.zorba-xquery.com/modules/util-jvm"
#define UTILJVM_OPTIONS_NAMESPACE "http://www.zorba-xquery.com/modules/util-jvm/util-jvm-options"

class JavaException {
};

#define CHECK_EXCEPTION(env)  if ((lException = env->ExceptionOccurred())) throw JavaException()

namespace zorba
{
namespace utiljvm
{

class UtilJvmModule;

class UtilJvmModule : public ExternalModule {
private:

public:
  UtilJvmModule() {}

  virtual ~UtilJvmModule() {}

  virtual String getURI() const
  {
    return UTILJVM_MODULE_NAMESPACE;
  }

  virtual ExternalFunction* getExternalFunction(const String& localName);

  virtual void destroy()
  {
    delete this;
  }
};

ExternalFunction* UtilJvmModule::getExternalFunction(const String& localName)
{
  return 0;
}

}}; // namespace zorba, utiljvm

#ifdef WIN32
#  define DLL_EXPORT __declspec(dllexport)
#else
#  define DLL_EXPORT __attribute__ ((visibility("default")))
#endif

extern "C" DLL_EXPORT zorba::ExternalModule* createModule()
{
  return new zorba::utiljvm::UtilJvmModule();
}
