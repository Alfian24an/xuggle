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

/*
 * JNIMemoryManagement.cpp
 *
 *  Created on: Aug 27, 2008
 *      Author: aclarke
 */
#include "JNIMemoryManager.h"
// for std::bad_alloc
#include <stdexcept>

#include <new>

// For free/malloc
#include <cstdlib>
#include <cstring>
#include <climits>

#include <jni.h>

//#define VSJNI_MEMMANAGER_DEBUG 1
#ifdef VSJNI_MEMMANAGER_DEBUG
#include <cstdio>
#endif

#include <com/xuggle/ferry/config.h>

#include "Ferry.h"
#include "RefCounted.h"

static void *
VSJNI_malloc(jobject obj, size_t requested_size);
static void
VSJNI_free(void * mem);
static void *
VSJNI_alignMemory(void *);
static void *
VSJNI_unalignMemory(void*);
static int VSJNI_ALIGNMENT_BOUNDARY = 16; // Must be power of two

enum VSJNIMemoryModel
{
  JAVA_BYTE_ARRAYS = 0,
  JAVA_DIRECT_BUFFERS = 1,
  NATIVE_BUFFERS = 2,
  NATIVE_BUFFERS_WITH_JAVA_NOTIFICATION = 3,
};

static enum VSJNIMemoryModel sVSJNI_IsMirroringNativeMemoryInJVM =
#ifdef VSJNI_USE_JVM_FOR_MEMMANAGEMENT
    JAVA_BYTE_ARRAYS;
#else
NATIVE_BUFFERS;
#endif

namespace com
{
namespace xuggle
{
namespace ferry
{

void *
JNIMemoryManager::malloc(size_t requested_size)
{
  return JNIMemoryManager::malloc(0, requested_size);
}

void *
JNIMemoryManager::malloc(void* obj, size_t requested_size)
{
  void * retval = VSJNI_malloc(static_cast<jobject> (obj), requested_size);
  return retval;
}

void
JNIMemoryManager::free(void* mem)
{
  VSJNI_free(mem);
}

}
}
}

static JavaVM* sCachedJVM = 0;
#ifdef VSJNI_USE_JVM_FOR_MEMMANAGEMENT
static JNIEnv*
VSJNI_getEnv();
static jclass sByteBufferReferenceClass = 0;
static jmethodID sByteBufferAllocateDirectMethod = 0;
static jclass sJNIMemoryAllocatorClass = 0;
static jmethodID sJNIMemoryAllocatorMallocMethod = 0;
static jmethodID sJNIMemoryAllocatorFreeMethod = 0;
#endif // VSJNI_USE_JVM_FOR_MEMMANAGEMENT
/*
 * This method is called by the JNIHelper when it loads up and
 * BEFORE it tries to allocate memory.
 *
 * WARNING: DO NOT CALL JNIHELPER METHODS HERE DIRECTLY; THEY
 * RELY ON CREATING A JNIHELPER OBJECT WHICH USES NEW!
 */
