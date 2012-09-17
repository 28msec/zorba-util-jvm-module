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

#include <zorba/util/path.h>
#include <zorba/util/file.h>
#include <zorba/zorba.h>


namespace zorba { namespace jvm {
JavaVMSingleton* JavaVMSingleton::instance = NULL;

JavaVMSingleton::JavaVMSingleton(const char* classPath, const char* javaLibPath)
{
  //std::cout << "JavaVMSingleton::JavaVMSingleton classPath: " << classPath << "\n"; std::cout.flush();

  memset(&args, 0, sizeof(args));
  jint r;
  jint nOptions = NO_OF_JVM_OPTIONS;

  std::string classpathOption;
  std::ostringstream os;
  os << "-Djava.class.path=" << classPath;
  classpathOption = os.str();
  classPathOption = new char[classpathOption.size() + 1];
  memset(classPathOption, 0, sizeof(char) * (classpathOption.size() + 1));
  memcpy(classPathOption, classpathOption.c_str(), classpathOption.size() * sizeof(char));

  std::string lAwtArgStr = "-Djava.awt.headless=true";
  awtOption = new char[lAwtArgStr.size() + 1];
  memset(awtOption, 0, sizeof(char) * (lAwtArgStr.size() + 1));
  memcpy(awtOption, lAwtArgStr.c_str(), sizeof(char) * lAwtArgStr.size());
  awtOption[lAwtArgStr.size()] = 0;

  std::string jlpStr = "-Djava.library.path=/home/cezar/dev/repo/fpdf/fpdf/build/zorba_modules/zorba_util-jvm_module/src/:/home/cezar/dev/repo/fpdf/fpdf/build/LIB_PATH/com/zorba-xquery/www/modules/";
  jlpOption = new char[jlpStr.size() + 1];
  memset(jlpOption, 0, sizeof(char) * (jlpStr.size() + 1));
  memcpy(jlpOption, jlpStr.c_str(), sizeof(char) * jlpStr.size());
  jlpOption[jlpStr.size()] = 0;

  options[0].optionString = classPathOption;
  options[0].extraInfo = NULL;
  options[1].optionString = awtOption;
  options[1].extraInfo = NULL;
  options[2].optionString = jlpOption;
  options[2].extraInfo = NULL;

  memset(&args, 0, sizeof(args));
  args.version  = JNI_VERSION_1_2;
  args.nOptions = nOptions;
  args.options  = options;
  args.ignoreUnrecognized = JNI_FALSE;

  r = JNI_CreateJavaVM(&m_vm, (void **)&m_env, &args);
  if (r != JNI_OK) {
    throw VMOpenException();
  }
}

JavaVMSingleton::~JavaVMSingleton()
{
  if (instance) {
    delete instance;
    instance = NULL;
  }
  m_vm->DestroyJavaVM();
  if (awtOption)
    delete[] awtOption;
  if (classPathOption)
    delete[] classPathOption;
}

/*JavaVMSingleton* JavaVMSingleton::getInstance(const char* classPath)
{
  return getInstance(classPath, "");
}*/

JavaVMSingleton* JavaVMSingleton::getInstance(const char* classPath, const char* javaLibPath)
{
//#ifdef WIN32
//  // If pointer to instance of JavaVMSingleton exists (true) then return instance pointer else look for
//  // instance pointer in memory mapped pointer. If the instance pointer does not exist in
//  // memory mapped pointer, return a newly created pointer to an instance of Abc.

//  return instance ?
//     instance : (instance = (JavaVMSingleton*) MemoryMappedPointers::getPointer("JavaVMSingleton")) ?
//     instance : (instance = (JavaVMSingleton*) MemoryMappedPointers::createEntry("JavaVMSingleton",(void*)new JavaVMSingleton(classPath)));
//#else


  // If pointer to instance of JavaVMSingleton exists (true) then return instance pointer
  // else return a newly created pointer to an instance of JavaVMSingleton.
  if (instance == NULL)
  {
    JavaVM *jvms;
    jsize nVMs;
    if ( JNI_GetCreatedJavaVMs(&jvms, 1, &nVMs)==0 )
    {
      //std::cout << "Got JVMs " << nVMs << "\n"; std::cout.flush();
      if (nVMs == 1)
      {
        JavaVM *jvm = jvms;
        JNIEnv *env;
        if( jvm->AttachCurrentThread((void **)&env, NULL) ==0 )
        {
          // if there is a jvm opened already by a diffrent dynamic lib
          // make a singleton for this lib with that jvm
          instance = new JavaVMSingleton(jvm, env);
        }
      }
    }

    if (instance == NULL)
    {
      instance = new JavaVMSingleton(classPath, javaLibPath);
    }
  }

  return instance;
}



JavaVMSingleton* JavaVMSingleton::getInstance(const zorba::StaticContext* aStaticContext)
{
  if (instance == NULL)
  {
    String cp = computeClassPath(aStaticContext);
    String lp = computeLibPath(aStaticContext);
    return getInstance(cp.c_str(), lp.c_str());
  }

  return instance;
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

  String pathSeparator(filesystem_path::get_path_separator());
  String dirSeparator(filesystem_path::get_directory_separator());

  for (std::vector<String>::iterator lIter = lCPV.begin();
       lIter != lCPV.end(); ++lIter)
  {
    // verify it contains a jars dir
    const filesystem_path baseFsPath((*lIter).str());
    const filesystem_path jarsFsPath(std::string("jars"));
    filesystem_path jarsDirPath(baseFsPath, jarsFsPath);

    file jarsDir(jarsDirPath);

    if ( jarsDir.exists() && jarsDir.is_directory())
    {
      std::vector<std::string> list;
      jarsDir.lsdir(list);

      for (std::vector<std::string>::iterator itemIter = list.begin();
           itemIter != list.end(); ++itemIter)
      {
        filesystem_path itemLocalFS(*itemIter);
        filesystem_path itemFS(jarsDirPath, itemLocalFS);
        file itemFile(itemFS);
        if ( itemFile.exists() && itemFile.is_file() )
        {
          std::string itemName = itemFile.get_path();
          std::string suffix = "-classpath.txt";
          size_t found;
          found = itemName.rfind(suffix);
          if (found!=std::string::npos &&
              found + suffix.length() == itemName.length() )
          {
            std::auto_ptr<std::istream> pathFile;
            pathFile.reset(new std::ifstream (itemName.c_str ()));
            if (!pathFile->good() || pathFile->eof() )
            {
              std::cerr << "file {" << itemName << "} not found or not readable." << std::endl;
              throw itemName;
            }

            // read file
            char line[1024];
            while( !pathFile->eof() && !pathFile->bad() && !pathFile->fail())
            {
              pathFile->getline(line, sizeof(line));
              std::string lineStr(line);

              if ( lineStr.size() == 0 )
                continue;

              //std::cout << "line: '" << lineStr << "'" << std::endl; std::cout.flush();

              const std::string normalizedPath =
                  filesystem_path::normalize_path( lineStr, jarsDirPath.get_path());

              cp += pathSeparator + normalizedPath;
            }
          }
        }
      }
    }
  }

  properties->setJVMClassPath(cp.str());

  //std::cout << "JavaVMSingleton::computeClassPath: '" << cp << "'" << std::endl; std::cout.flush();
  return cp;
}


String JavaVMSingleton::computeLibPath(const zorba::StaticContext* aStaticContext)
{
  String lp;
  std::vector<String> lCPV;
  String pathSeparator(filesystem_path::get_path_separator());

  aStaticContext->getFullLibPath(lCPV);
  for (std::vector<String>::iterator lIter = lCPV.begin();
       lIter != lCPV.end(); ++lIter)
  {
    String p = *lIter;
    std::cout << "FullLibPath: '" << p << "'" << std::endl; std::cout.flush();
    lp += pathSeparator + p;
  }

  aStaticContext->getLibPath(lCPV);
  for (std::vector<String>::iterator lIter = lCPV.begin();
       lIter != lCPV.end(); ++lIter)
  {
    String p = *lIter;
    std::cout << "LibPath: '" << p << "'" << std::endl; std::cout.flush();
    lp += pathSeparator + p;
  }

  aStaticContext->getFullURIPath(lCPV);
  for (std::vector<String>::iterator lIter = lCPV.begin();
       lIter != lCPV.end(); ++lIter)
  {
    String p = *lIter;
    std::cout << "FullURIPath: '" << p << "'" << std::endl; std::cout.flush();
    lp += pathSeparator + p;
  }

  aStaticContext->getURIPath(lCPV);
  for (std::vector<String>::iterator lIter = lCPV.begin();
       lIter != lCPV.end(); ++lIter)
  {
    String p = *lIter;
    std::cout << "URIPath: '" << p << "'" << std::endl; std::cout.flush();
    lp += pathSeparator + p;
  }

  aStaticContext->getFullModulePaths(lCPV);
  for (std::vector<String>::iterator lIter = lCPV.begin();
       lIter != lCPV.end(); ++lIter)
  {
    String p = *lIter;
    std::cout << "FullModulePath: '" << p << "'" << std::endl; std::cout.flush();
    lp += pathSeparator + p;
  }

  aStaticContext->getModulePaths(lCPV);
  for (std::vector<String>::iterator lIter = lCPV.begin();
       lIter != lCPV.end(); ++lIter)
  {
    String p = *lIter;
    std::cout << "ModulePath: '" << p << "'" << std::endl; std::cout.flush();
    lp += pathSeparator + p;
  }


  std::cout << "JavaVMSingleton::computeLibPath: '" << lp << "'" << std::endl; std::cout.flush();
  return lp;
}


}} // namespace zorba, jvm


