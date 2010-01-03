/*******************************************************************************
 * Copyright (c) 2008, 2010 Xuggle Inc.  All rights reserved.
 *  
 * This file is part of Xuggle-Xuggler-Main.
 *
 * Xuggle-Xuggler-Main is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Xuggle-Xuggler-Main is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Xuggle-Xuggler-Main.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
package com.xuggle.ferry;


import java.nio.ByteBuffer;
import java.util.LinkedHashMap;
import java.util.concurrent.atomic.AtomicReference;

import junit.framework.TestCase;

/**
 * A collection of test methods that can be used to test
 * the performance of the IBuffer Memory Model. 
 * 
 * This test is in JUnit3 format so it can be run from JMeter.
 * I know it's a canonical example of a JUnit4 parameterized
 * test, but them's the breaks.
 * 
 * @author aclarke
 *
 */
public class MemoryModelExhaustiveTest extends TestCase
{
  private static final int MAX_BUFFERS_TO_CACHE=5;
  private static final int NUM_ALLOCS_PER_TEST=1000;
  private static final int TEST_BUFFER_SIZE=1024*1024;

  public MemoryModelExhaustiveTest()
  {
  }
  
  public void tearDown()
  {
    JNIMemoryManager.getMgr().flush();
  }
  
  public void testMediumTermReference_JAVA_BYTE_ARRAYS()
  {
    helperMaintainMediumTermReference(
        JNIMemoryManager.MemoryModel.JAVA_STANDARD_HEAP,
        false); 
  }
  public void testMediumTermReference_JAVA_DIRECT_BUFFERS()
  {
    helperMaintainMediumTermReference(
        JNIMemoryManager.MemoryModel.JAVA_DIRECT_BUFFERS,
        false); 
  }
  public void testMediumTermReference_JAVA_DIRECT_BUFFERS_WITH_STANDARD_HEAP_NOTIFICATION()
  {
    helperMaintainMediumTermReference(
        JNIMemoryManager.MemoryModel.JAVA_DIRECT_BUFFERS_WITH_STANDARD_HEAP_NOTIFICATION,
        false); 
  }
  public void testMediumTermReference_NATIVE_BUFFERS_WITH_STANDARD_HEAP_NOTIFICATION()
  {
    helperMaintainMediumTermReference(
        JNIMemoryManager.MemoryModel.NATIVE_BUFFERS_WITH_STANDARD_HEAP_NOTIFICATION,
        false); 
  }
  public void testMediumTermReference_ExplicitCleanup_JAVA_BYTE_ARRAYS()
  {
    helperMaintainMediumTermReference(
        JNIMemoryManager.MemoryModel.JAVA_STANDARD_HEAP,
        true); 
  }
  public void testMediumTermReference_ExplicitCleanup_JAVA_DIRECT_BUFFERS()
  {
    helperMaintainMediumTermReference(
        JNIMemoryManager.MemoryModel.JAVA_DIRECT_BUFFERS,
        true); 
  }
  public void testMediumTermReference_ExplicitCleanup_JAVA_DIRECT_BUFFERS_WITH_STANDARD_HEAP_NOTIFICATION()
  {
    helperMaintainMediumTermReference(
        JNIMemoryManager.MemoryModel.JAVA_DIRECT_BUFFERS_WITH_STANDARD_HEAP_NOTIFICATION,
        true); 
  }
  public void testMediumTermReference_ExplicitCleanup_NATIVE_BUFFERS()
  {
    helperMaintainMediumTermReference(
        JNIMemoryManager.MemoryModel.NATIVE_BUFFERS,
        true); 
  }
  public void testMediumTermReference_ExplicitCleanup_NATIVE_BUFFERS_WITH_STANDARD_HEAP_NOTIFICATION()
  {
    helperMaintainMediumTermReference(
        JNIMemoryManager.MemoryModel.NATIVE_BUFFERS_WITH_STANDARD_HEAP_NOTIFICATION,
        true); 
  }

  public void testShortTermReference_JAVA_BYTE_ARRAYS()
  {
    helperMaintainShortTermReference(
        JNIMemoryManager.MemoryModel.JAVA_STANDARD_HEAP,
        false); 
  }
  public void testShortTermReference_JAVA_DIRECT_BUFFERS()
  {
    helperMaintainShortTermReference(
        JNIMemoryManager.MemoryModel.JAVA_DIRECT_BUFFERS,
        false); 
  }
  public void testShortTermReference_JAVA_DIRECT_BUFFERS_WITH_STANDARD_HEAP_NOTIFICATION()
  {
    helperMaintainShortTermReference(
        JNIMemoryManager.MemoryModel.JAVA_DIRECT_BUFFERS_WITH_STANDARD_HEAP_NOTIFICATION,
        false); 
  }
  public void testShortTermReference_NATIVE_BUFFERS_WITH_STANDARD_HEAP_NOTIFICATION()
  {
    helperMaintainShortTermReference(
        JNIMemoryManager.MemoryModel.NATIVE_BUFFERS_WITH_STANDARD_HEAP_NOTIFICATION,
        false); 
  }
  public void testShortTermReference_ExplicitCleanup_JAVA_BYTE_ARRAYS()
  {
    helperMaintainShortTermReference(
        JNIMemoryManager.MemoryModel.JAVA_STANDARD_HEAP,
        true); 
  }
  public void testShortTermReference_ExplicitCleanup_JAVA_DIRECT_BUFFERS()
  {
    helperMaintainShortTermReference(
        JNIMemoryManager.MemoryModel.JAVA_DIRECT_BUFFERS,
        true); 
  }
  public void testShortTermReference_ExplicitCleanup_JAVA_DIRECT_BUFFERS_WITH_STANDARD_HEAP_NOTIFICATION()
  {
    helperMaintainShortTermReference(
        JNIMemoryManager.MemoryModel.JAVA_DIRECT_BUFFERS_WITH_STANDARD_HEAP_NOTIFICATION,
        true); 
  }
  public void testShortTermReference_ExplicitCleanup_NATIVE_BUFFERS()
  {
    helperMaintainShortTermReference(
        JNIMemoryManager.MemoryModel.NATIVE_BUFFERS,
        true); 
  }
  public void testShortTermReference_ExplicitCleanup_NATIVE_BUFFERS_WITH_STANDARD_HEAP_NOTIFICATION()
  {
    helperMaintainShortTermReference(
        JNIMemoryManager.MemoryModel.NATIVE_BUFFERS_WITH_STANDARD_HEAP_NOTIFICATION,
        true); 
  }
  