void
VSJNI_MemoryManagerInit(JavaVM* aJVM)
{
  sCachedJVM = aJVM;

#ifdef VSJNI_USE_JVM_FOR_MEMMANAGEMENT
  /**
   * Let's get handles to the JNI calls we'll need.
   */
  try
  {
    // Now, let's cache the commonly used classes.
    JNIEnv *env = VSJNI_getEnv();
    if (!env)
      throw std::runtime_error("could not find environment");

    jclass cls = env->FindClass("java/nio/ByteBuffer");
    if (!cls)
      throw std::runtime_error("could not find java.nio.ByteBuffer class");

    sByteBufferReferenceClass = (jclass) env->NewWeakGlobalRef(cls);
    if (!sByteBufferReferenceClass)
      throw std::runtime_error("could not get weak reference for class");

    sByteBufferAllocateDirectMethod = env->GetStaticMethodID(cls,
        "allocateDirect", "(I)Ljava/nio/ByteBuffer;");
    if (!sByteBufferAllocateDirectMethod)
      throw std::runtime_error(
          "could not find allocateDirect(int) method in java.nio.ByteBuffer");
    env->DeleteLocalRef(cls);
    if (env->ExceptionCheck())
      throw std::runtime_error("got exception in jni");

    cls = env->FindClass("com/xuggle/ferry/JNIMemoryAllocator");
    if (!cls)
      throw std::runtime_error(
          "could not find com.xuggle.ferry.JNIMemoryAllocatorclass");

    sJNIMemoryAllocatorClass = (jclass) env->NewWeakGlobalRef(cls);
    if (!sJNIMemoryAllocatorClass)
      throw std::runtime_error("could not get weak reference for class");

    sJNIMemoryAllocatorMallocMethod = env->GetMethodID(cls, "malloc", "(I)[B");
    if (!sJNIMemoryAllocatorMallocMethod)
      throw std::runtime_error(
          "could not find malloc(int) method in com.xuggle.ferry.JNIMemoryAllocator");
    sJNIMemoryAllocatorFreeMethod = env->GetMethodID(cls, "free", "([B)V");
    if (!sJNIMemoryAllocatorFreeMethod)
      throw std::runtime_error(
          "could not find free(byte[]) method in com.xuggle.ferry.JNIMemoryAllocator");
  }
  catch (std::exception e)
  {
    // DON'T HAVE JAVA do memory management.
    sCachedJVM = 0;
#ifdef VSJNI_MEMMANAGER_DEBUG
    fprintf(stderr,
        "got exception initializing jvm; using stdlib for memory allocation\n");
#endif
  }
#else
#endif // VSJNI_USE_JVM_FOR_MEMMANGEMENT
}

#ifdef VSJNI_USE_JVM_FOR_MEMMANAGEMENT
static JNIEnv*
VSJNI_getEnv()
{
  JNIEnv * env = 0;
  if (sCachedJVM)
  {
    jint retval = sCachedJVM->GetEnv((void**) (void*) &env, JNI_VERSION_1_2);
    if (retval == JNI_EDETACHED)
      throw std::runtime_error("not attached to JVM");

    if (retval == JNI_EVERSION)
      throw std::runtime_error("Java v1.2 not supported");

  }
  return env;
}

struct VSJNI_AllocationHeader
{
  /**
   * A Global Reference to mMem that tells the JVM we're not done
   * yet.  If this is 0, we assume that malloc()/free() should
   * be used instead to free.
   */
  jobject mRef;
  /**
   * A Global Reference to the object that we actually go the
   * mRef backing object from
   */
  jobject mAllocator;
  /**
   * Yes it's a shame, but another four bytes go to the model.
   */
  enum VSJNIMemoryModel mModel;
};

/**
 * Allocates a new block of memory.
 *
 * The memory allocated is always aligned on a 16-byte (i.e. 64bit)
 * boundary as several Ffmpeg optimizations depend on that.
 *
 * @param requested_size The amount of memory requested
 */

static void *
VS_JNI_malloc_native(JNIEnv *, jobject, size_t requested_size)
{
  void *retval = 0;
  void *buffer = 0;

  // We're not in a JVM, so use malloc/free instead
  buffer = malloc((int) requested_size + sizeof(VSJNI_AllocationHeader)
      + VSJNI_ALIGNMENT_BOUNDARY);
  VSJNI_AllocationHeader *header = (VSJNI_AllocationHeader*) buffer;
  if (!header)
    throw std::bad_alloc();
  // initialize the header to 0; that way later on delete we can tell
  // if this was JVMed or Malloced.
  memset(header, 0, sizeof(VSJNI_AllocationHeader));
  header->mModel = NATIVE_BUFFERS;

  retval = (void*) ((char*) header + sizeof(VSJNI_AllocationHeader));
  return retval;
}

