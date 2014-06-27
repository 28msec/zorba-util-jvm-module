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

#include "JavaVMSingleton.h"

#include <fstream>
#include <iostream>
#include <istream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <memory>

#include <zorba/zorba.h>
#include <zorba/util/fs_util.h>
#include <zorba/internal/unique_ptr.h>

namespace zorba {
namespace jvm {

JavaVMSingleton::JavaVMSingleton(const char* classPath, const char* javaLibPath)
{
  JavaVM *jvms;
  jsize nVMs;
  if ( JNI_GetCreatedJavaVMs(&jvms, 1, &nVMs) == 0 )
  {
    if (nVMs == 1)
    {
      JavaVM *jvm = jvms;
      JNIEnv *env;
      if( jvm->AttachCurrentThread((void **)&env, NULL) == 0)
      {
        init(jvm, env);
        return;
      }
    }
  }

  init(classPath, javaLibPath);
}

void JavaVMSingleton::init(const char* classPath, const char* javaLibPath)
{
  memset(&m_args, 0, sizeof(m_args));
  jint nOptions = NO_OF_JVM_OPTIONS;

  std::string classpathOption;
  std::ostringstream os;
  os << "-Djava.class.path=" << classPath;
  classpathOption = os.str();
  m_classPathOption = new char[classpathOption.size() + 1];
  memset(m_classPathOption, 0, sizeof(char) * (classpathOption.size() + 1));
  memcpy(m_classPathOption, classpathOption.c_str(), classpathOption.size() * sizeof(char));

  std::string lAwtArgStr = "-Djava.awt.headless=true";
  m_awtOption = new char[lAwtArgStr.size() + 1];
  memset(m_awtOption, 0, sizeof(char) * (lAwtArgStr.size() + 1));
  memcpy(m_awtOption, lAwtArgStr.c_str(), sizeof(char) * lAwtArgStr.size());
  m_awtOption[lAwtArgStr.size()] = 0;

  // javaLibPath are only base pathes, the full path will be computed at runtime in the Java class
  std::string jlpStr = "-Djava.library.path=" + std::string(javaLibPath);
  m_jlpOption = new char[jlpStr.size() + 1];
  memset(m_jlpOption, 0, sizeof(char) * (jlpStr.size() + 1));
  memcpy(m_jlpOption, jlpStr.c_str(), sizeof(char) * jlpStr.size());
  m_jlpOption[jlpStr.size()] = 0;

  m_options[0].optionString = m_classPathOption;
  m_options[0].extraInfo = NULL;
  m_options[1].optionString = m_awtOption;
  m_options[1].extraInfo = NULL;
  m_options[2].optionString = m_jlpOption;
  m_options[2].extraInfo = NULL;

  memset(&m_args, 0, sizeof(m_args));
  m_args.version  = JNI_VERSION_1_2;
  m_args.nOptions = nOptions;
  m_args.options  = m_options;
  m_args.ignoreUnrecognized = JNI_FALSE;

  if (JNI_CreateJavaVM(&m_vm, (void **)&m_env, &m_args) != JNI_OK)
  {
    throw VMOpenException();
  }
}

void JavaVMSingleton::init(JavaVM *jvm, JNIEnv *env)
{
  m_vm = jvm;
  m_env = env;
  m_classPathOption = NULL;
  m_awtOption = NULL;
  m_jlpOption = NULL;
}

JavaVMSingleton::~JavaVMSingleton()
{
  m_vm->DestroyJavaVM();
  delete[] m_awtOption;
  delete[] m_jlpOption;
  delete[] m_classPathOption;
}

JavaVMSingleton* JavaVMSingleton::getInstance(const char* classPath, const char* javaLibPath)
{
  static JavaVMSingleton s_instance(classPath, javaLibPath);
  return &s_instance;
}



JavaVMSingleton* JavaVMSingleton::getInstance(const zorba::StaticContext* aStaticContext)
{
  String cp = computeClassPath(aStaticContext);
  String lp = computeLibPath(aStaticContext);
  return getInstance(cp.c_str(), lp.c_str());
}

JavaVM* JavaVMSingleton::getVM()
{
  return m_vm;
}

JNIEnv* JavaVMSingleton::getEnv()
{
  return m_env;
}


String JavaVMSingleton::computeClassPath(const zorba::StaticContext* aStaticContext)
{
  String cp;

  // get classpath from global Properties
  PropertiesGlobal * properties = Zorba::getInstance(NULL)->getPropertiesGlobal();
  std::string globalClassPath;
  properties->getJVMClassPath(globalClassPath);
  cp += globalClassPath;

  std::vector<String> lCPV;
  aStaticContext->getFullLibPath(lCPV);

  for (std::vector<String>::iterator lIter = lCPV.begin();
       lIter != lCPV.end(); ++lIter)
  {
    // verify it contains a jars dir
    String jarsDir( *lIter );
    fs::append( jarsDir, "jars" );

    if ( fs::get_type( jarsDir ) == fs::directory )
    {
      fs::iterator itemIter( jarsDir );

      while ( itemIter.next() )
      {
        String itemFile( jarsDir );
        fs::append( itemFile, itemIter->name );
        if ( fs::get_type( itemFile ) == fs::file )
        {
          std::string suffix = "-classpath.txt";
          size_t found;
          found = itemFile.rfind(suffix);
          if (found!=std::string::npos &&
              found + suffix.length() == itemFile.length() )
          {
            std::auto_ptr<std::istream> pathFile;
            pathFile.reset(new std::ifstream (itemFile.c_str ()));
            if (!pathFile->good() || pathFile->eof() )
            {
              std::cerr << "file {" << itemFile << "} not found or not readable." << std::endl;
              throw itemFile;
            }

            // read file
            char line[1024];
            while( !pathFile->eof() && !pathFile->bad() && !pathFile->fail())
            {
              pathFile->getline(line, sizeof(line));
              std::string lineStr(line);

              if ( lineStr.size() > 0 ) {
                cp += fs::path_separator;
                cp += fs::normalize_path( lineStr, jarsDir );
              }
            }
          }
        }
      }
    }
  }

  properties->setJVMClassPath(cp.str());

  return cp;
}


String JavaVMSingleton::computeLibPath(const zorba::StaticContext* aStaticContext)
{
  String lp;
  std::vector<String> lCPV;

  aStaticContext->getFullLibPath(lCPV);
  for (std::vector<String>::iterator lIter = lCPV.begin();
       lIter != lCPV.end(); ++lIter)
  {
    String p( *lIter );
    lp += fs::path_separator;
    lp += p;
  }

  return lp;
}


} // namespace jvm
} // namespace zorba
/* vim:set et sw=2 ts=2: */
