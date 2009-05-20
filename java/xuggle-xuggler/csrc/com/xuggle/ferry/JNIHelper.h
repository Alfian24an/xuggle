/*
 * This file is part of Xuggler.
 * 
 * Xuggler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 * 
 * Xuggler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public
 * License along with Xuggler.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef JNIHELPER_H_
#define JNIHELPER_H_

#include <jni.h>
#include <list>

#include <com/xuggle/ferry/Ferry.h>
#include <inttypes.h>

namespace com { namespace xuggle { namespace ferry {
/**
 * The JNIHelper object contains a series of methods designed
 * to assist functions when running inside a Java JVM.
 *
 * There is only one JNIHelper object per shared library loaded.
 *
 */
class JNIHelper
  {
  public:
    typedef void (*CallbackFunc)(JavaVM* jvm, void* closure);

    static VS_API_FERRY JNIHelper* getHelper();

    // Static convenience methods
    inline static JavaVM* sGetVM()
    {
      return getHelper()->getVM();
    }
    inline static void sSetVM(JavaVM *jvm)
    {
      getHelper()->setVM(jvm);
    }
    inline static jint sGetJNIVersion()
    {
      return getHelper()->getJNIVersion();
    }
    inline static JNIEnv *sGetEnv()
    {
      return getHelper()->getEnv();
    }
    inline static void *sGetPointer(jobject pointerRef)
    {
      return getHelper()->getPointer(pointerRef);
    }
    inline static void *sSetPointer(jobject pointerRef, void *newVal)
    {
      return getHelper()->setPointer(pointerRef, newVal);
    }
    inline static jobject sNewLocalRef(jobject ref)
    {
      return getHelper()->newLocalRef(ref);
    }
    inline static void sDeleteLocalRef(jobject ref)
    {
      return getHelper()->deleteLocalRef(ref);
    }
    inline static jobject sNewGlobalRef(jobject ref)
    {
      return getHelper()->newGlobalRef(ref);
    }
    inline static void sDeleteGlobalRef(jobject ref)
    {
      return getHelper()->deleteGlobalRef(ref);
    }
    inline static jweak sNewWeakGlobalRef(jobject ref)
    {
      return getHelper()->newWeakGlobalRef(ref);
    }
    inline static void sDeleteWeakGlobalRef(jweak ref)
    {
      return getHelper()->deleteWeakGlobalRef(ref);
    }
    inline static void sRegisterInitializationCallback(
        CallbackFunc func, void* closure)
    {
      return getHelper()->registerInitializationCallback(func, closure);
    }
    inline static void sRegisterTerminationCallback(
        CallbackFunc func, void* closure)
    {
      return getHelper()->registerTerminationCallback(func, closure);
    }
    // Actual methods
    VS_API_FERRY JavaVM* getVM();
    VS_API_FERRY void setVM(JavaVM* jvm);

    VS_API_FERRY void * getPointer(jobject pointerRef);
    VS_API_FERRY void * setPointer(jobject pointerRef, void *newVal);

    VS_API_FERRY jobject newLocalRef(jobject ref);
    VS_API_FERRY void deleteLocalRef(jobject ref);
    VS_API_FERRY jobject newGlobalRef(jobject ref);
    VS_API_FERRY void deleteGlobalRef(jobject ref);
    VS_API_FERRY jweak newWeakGlobalRef(jobject ref);
    VS_API_FERRY void deleteWeakGlobalRef(jweak ref);

    VS_API_FERRY jint getJNIVersion();
    VS_API_FERRY JNIEnv* getEnv();

    /*
     * Register a callback function to be called when the JNIHelper
     * gets a JavaVM context passed to it.  The JNIHelper will also
     * pass back to the callback the closure value registered with
     * the callback.
     *
     * NOTE: If the JNIHelper already has a JavaVM registered, it will
     * call the callback WHILE REGISTERING.  Be aware of this semantic
     * and deal accordingly.
     */
    VS_API_FERRY void registerInitializationCallback(CallbackFunc, void* closure);
    VS_API_FERRY void registerTerminationCallback(CallbackFunc, void*closure);

    VS_API_FERRY virtual ~JNIHelper();

    /*
     * This method is not meant for general calling.  A program that
     * is DAMN SURE it never needs to use the helper again can
     * call it, and we will delete memory.
     *
     * Really, the only use case for this is testing that our memory
     * clean-up code does work.
     */
    VS_API_FERRY static void shutdownHelper();
    /*
     * This needs to be public for the stupid auto_ptr to work,
     * but don't go creating one of these and expecting things to
     * work.
     */
    VS_API_FERRY JNIHelper();
    
    VS_API_FERRY void throwOutOfMemoryError();

  private:

    JavaVM* mCachedVM;
    jint mVersion;

    jweak mJNIPointerReference_class;
    jmethodID mJNIPointerReference_setPointer_mid;
    jmethodID mJNIPointerReference_getPointer_mid;
    jthrowable mOutOfMemoryErrorSingleton;

    struct CallbackHelper {
      CallbackFunc mCallback;
      void* mClosure;
    };
    static void addCallback(std::list<CallbackHelper*>*, CallbackFunc, void*closure);
    static void processCallbacks(std::list<CallbackHelper*>*, JavaVM*, bool execFunc=true);
    std::list<CallbackHelper*> mInitializationCallbacks;
    std::list<CallbackHelper*> mTerminationCallbacks;

    void waitForDebugger(JNIEnv*);

    static JNIHelper* sSingleton;
    static volatile bool sDebuggerAttached;
  };
}}}

#endif /*JNIHELPER_H_*/