static void *
VS_JNI_malloc_javaDirectBufferBacked(JNIEnv *env, jobject,
    size_t requested_size)
{
  void *retval = 0;
  void *buffer = 0;
  // We allocate a byte array for the actual memory
  jlong size = requested_size + sizeof(VSJNI_AllocationHeader)
      + VSJNI_ALIGNMENT_BOUNDARY;

  jobject bytearray = env->CallStaticObjectMethod(sByteBufferReferenceClass,
      sByteBufferAllocateDirectMethod, size);

  // if JVM didn't like that, return bad_alloc(); when we return all the way back to
  // JVM the Exception in Java will still exist; even if someone else catches
  // std::bad_alloc()
  if (env->ExceptionCheck())
    throw std::bad_alloc();

  jlong availableCapacity = env->GetDirectBufferCapacity(bytearray);
  if (env->ExceptionCheck())
    throw std::bad_alloc();
  if (availableCapacity < size)
    throw std::bad_alloc();

  buffer = env->GetDirectBufferAddress(bytearray);
  if (!buffer)
    throw std::bad_alloc();

  // We're going to take up the first few (usually 4 on 32bit
  // and 8 on 64 bit machines)
  // bytes of this for our header, so be aware if you use this 
  // for really small objects
  // that the overhead is QUITE high.
  VSJNI_AllocationHeader* header = (VSJNI_AllocationHeader*) buffer;
  memset(header, 0, sizeof(VSJNI_AllocationHeader));

  // And tell the JVM that it can't cleanup the bytearray yet; we've got
  // things to do and places to go.
  header->mRef = static_cast<jobject> (env->NewGlobalRef(bytearray));
  if (!header->mRef)
    throw std::bad_alloc();
  header->mModel = JAVA_DIRECT_BUFFERS;

  // But be nice and delete the local ref we had since we now have a
  // stronger reference.
  env->DeleteLocalRef(bytearray);
  if (env->ExceptionCheck())
    throw std::bad_alloc();
  // Finally, return the buffer, but skip past our header
  retval = (void*) ((char*) buffer + sizeof(VSJNI_AllocationHeader));
  return retval;
}

static void *
VS_JNI_malloc_javaByteBacked(JNIEnv* env, jobject obj, size_t requested_size)
{
  void* retval = 0;
  void* buffer = 0;
  // make sure we don't already have a pending exception
  if (env->ExceptionCheck())
    throw std::bad_alloc();

  jbyteArray bytearray = 0;
  if (obj)
  {
    // We allocate a byte array for the actual memory
    bytearray = static_cast<jbyteArray> (env->CallObjectMethod(obj,
        sJNIMemoryAllocatorMallocMethod, requested_size
            + sizeof(VSJNI_AllocationHeader) + VSJNI_ALIGNMENT_BOUNDARY));
  }
  else
  {
    // We allocate a byte array for the actual memory
    bytearray = env->NewByteArray(requested_size
        + sizeof(VSJNI_AllocationHeader) + VSJNI_ALIGNMENT_BOUNDARY);
  }

  // if JVM didn't like that, return bad_alloc(); when we 
  // return all the way back to
  // JVM the Exception in Java will still exist; even if someone else catches
  // std::bad_alloc()
  if (!bytearray)
    throw std::bad_alloc();
  if (env->ExceptionCheck())
    throw std::bad_alloc();

  // Now this is the actual memory pointed to by the java byte array
  // I use a void* buffer here so I can peak more easily in a debugger
  buffer = (void*) (env->GetByteArrayElements(bytearray, 0));
  if (env->ExceptionCheck())
    throw std::bad_alloc();

  // Technically this should never occur (i.e. if buffer is null
  // then a Java exception should have occurred).  still, we'll
  // be doubly safe for fun
  if (!buffer)
    throw std::bad_alloc();

  VSJNI_AllocationHeader* header = (VSJNI_AllocationHeader*) buffer;
  memset(header, 0, sizeof(VSJNI_AllocationHeader));

  // And tell the JVM that it can't cleanup the bytearray yet; we've got
  // things to do and places to go.
  header->mRef = env->NewGlobalRef(bytearray);
  if (!header->mRef)
    throw std::bad_alloc();
  header->mAllocator = 0;
  if (obj)
  {
    header->mAllocator = env->NewGlobalRef(obj);
    if (!header->mAllocator)
      throw std::bad_alloc();
  }
  header->mModel = JAVA_BYTE_ARRAYS;

  // But be nice and delete the local ref we had since we now have a
  // stronger reference.
  env->DeleteLocalRef(bytearray);
  if (env->ExceptionCheck())
    throw std::bad_alloc();

  // Finally, return the buffer, but skip past our header
  retval = (void*) ((char*) buffer + sizeof(VSJNI_AllocationHeader));
  return retval;
}
static void *
VS_JNI_malloc_nativeWithSmallJavaByteBacked(JNIEnv* env, jobject obj,
    size_t requested_size)
{
  void* retval = 0;
  void* buffer = 0;

  // make sure we don't already have a pending exception
  if (env->ExceptionCheck())
    throw std::bad_alloc();

  jbyteArray bytearray = 0;
  // We allocate a byte array for the actual memory
  bytearray = env->NewByteArray(requested_size);

  // if JVM didn't like that, return bad_alloc(); when we 
  // return all the way back to
  // JVM the Exception in Java will still exist; even if someone else catches
  // std::bad_alloc()
  if (!bytearray)
    throw std::bad_alloc();
  if (env->ExceptionCheck())
    throw std::bad_alloc();

  buffer = VS_JNI_malloc_native(env, obj, requested_size);
  if (!buffer)
    throw std::bad_alloc();

  if (!buffer)
    throw std::bad_alloc();
  VSJNI_AllocationHeader *header = (VSJNI_AllocationHeader*) ((char*) buffer
      - sizeof(VSJNI_AllocationHeader));

  // Here, oddly enough, we're going to set 0, and not remember the bytearray
  // the purpose of this model is to ask Java to always allocate our bytes,
  // so that it does mini-collections more often.
  header->mRef = 0;
  header->mAllocator = 0;
  header->mModel = NATIVE_BUFFERS_WITH_JAVA_NOTIFICATION;

  // Now, tell Java to delete that byte array it so handily made
  // for us.
  env->DeleteLocalRef(bytearray);
  if (env->ExceptionCheck())
    throw std::bad_alloc();

  // Finally, return the buffer, but skip past our header
  retval = buffer;
  return retval;
}

