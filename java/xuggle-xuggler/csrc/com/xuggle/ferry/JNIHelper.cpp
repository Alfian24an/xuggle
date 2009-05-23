/*
 * Copyright (c) 2008, 2009 by Xuggle Incorporated.  All rights reserved.
 * 
 * This file is part of Xuggler.
 * 
 * You can redistribute Xuggler and/or modify it under the terms of the GNU
 * Affero General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any
 * later version.
 * 
 * Xuggler is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public
 * License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with Xuggler.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "JNIHelper.h"

#include <iostream>

#include <stdexcept>

// declare a signature for the memory manager
extern void VSJNI_MemoryManagerInit(JavaVM* aJVM);

namespace com { namespace xuggle { namespace ferry {
  JNIHelper* JNIHelper::sSingleton = 0;
  volatile bool JNIHelper::sDebuggerAttached = false;

  JNIHelper*
  JNIHelper :: getHelper()
  {
    JNIHelper *retval = 0;
    retval = sSingleton;
    if (!retval) {
      retval = new JNIHelper();
      sSingleton = retval;
    }
    return retval;
  }

  void
  JNIHelper :: shutdownHelper()
  {
    if (sSingleton) {
      delete sSingleton;
      sSingleton = 0;
    }
  }

  JNIHelper :: JNIHelper()
  {
    mCachedVM = 0;
    mVersion = JNI_VERSION_1_2;
    mJNIPointerReference_class = 0;
    // Now, look for our get and set methods.
    mJNIPointerReference_setPointer_mid = 0;
    mJNIPointerReference_getPointer_mid = 0;
    mOutOfMemoryErrorSingleton = 0;
    
    mThread_class = 0;
    mThread_isInterrupted_mid = 0;
    mThread_currentThread_mid = 0;
  }

  void
  JNIHelper :: addCallback(std::list<CallbackHelper*>* list,
      CallbackFunc callback,
      void *closure)
  {
    CallbackHelper *helper = new CallbackHelper;
    if (helper)
    {
      helper->mCallback = callback;
      helper->mClosure = closure;
      list->push_back(helper);
      helper = 0;
    }
  }

  void
  JNIHelper :: processCallbacks(std::list<CallbackHelper*>* list,
      JavaVM* vm,
      bool execFunc)
  {
    while (!list->empty())
    {
      // then call them.
      CallbackHelper *helper = list->front();
      if (helper) {
        if (helper->mCallback && execFunc)
          helper->mCallback(vm, helper->mClosure);
        delete helper;
        helper = 0;
      }
      // now delete the helper ref from the front
      list->pop_front();
    }
  }

  JNIHelper :: ~JNIHelper()
  {
    JNIEnv *env;

    processCallbacks(&mInitializationCallbacks, 0, false);
    processCallbacks(&mTerminationCallbacks, mCachedVM, true);

    env = this->getEnv();
    if (env)
    {
      if (mOutOfMemoryErrorSingleton)
      {
        env->DeleteGlobalRef(mOutOfMemoryErrorSingleton);
        mOutOfMemoryErrorSingleton = 0;
      }
      if (mJNIPointerReference_class)
      {
        // Tell the JVM we're done with it.
        env->DeleteWeakGlobalRef(mJNIPointerReference_class);
        mJNIPointerReference_class = 0;
      }
      if (mThread_class)
      {
        env->DeleteWeakGlobalRef(mThread_class);
        mThread_class = 0;
      }
    }
    mCachedVM = 0;
  }

  void
  JNIHelper :: setVM(JavaVM * jvm)
  {
    if (!mCachedVM)
    {
      mCachedVM = jvm;

      // Now, let's cache the commonly used classes.
      JNIEnv *env=this->getEnv();

      // It can be helpful to attach a GDB or Debugger to
      // a Running JVM.  This function attempts to see if that's
      // required.
      waitForDebugger(env);

      jclass cls=0;

      // let's set up a singleton out of memory error for potential
      // reuse later
      cls = env->FindClass("java/lang/OutOfMemoryError");
      if (!cls || env->ExceptionCheck())
        return;
      
      jmethodID constructor = env->GetMethodID(cls, "<init>", "()V");
      if (!constructor || env->ExceptionCheck())
        return;
      
      jthrowable exception=static_cast<jthrowable>(env->NewObject(cls, constructor));
      if (!exception || env->ExceptionCheck())
        return;
      
      mOutOfMemoryErrorSingleton = static_cast<jthrowable>(env->NewGlobalRef(exception));
      if (!mOutOfMemoryErrorSingleton || env->ExceptionCheck())
        return;
      
      env->DeleteLocalRef(cls);
      cls = env->FindClass("com/xuggle/ferry/JNIPointerReference");
      if (!cls || env->ExceptionCheck())
        return;
      // Keep a reference around
      mJNIPointerReference_class=env->NewWeakGlobalRef(cls);
      if (!mJNIPointerReference_class || env->ExceptionCheck())
        return;
      
      // Now, look for our get and set mehods.
      mJNIPointerReference_setPointer_mid = env->GetMethodID(cls, "setPointer",
          "(J)J");
      if (!mJNIPointerReference_setPointer_mid || env->ExceptionCheck())
        return;

      mJNIPointerReference_getPointer_mid = env->GetMethodID(cls, "getPointer",
          "()J");
      if (!mJNIPointerReference_getPointer_mid || env->ExceptionCheck())
        return;

      env->DeleteLocalRef(cls);

      cls = env->FindClass("java/lang/Thread");
      if (!cls || env->ExceptionCheck())
        return;
      // Keep a reference around
      mThread_class=env->NewWeakGlobalRef(cls);
      if (!mThread_class || env->ExceptionCheck())
        return;
      
      // now look for the isInterrupted method
      mThread_currentThread_mid = env->GetStaticMethodID(cls,
          "currentThread", "()Ljava/lang/Thread;");
      if (!mThread_currentThread_mid || env->ExceptionCheck())
        return;
      
      mThread_isInterrupted_mid = env->GetMethodID(cls,
          "isInterrupted", "()Z");
      if (!mThread_isInterrupted_mid || env->ExceptionCheck())
        return;
      
      env->DeleteLocalRef(cls);

      // Are there any pending callbacks?
      processCallbacks(&mInitializationCallbacks, mCachedVM, true);

      // And Initialize the JNI Memory manager;
      // This means that any callbacks will NOT use the
      // JVM for memory management.
      VSJNI_MemoryManagerInit(mCachedVM);

    }
  }

  void
  JNIHelper :: waitForDebugger(JNIEnv* env)
  {
    // Find the System class.
    jclass cls = 0;
    jstring jDebugKey = 0;
    jstring jDebugVal = 0;
    const char* debugKey = "com.xuggle.ferry.WaitForDebugger";
    const char* debugVal = 0;

    try
    {
      cls = env->FindClass("java/lang/System");
      if (!cls)
        throw std::runtime_error("could not find System class");

      jmethodID getProperty = env->GetStaticMethodID(cls, "getProperty",
          "(Ljava/lang/String;)Ljava/lang/String;");
      if (!getProperty)
        throw std::runtime_error("could not find getProperty class");

      jDebugKey = env->NewStringUTF(debugKey);
      if (!jDebugKey)
        throw std::runtime_error("could not create java string for the debug key");

      jDebugVal = (jstring) env->CallStaticObjectMethod(cls, getProperty, jDebugKey);
      if (jDebugVal)
      {
        debugVal = env->GetStringUTFChars(jDebugVal, 0);
        if (debugVal && *debugVal)
        {
          // It's set to some out put value.  Sit and spin waiting
          // for a debugger.
          std::cerr << "Waiting for a debugger.  Please attach and set \"JNIHelper::sDebuggerAttached\" to true"
            << " (" << __FILE__ << ":" << __LINE__+1 <<")" << std::endl;
          while (!sDebuggerAttached) ;
        }
      }
    }
    catch (std::exception e)
    {
      std::cerr << "Got exception while checking for debugger: "
        << e.what()
        << std::endl;
    }

    // And do the clean up here.
    if (jDebugKey)
      env->DeleteLocalRef(jDebugKey);

    if (jDebugVal && debugVal)
      env->ReleaseStringUTFChars(jDebugVal, debugVal);

    if (cls)
      env->DeleteLocalRef(cls);
  }

  void
  JNIHelper :: registerInitializationCallback(
      CallbackFunc callback,
      void * closure)
  {
    if (callback)
    {
      if (!mCachedVM)
      {
        // we don't have a callback; so save the function
        // call for later.
        addCallback(&mInitializationCallbacks, callback, closure);
      }
      else
      {
       // We have a VM already; just callback immediately.
       callback(mCachedVM, closure);
      }
    }
  }

  void
  JNIHelper :: registerTerminationCallback(
      CallbackFunc callback,
      void * closure)
  {
    if (callback)
    {
      addCallback(&mTerminationCallbacks, callback, closure);
    }
  }

  JavaVM *
  JNIHelper :: getVM()
  {
    return mCachedVM;
  }

  void *
  JNIHelper :: getPointer(jobject pointerRef)
  {
    JNIEnv *env = this->getEnv();
    if (!env)
      return 0;

    jlong pointerVal = env->CallLongMethod(pointerRef,
        mJNIPointerReference_getPointer_mid);

    /*
     * What's going on here is inherently unsafe, but is designed
     * to pass pointer values into Java.  It is structured like this
     * to compile without warning on as many OSes as possible
     *
     * It'll only work for 64-bit or lower os'es (what you complaining
     * about -- you got a 65-bit OS you're chomping to try out?).
     *
     * Now stop looking.  Go back to your day job.
     */
    void *retval = (void*)(size_t)pointerVal;

    return retval;
  }

  void *
  JNIHelper :: setPointer(jobject pointerRef, void *newVal)
  {
    JNIEnv *env = this->getEnv();
    if (!env)
      return 0;
    if (!pointerRef)
      return 0;

    jlong newPointerVal = (jlong)(long)newVal;

    jlong pointerVal = env->CallLongMethod(pointerRef,
        mJNIPointerReference_setPointer_mid, newPointerVal);

    void *retval = (void*)(size_t)pointerVal;

    return retval;
  }

  JNIEnv *
  JNIHelper :: getEnv()
  {
    JNIEnv *env=0;
    if (mCachedVM)
      mCachedVM->GetEnv((void**)(void*)&env, mVersion);
    return env;
  }

  jint
  JNIHelper :: getJNIVersion()
  {
    return mVersion;
  }

  jobject
  JNIHelper :: newLocalRef(jobject ref)
  {
    jobject retval = 0;
    JNIEnv *env = this->getEnv();
    if (!env)
      return 0;
    retval = env->NewLocalRef(ref);
    if (!retval)
      throw std::runtime_error("could not get JVM LocalRef");
    if (env->ExceptionCheck()) {
      env->DeleteLocalRef(retval);
      throw std::runtime_error("could not get JVM LocalRef");
    }
    return retval;
  }

  void
  JNIHelper :: deleteLocalRef(jobject ref)
  {
    JNIEnv *env = this->getEnv();
    if (!env)
      throw std::runtime_error("attempted to delete LocalRef without JVM");
    env->DeleteLocalRef(ref);
  }

  jobject
  JNIHelper :: newGlobalRef(jobject ref)
  {
    jobject retval = 0;
    JNIEnv *env = this->getEnv();
    if (!env)
      return 0;
    retval = env->NewGlobalRef(ref);
    if (!retval)
      throw std::runtime_error("could not get JVM GlobalRef");
    if (env->ExceptionCheck())
    {
      env->DeleteGlobalRef(retval);
      throw std::runtime_error("could not get JVM GlobalRef");
    }
    return retval;
  }

  void
  JNIHelper :: deleteGlobalRef(jobject ref)
  {
    JNIEnv *env = this->getEnv();
    if (!env)
      throw std::runtime_error("attempted to delete GlobalRef without JVM");
    env->DeleteGlobalRef(ref);
  }

  jweak
  JNIHelper :: newWeakGlobalRef(jobject ref)
  {
    if (!ref)
      return 0;
    JNIEnv *env = this->getEnv();
    if (!env)
      return 0;
    jweak retval = 0;
    retval = env->NewWeakGlobalRef(ref);
    if (!retval)
      throw std::runtime_error("could not get JVM WeakGlobal ref");
    if (env->ExceptionCheck())
    {
      env->DeleteWeakGlobalRef(retval);
      throw std::runtime_error("could not get JVM WeakGlobal ref");
    }
    return retval;
  }

  void
  JNIHelper :: deleteWeakGlobalRef(jweak ref)
  {
    if (!ref)
      return;
    JNIEnv *env = this->getEnv();
    if (!env)
      throw std::runtime_error("attempted to delete WeakGlobalRef without JVM");
    env->DeleteWeakGlobalRef(ref);
  }

  void
  JNIHelper :: throwOutOfMemoryError()
  {
    /**
     * Be VERY careful in this function; it gets call when we're low on 
     * memory so you must avoid allocating memory, as that is incredibly
     * likely to fail -- and will do so on the most inconvenient OS more
     * often than others.
     */
    JNIEnv *env = this->getEnv();
    if (!env)
      return;
    if (!env->ExceptionCheck()) {
      // don't override a pending exception
      if (!mOutOfMemoryErrorSingleton)
        return;
      env->Throw(mOutOfMemoryErrorSingleton);
    }
  }
  
  int32_t
  JNIHelper :: isInterrupted()
  {
    JNIEnv * env = this->getEnv();
    if (!env)
      return 0;
    if (env->ExceptionCheck())
      return 1; // a pending exception interrupts us
    if (!mThread_class ||
        !mThread_isInterrupted_mid ||
        !mThread_currentThread_mid)
      // this is an error if these are not set up, but we're
      // going to assume no interrupt then.
      return 0;
    jobject thread = env->CallStaticObjectMethod(
        static_cast<jclass>(mThread_class),
        mThread_currentThread_mid);
    if (!thread || env->ExceptionCheck())
      return 1;
    jboolean result = env->CallBooleanMethod(thread,
        mThread_isInterrupted_mid);
    env->DeleteLocalRef(thread);
    if (env->ExceptionCheck())
      result = false;
    if (result)
      return 1;
    return 0; 
  }

}}}
