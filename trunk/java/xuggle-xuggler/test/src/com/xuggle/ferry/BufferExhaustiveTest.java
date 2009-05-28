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

package com.xuggle.ferry;

import static junit.framework.Assert.assertEquals;
import static junit.framework.Assert.assertNotNull;

import java.nio.ByteBuffer;
import java.util.Collection;
import java.util.LinkedList;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;

import com.xuggle.ferry.JNIMemoryManager.MemoryModel;

@RunWith(Parameterized.class)
public class BufferExhaustiveTest
{
  @Before
  public void setUp()
  {
  }
  @After
  public void tearDown()
  {
  }

  @Parameters
  public static Collection<Object[]> getModels()
  {
    Collection<Object[]> retval = new LinkedList<Object[]>();
    // add all the models.
    for(MemoryModel model: JNIMemoryManager.MemoryModel.values())
      retval.add(new Object[]{
          model
      });
//    retval.add(new Object[]{ JNIMemoryManager.MemoryModel.NATIVE_BUFFERS_WITH_STANDARD_HEAP_NOTIFICATION});
    return retval;
  }

  public BufferExhaustiveTest(MemoryModel model)
  {
    JNIMemoryManager.setMemoryModel(model);
  }
  
  /**
   * This tests tries to make sure the IBuffer correctly releases any
   * references it might hold to the underlying java.nio.ByteBuffer
   * it wraps.
   * 
   * It does this by creating a lot of large IBuffer objects.
   */
  @Test
  public void testNoLeaksWhenBufferMadeFromDirectJavaByteBuffer()
  {
    // Start from zero
    JNIReference.getMgr().flush();
    for(int i = 0; i < 100; i++)
    {
      assertEquals(0, JNIReference.getMgr().getNumPinnedObjects());
      ByteBuffer nativeBytes = ByteBuffer.allocateDirect(10*1024*1024);
      IBuffer buf = IBuffer.make(null, nativeBytes, 0, nativeBytes.capacity());
      assertNotNull(buf);
      assertEquals(1, JNIReference.getMgr().getNumPinnedObjects());
      buf.delete();
      buf = null;
      assertEquals(0, JNIReference.getMgr().getNumPinnedObjects());
      
      // We need to do three GC's because it appears Java NIO ByteBuffers sometimes rely
      // on finalizers to free memory.  The first GC will mark something for finalization
      // The second will finalize, and the third will reclaim.
      System.gc();
      System.gc();
      System.gc();
      Thread.yield();
    }
    // and flush again
    JNIReference.getMgr().flush();
  }

  /**
   * This is a paranoia test really testing java.nio.ByteBuffers.
   * 
   * If you make a view of a ByteBuffer (ByteBuffer.asIntBuffer for example),
   * we want to make sure the resulting IntBuffer maintains a reference
   * back to the original ByteBuffer.
   * 
   */
  @Test(timeout=2*60*1000)
  public void testIntBuffersMaintainReferenceToByteBuffer()
  {
    // make sure we don't have any pinned objects lying around
    while(JNIReference.getMgr().getNumPinnedObjects()>0)
    {
      byte[] arr = new byte[10*1024*1024];
      arr[0] = 0;
      System.gc();
      JNIReference.getMgr().gc();
    }
    assertEquals(0, JNIReference.getMgr().getNumPinnedObjects());

    // now start the test
    IBuffer buf = IBuffer.make(null, 1024*1024);
    assertNotNull(buf);

    assertEquals(1, JNIReference.getMgr().getNumPinnedObjects());

    // This will create a reference
    java.nio.ByteBuffer jbuf = buf.getByteBuffer(0, buf.getBufferSize());

    assertEquals(2, JNIReference.getMgr().getNumPinnedObjects());

    // kill the xuggler refcount in the IBuffer
    buf.delete();

    assertEquals(1, JNIReference.getMgr().getNumPinnedObjects());


    // this is the test; we want to know if creating an IntBuffer
    // ends up holding a reference to the underlying jbuf
    java.nio.IntBuffer ibuf = jbuf.asIntBuffer();
    ibuf.put(0, 15);

    // release the reference to the bytebuffer
    jbuf = null;

    assertEquals(1, JNIReference.getMgr().getNumPinnedObjects());
    assertEquals(15, ibuf.get(0));
    
    // now we're going to try for force a gc
    byte[] arr;
    for(int i = 0; i < 1000; i++)
    {
      arr = new byte[1024*1024]; 
      JNIReference.getMgr().gc();
      arr[0] = (byte)1;
      ibuf.put(i, 16);
    }
    
    // even though the buf and jbuf are now dead, the ibuf should
    // be forcing one buffer to be pinned.
    assertEquals(1, JNIReference.getMgr().getNumPinnedObjects());
    
    // this is here to make sure the compiler doesn't optimize
    // away ibuf
    assertEquals(16, ibuf.get(999));
    // release the ibuf
    ibuf= null;
    // try to force another gc to make sure ibuf is now collected
    while(JNIReference.getMgr().getNumPinnedObjects()>0)
    {
      arr = new byte[1024*1024]; 
      System.gc();
      JNIReference.getMgr().gc();
      arr[0] = (byte)1;
    }

    // and make sure the ibuf is collected
    assertEquals(0, JNIReference.getMgr().getNumPinnedObjects());
  }
  
  @Test(timeout=60*1000)
  public void testJNIMemoryManagerCollectionThread()
  {
    JNIMemoryManager.getMgr().startCollectionThread();
    IBuffer buf = IBuffer.make(null, 1024*1024);
    assertNotNull(buf);

    // This will create a reference
    java.nio.ByteBuffer jbuf = buf.getByteBuffer(0, buf.getBufferSize());

    assertEquals(2, JNIMemoryManager.getMgr().getNumPinnedObjects());
    // kill the xuggler refcount in the IBuffer
    buf.delete();
    assertEquals(1, JNIMemoryManager.getMgr().getNumPinnedObjects());
    jbuf.put((byte) 0);
    
    // now kill reference to the byte buffer which should enqueue
    // the reference
    jbuf = null;
    
    // try to force another gc to make sure jbuf is now collected
    // by the separate thread
    while(JNIReference.getMgr().getNumPinnedObjects()>0)
    {
      byte[] arr = new byte[1024*1024]; 
      System.gc();
      arr[0] = (byte)1;
    }
    JNIMemoryManager.getMgr().stopCollectionThread();
  }
  
  /**
   * This method allocates one large IBuffer and then repeatedly
   * copies out the bytes into a Java byte[] array.
   * 
   * If the system is not leaking, the garbage collector will ensure
   * we don't run out of heap space.  If we're leaking, bad things
   * will occur.
   */
  @Test
  public void testNoLeakingMemoryOnCopy()
  {
    IBuffer buf = IBuffer.make(null, 1024*1024); // 1 MB
    assertNotNull(buf);
    for(int i = 0; i < 1000; i++)
    {
      byte[] outBytes = buf.getByteArray(0, buf.getBufferSize());
      // and we do nothing with the outBytes
      assertEquals(outBytes.length, buf.getBufferSize());
      outBytes = null;
    }
  }
  

}