static void *
VSJNI_malloc(jobject obj, size_t requested_size)
{
  try
  {
    void* retval = 0;
    JNIEnv* env = 0;

    if ((int) requested_size > INT_MAX - VSJNI_ALIGNMENT_BOUNDARY)
      // we need 16 byte clearance; and only support up to 4GBs.  Sorry folks, if
      // you need more than 4GB of contiguous memory, it's not us.
      return 0;

    (void) obj;
    env = VSJNI_getEnv();
    enum VSJNIMemoryModel model = sVSJNI_IsMirroringNativeMemoryInJVM;
    if (!env)
      model = NATIVE_BUFFERS;
    switch (model)
    {
      case JAVA_BYTE_ARRAYS:
        retval = VS_JNI_malloc_javaByteBacked(env, obj, requested_size);
        break;
      case JAVA_DIRECT_BUFFERS:
        retval = VS_JNI_malloc_javaDirectBufferBacked(env, obj, requested_size);
        break;
      case NATIVE_BUFFERS_WITH_JAVA_NOTIFICATION:
        retval = VS_JNI_malloc_nativeWithSmallJavaByteBacked(env, obj,
            requested_size);
        break;
      case NATIVE_BUFFERS:
        /** fall through */
      default:
        retval = VS_JNI_malloc_native(env, obj, requested_size);
        break;
    }
#ifdef VSJNI_MEMMANAGER_DEBUG
    printf ("alloc: actual %p(%lld) returned %p(%lld) size (%ld)\n",
        buffer,
        (long long)buffer,
        retval,
        (long long) retval,
        (long)requested_size);
#endif
    // Now, align on VSJNI_ALIGNMENT_BOUNDARY byte boundary;
    // on Posix system we could have used memalign for the malloc,
    // but that doesn't work
    // for Java allocations, so we do this hack for everyone.
    return VSJNI_alignMemory(retval);
  }
  catch (std::bad_alloc & e)
  {
    return 0;
  }
}

/**
 * Aligns memory on a VSJNI_ALIGNMENT_BOUNDARY address.
 *
 * @param aInput The address to align; caller is responsible for
 *  ensuring non-null and
 * that sufficient extra space exists in buffer to allow alignment.
 */