  public static void helperMaintainShortTermReference(
      JNIMemoryManager.MemoryModel model,
      final boolean cleanUpAfterOurselves)
  {
    helperMaintainShortTermReference(model, cleanUpAfterOurselves,
        NUM_ALLOCS_PER_TEST, TEST_BUFFER_SIZE, true);
  }
  public static void helperMaintainShortTermReference(
      final JNIMemoryManager.MemoryModel model,
      final boolean cleanUpAfterOurselves,
      final int numAllocsPerTest,
      final int bufferSize,
      final boolean forceCommit)
  {
    JNIMemoryManager.setMemoryModel(model);
    System.out.println("Short Term Test; Memory model = " + JNIMemoryManager.getMemoryModel());
    int i = 0;
    RefCounted allocator = RefCountedTester.make();

    for(i = 0; i < numAllocsPerTest; i++)
    {
      IBuffer buffer = getTestBuffer(allocator, bufferSize, forceCommit);
      if (cleanUpAfterOurselves)
        buffer.delete();
    }
    System.out.println("Finished allocating objects: " + i);
  }
  
  public static void helperMaintainMediumTermReference(
      JNIMemoryManager.MemoryModel model,
      final boolean cleanUpAfterOurselves
  )
  {
    helperMaintainMediumTermReference(model,
        cleanUpAfterOurselves,
        NUM_ALLOCS_PER_TEST,
        TEST_BUFFER_SIZE,
        MAX_BUFFERS_TO_CACHE,
        true);
  }
  public static void helperMaintainMediumTermReference(
      final JNIMemoryManager.MemoryModel model,
      final boolean cleanUpAfterOurselves,
      final int numAllocsPerTest,
      final int bufferSize,
      final int cacheSize,
      final boolean forceCommit
  )
  {
    JNIMemoryManager.setMemoryModel(model);
    System.out.println("Medium Term Test; Memory model = " + JNIMemoryManager.getMemoryModel());
    LinkedHashMap<IBuffer, IBuffer> map = new LinkedHashMap<IBuffer, IBuffer>()
    {
      private static final long serialVersionUID = 1L;

      @Override
      protected boolean removeEldestEntry(
          java.util.Map.Entry<IBuffer,IBuffer> eldest) 
      {
        if(size() > cacheSize)
        {
          IBuffer key = eldest.getKey();
          this.remove(key);
          if(cleanUpAfterOurselves)
            key.delete();
        }
        return false;
      }
    };
    int i = 0;
    RefCounted allocator = RefCountedTester.make();
    for(i = 0; i < numAllocsPerTest; i++)
    {
      IBuffer buffer = getTestBuffer(allocator, bufferSize, forceCommit);
      map.put(buffer, buffer);
      assertTrue("unexpected map size: " + map.size(), 
          map.size() <= cacheSize);
    }
    System.out.println("Finished allocating objects: " + i);
  }

  public static IBuffer getTestBuffer(RefCounted allocator, int size,
      boolean forceCommit) throws OutOfMemoryError
  {
    IBuffer buffer = null;
    try {
      buffer = IBuffer.make(allocator, size);
      if (forceCommit) {
        AtomicReference<JNIReference> ref = new AtomicReference<JNIReference>(null);
        ByteBuffer jbuffer = buffer.getByteBuffer(0, size, ref);
        for(int i = 0; i < size; i++)
          // write to every byte to force a commit to memory on some OSes
          jbuffer.put(i, (byte)0xFF);
        // and release the byte buffer reference so it doesn't pollute
        // our test
        ref.get().delete();
      }
    } catch (OutOfMemoryError e) {
      // odd here, but we're going to force the standard heap
      // to allocate until there is an exception; this should
      // clear up some ferry references and let the allocation
      // of the direct heap succeed.
      System.out.println("Attempting recovery from OOME");
      try {
        MemoryTestHelper.forceJavaHeapWeakReferenceClear();
        JNIMemoryManager.collect();
        buffer = IBuffer.make(null, size);
      }
      catch (OutOfMemoryError e2)
      {
        throw e2;
      }
      System.out.println("Recovered from OOME");
    }
    return buffer;
  }

}