static void
VSJNI_free(void * mem)
{
  void *buffer = 0;
  if (mem)
  {
    // realign back to original boundary
    mem = VSJNI_unalignMemory(mem);

    // Find the actual start of the memory
    buffer = (void*) ((char*) mem - sizeof(VSJNI_AllocationHeader));

    // Get our header
    VSJNI_AllocationHeader *header = (VSJNI_AllocationHeader*) buffer;
    enum VSJNIMemoryModel model = header->mModel;
    if (model != NATIVE_BUFFERS)
    {
      // We know that this was allocated with a JVM
      JNIEnv * env = VSJNI_getEnv();
      if (env)
      {
        if (model == JAVA_DIRECT_BUFFERS)
        {
          // delete the global ref, so that when we return the
          // jvm can gc
          jobject ref = header->mRef;
          header->mRef = 0;
          env->DeleteGlobalRef(ref);
        }
        else if (model == NATIVE_BUFFERS_WITH_JAVA_NOTIFICATION)
        {
          // delete the small memory token we asked Java to allocate for us
          jobject ref = header->mRef;
          header->mRef = 0;
          env->DeleteGlobalRef(ref);
          free(buffer);
        }
        else
        {
          if (env->ExceptionCheck())
            throw std::runtime_error("got java exception");

          if (header->mAllocator)
          {
            // Tell the allocator we're done
            // We're relying on the fact that the WeakReference passed in
            // is always outlived
            // by the allocator object (knock on wood)
            env->CallVoidMethod(header->mAllocator,
                sJNIMemoryAllocatorFreeMethod, header->mRef);
            if (env->ExceptionCheck())
              throw std::runtime_error("got java exception");
            env->DeleteGlobalRef(header->mAllocator);
            if (env->ExceptionCheck())
              throw std::runtime_error("got java exception");
          }

          // Get a local copy so that when we delete the global
          // ref, the gc thread won't free the underlying memory
          jbyteArray array = static_cast<jbyteArray> (env->NewLocalRef(
              header->mRef));
          if (env->ExceptionCheck())
            throw std::runtime_error("got java exception");
          if (!array)
            throw std::runtime_error("got java exception");

          // delete the global ref, so that when we return the
          // jvm can gc
          env->DeleteGlobalRef(header->mRef);
          header->mRef = 0;
          if (env->ExceptionCheck())
            throw std::runtime_error("got java exception");

          // Tell the JVM to release the elements of raw memory
          env->ReleaseByteArrayElements(array, (jbyte*) buffer, JNI_ABORT);
          if (env->ExceptionCheck())
            throw std::runtime_error("got java exception");

          // and we should be careful to delete our local ref because
          // we don't know how deep we are in native calls, or when
          // we'll actually return to the jvm
          env->DeleteLocalRef(array);
          if (env->ExceptionCheck())
            throw std::runtime_error("got java exception");
        }
      }
      else
      {
        // Technically we should never get here; once the JVM is initialized
        // then we should always have it around; but we can't guarantee that,
        // so in this case we LEAK the memory.
        // That'll probably be OK because this comes up mostly when the JVM has
        // shutdown anyway, and we really only care about running within a JVM
      }
#ifdef VSJNI_MEMMANAGER_DEBUG
      printf("jvm    ");
#endif
    }
    else
    {
      free(buffer);
#ifdef VSJNI_MEMMANAGER_DEBUG
      printf("stdlib ");
#endif

    }
#ifdef VSJNI_MEMMANAGER_DEBUG
    printf("free: orig %p (%lld) adjusted %p (%lld)\n",
        mem,
        (long long) mem,
        buffer,
        (long long)buffer);
#endif
  }
}

#else
static void *VSJNI_malloc(jobject, size_t requested_size)
{
  void* retval = 0;

  if (
      (sizeof(size_t) == 4 && (int)requested_size > INT_MAX - VSJNI_ALIGNMENT_BOUNDARY) ||
      ((long long)requested_size > LLONG_MAX - VSJNI_ALIGNMENT_BOUNDARY))
  // we need 16 byte clearance; ok, for 64-bit machines if you're
  // asking for 9-Tera-Whatevers of memory, you'll fail anyway,
  // but we try to be complete.
  return 0;
  retval = malloc(requested_size + VSJNI_ALIGNMENT_BOUNDARY);
  if (!retval)
  return 0;

  return VSJNI_alignMemory(retval);
}
static void
VSJNI_free(void * mem)
{
  // realign back to original boundary
  if (mem)
  {
    mem = VSJNI_unalignMemory(mem);
    free(mem);
  }
}
#endif // USE_JVM_FOR_MEM_MANAGEMENT
static void*
VSJNI_alignMemory(void* aInput)
{
  void* retval = aInput;
  retval = aInput;
  int alignDiff = ((-(long) retval - 1) & (VSJNI_ALIGNMENT_BOUNDARY - 1)) + 1;
  retval = (char*) retval + alignDiff;
  ((char*) retval)[-1] = (char) alignDiff;
#ifdef VSJNI_MEMMANAGER_DEBUG
  printf ("align: orig(%p:%lld) new(%p:%lld) align(%d)\n",
      aInput, (long long)aInput,
      retval, (long long) retval,
      alignDiff);
#endif

  return retval;
}

/**
 * Unaligns memory aligned with VSJNI_alignMemory(void*)
 *
 * @param aInput The address to unalign; caller is responsible for ensuring non null and
 *   that address was prior returned from VSJNI_alignMemory(void*)
 */
static void*
VSJNI_unalignMemory(void *aInput)
{
  int alignDiff = ((char*) aInput)[-1];
  void * retval = (void*) (((char*) aInput) - alignDiff);
#ifdef VSJNI_MEMMANAGER_DEBUG
  printf ("unalign: orig(%p:%lld) new(%p:%lld) align(%d)\n",
      aInput, (long long)aInput,
      retval, (long long) retval,
      alignDiff);
#endif
  return retval;
}

/**
 * Don't enable this; it turns out that on Mac i86_64 at least during the
 * loading of shared libraries, new linkages can override existing linkages
 * So if a library calls ::new before we're loaded, then loads us, and then
 * calls ::delete, the Mac may accidentally call our ::delete; not good.
 */
#ifdef VSJNI_OVERRIDE_CPP_NEW_AND_DELETE
VS_API_FERRY void *
operator new (size_t requested_size)
{
  return VSJNI_malloc(0, requested_size);
}

VS_API_FERRY void
operator delete (void *mem)
{
  VSJNI_free(mem);
}
#endif // VSJNI_OVERRIDE_CPP_NEW_AND_DELETE
/**
 * Here are some JNI functions used by the JNIMemoryAllocator.java method
 */
extern "C"
{
JNIEXPORT void JNICALL
Java_com_xuggle_ferry_JNIMemoryAllocator_setAllocator(JNIEnv *, jclass,
    jlong aNativeObj, jobject aMemMgr)
{
  // assume that the native object is a RefCounted
  com::xuggle::ferry::RefCounted *obj =
      *(com::xuggle::ferry::RefCounted**) &aNativeObj;
  obj->setJavaAllocator(aMemMgr);
}

JNIEXPORT jobject JNICALL
Java_com_xuggle_ferry_JNIMemoryAllocator_getAllocator(JNIEnv *env, jclass,
    jlong aNativeObj)
{
  // assume that the native object is a RefCounted
  com::xuggle::ferry::RefCounted *obj =
      *(com::xuggle::ferry::RefCounted**) &aNativeObj;
  jobject result = (jobject) obj->getJavaAllocator();
  if (result)
    result = env->NewLocalRef(result);
  return result;
}

JNIEXPORT jint JNICALL
Java_com_xuggle_ferry_FerryJNI_getMemoryModel(JNIEnv *, jclass)
{
  return (jint) sVSJNI_IsMirroringNativeMemoryInJVM;
}

JNIEXPORT void JNICALL
Java_com_xuggle_ferry_FerryJNI_setMemoryModel(JNIEnv *, jclass, jint value)
{
#ifdef VSJNI_USE_JVM_FOR_MEMMANAGEMENT
  sVSJNI_IsMirroringNativeMemoryInJVM = (enum VSJNIMemoryModel) value;
#endif
}

}
