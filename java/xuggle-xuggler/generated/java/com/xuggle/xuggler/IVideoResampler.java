/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.37
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.xuggle.xuggler;
import com.xuggle.ferry.*;
/**
 * Converts {@link IVideoPicture} objects of a given width, height and 
 * format to a new  
 * width, height or format.  
 */
public class IVideoResampler extends RefCounted implements com.xuggle.xuggler.IConfigurable {
  // JNIHelper.swg: Start generated code
  // >>>>>>>>>>>>>>>>>>>>>>>>>>>
  /**
   * This method is only here to use some references and remove
   * a Eclipse compiler warning.
   */
  @SuppressWarnings("unused")
  private void noop()
  {
    IBuffer.make(null, 1);
  }
   
  private volatile long swigCPtr;

  /**
   * Internal Only.
   */
  protected IVideoResampler(long cPtr, boolean cMemoryOwn) {
    super(XugglerJNI.SWIGIVideoResamplerUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }
  
  /**
   * Internal Only.
   */
  protected IVideoResampler(long cPtr, boolean cMemoryOwn,
      java.util.concurrent.atomic.AtomicLong ref)
  {
    super(XugglerJNI.SWIGIVideoResamplerUpcast(cPtr),
     cMemoryOwn, ref);
    swigCPtr = cPtr;
  }
    
  /**
   * Internal Only.  Not part of public API.
   *
   * Get the raw value of the native object that obj is proxying for.
   *   
   * @param obj The java proxy object for a native object.
   * @return The raw pointer obj is proxying for.
   */
  public static long getCPtr(IVideoResampler obj) {
    if (obj == null) return 0;
    return obj.getMyCPtr();
  }

  /**
   * Internal Only.  Not part of public API.
   *
   * Get the raw value of the native object that we're proxying for.
   *   
   * @return The raw pointer we're proxying for.
   */  
  public long getMyCPtr() {
    if (swigCPtr == 0) throw new IllegalStateException("underlying native object already deleted");
    return swigCPtr;
  }
  
  /**
   * Create a new IVideoResampler object that is actually referring to the
   * exact same underlying native object.
   *
   * @return the new Java object.
   */
  @Override
  public IVideoResampler copyReference() {
    if (swigCPtr == 0)
      return null;
    else
      return new IVideoResampler(swigCPtr, swigCMemOwn, getJavaRefCount());
  }

  /**
   * Releases ths instance of IVideoResampler and frees any underlying
   * native memory.
   * <p>
   * {@inheritDoc}
   * </p> 
   */
  @Override
  public void delete()
  {
    do {} while(false); // remove a warning
    super.delete();
  }

  /**
   * Compares two values, returning true if the underlying objects in native code are the same object.
   *
   * That means you can have two different Java objects, but when you do a comparison, you'll find out
   * they are the EXACT same object.
   *
   * @return True if the underlying native object is the same.  False otherwise.
   */
  public boolean equals(Object obj) {
    boolean equal = false;
    if (obj instanceof IVideoResampler)
      equal = (((IVideoResampler)obj).swigCPtr == this.swigCPtr);
    return equal;
  }
  
  /**
   * Get a hashable value for this object.
   *
   * @return the hashable value.
   */
  public int hashCode() {
     return (int)swigCPtr;
  }
  
  // <<<<<<<<<<<<<<<<<<<<<<<<<<<
  // JNIHelper.swg: End generated code
  

  /**
   * {@inheritDoc}
   */
  public java.util.Collection<String> getPropertyNames()
  {
    java.util.Collection<String> retval = new java.util.LinkedList<String>();
    int numProperties = this.getNumProperties();
    for(int i = 0; i < numProperties; i++)
    {
      IProperty property = this.getPropertyMetaData(i);
      String name = property.getName();
      retval.add(name);
    }
    return retval;
  }

/**
 * Get the width in pixels we expect on the input frame to the resampler. 
 *  
 * @return	The width we expect on the input frame to the resampler. 
 *		  
 */
  public int getInputWidth() {
    return XugglerJNI.IVideoResampler_getInputWidth(swigCPtr, this);
  }

/**
 * Get the height in pixels we expect on the input frame to the resampler. 
 *  
 * @return	The height we expect on the input frame to the resampler. 
 *		  
 */
  public int getInputHeight() {
    return XugglerJNI.IVideoResampler_getInputHeight(swigCPtr, this);
  }

/**
 * Get the input pixel format.  
 * @return	The pixel format we expect on the input frame to the resampler. 
 *		  
 */
  public IPixelFormat.Type getInputPixelFormat() {
    return IPixelFormat.Type.swigToEnum(XugglerJNI.IVideoResampler_getInputPixelFormat(swigCPtr, this));
  }

/**
 * Get the output width, in pixels.  
 * @return	The width we will resample the output frame to  
 */
  public int getOutputWidth() {
    return XugglerJNI.IVideoResampler_getOutputWidth(swigCPtr, this);
  }

/**
 * Get the output height, in pixels.  
 * @return	The height we will resample the output frame to  
 */
  public int getOutputHeight() {
    return XugglerJNI.IVideoResampler_getOutputHeight(swigCPtr, this);
  }

/**
 * Get the output pixel format.  
 * @return	The pixel format we will resample the output frame to  
 */
  public IPixelFormat.Type getOutputPixelFormat() {
    return IPixelFormat.Type.swigToEnum(XugglerJNI.IVideoResampler_getOutputPixelFormat(swigCPtr, this));
  }

/**
 * Resample in to out based on the resampler parameters.  
 * Resamples the in picture based on the parameters set when  
 * this resampler was constructed.  
 * @param	out The picture we'll resample to. Check  
 * {@link IVideoPicture#isComplete()} after the call.  
 * @param	in The picture we'll resample from.  
 * @return	>= 0 on success; <0 on error.  
 */
  public int resample(IVideoPicture out, IVideoPicture in) {
    return XugglerJNI.IVideoResampler_resample(swigCPtr, this, IVideoPicture.getCPtr(out), out, IVideoPicture.getCPtr(in), in);
  }

/**
 * Returns the total number of settable properties on this object  
 * @return	total number of options (not including constant definitions) 
 *		  
 */
  public int getNumProperties() {
    return XugglerJNI.IVideoResampler_getNumProperties(swigCPtr, this);
  }

/**
 * Returns the name of the numbered property.  
 * @param	propertyNo The property number in the options list.  
 * @return	an IProperty value for this properties meta-data  
 */
  public IProperty getPropertyMetaData(int propertyNo) {
    long cPtr = XugglerJNI.IVideoResampler_getPropertyMetaData__SWIG_0(swigCPtr, this, propertyNo);
    return (cPtr == 0) ? null : new IProperty(cPtr, false);
  }

/**
 * Returns the name of the numbered property.  
 * @param	name The property name.  
 * @return	an IProperty value for this properties meta-data  
 */
  public IProperty getPropertyMetaData(String name) {
    long cPtr = XugglerJNI.IVideoResampler_getPropertyMetaData__SWIG_1(swigCPtr, this, name);
    return (cPtr == 0) ? null : new IProperty(cPtr, false);
  }

/**
 * Sets a property on this Object.  
 * All AVOptions supported by the underlying AVClass are supported. 
 *  
 * @param	name The property name. For example "b" for bit-rate.  
 * @param	value The value of the property.  
 * @return	>= 0 if the property was successfully set; <0 on error  
 */
  public int setProperty(String name, String value) {
    return XugglerJNI.IVideoResampler_setProperty__SWIG_0(swigCPtr, this, name, value);
  }

/**
 * Looks up the property 'name' and sets the  
 * value of the property to 'value'.  
 * @param	name name of option  
 * @param	value Value of option  
 * @return	>= 0 on success; <0 on error.  
 */
  public int setProperty(String name, double value) {
    return XugglerJNI.IVideoResampler_setProperty__SWIG_1(swigCPtr, this, name, value);
  }

/**
 * Looks up the property 'name' and sets the  
 * value of the property to 'value'.  
 * @param	name name of option  
 * @param	value Value of option  
 * @return	>= 0 on success; <0 on error.  
 */
  public int setProperty(String name, long value) {
    return XugglerJNI.IVideoResampler_setProperty__SWIG_2(swigCPtr, this, name, value);
  }

/**
 * Looks up the property 'name' and sets the  
 * value of the property to 'value'.  
 * @param	name name of option  
 * @param	value Value of option  
 * @return	>= 0 on success; <0 on error.  
 */
  public int setProperty(String name, boolean value) {
    return XugglerJNI.IVideoResampler_setProperty__SWIG_3(swigCPtr, this, name, value);
  }

/**
 * Looks up the property 'name' and sets the  
 * value of the property to 'value'.  
 * @param	name name of option  
 * @param	value Value of option  
 * @return	>= 0 on success; <0 on error.  
 */
  public int setProperty(String name, IRational value) {
    return XugglerJNI.IVideoResampler_setProperty__SWIG_4(swigCPtr, this, name, IRational.getCPtr(value), value);
  }

/**
 * Gets a property on this Object.  
 * Note for C++ callers; you must free the returned array with  
 * delete[] in order to avoid a memory leak. Other language  
 * folks need not worry.  
 * @param	name property name  
 * @return	an string copy of the option value, or null if the option 
 *		 doesn't exist.  
 */
  public String getPropertyAsString(String name) {
    return XugglerJNI.IVideoResampler_getPropertyAsString(swigCPtr, this, name);
  }

/**
 * Gets the value of this property, and returns as a double;  
 * @param	name name of option  
 * @return	double value of property, or 0 on error.  
 */
  public double getPropertyAsDouble(String name) {
    return XugglerJNI.IVideoResampler_getPropertyAsDouble(swigCPtr, this, name);
  }

/**
 * Gets the value of this property, and returns as an long;  
 * @param	name name of option  
 * @return	long value of property, or 0 on error.  
 */
  public long getPropertyAsLong(String name) {
    return XugglerJNI.IVideoResampler_getPropertyAsLong(swigCPtr, this, name);
  }

/**
 * Gets the value of this property, and returns as an IRational;  
 * @param	name name of option  
 * @return	long value of property, or 0 on error.  
 */
  public IRational getPropertyAsRational(String name) {
    long cPtr = XugglerJNI.IVideoResampler_getPropertyAsRational(swigCPtr, this, name);
    return (cPtr == 0) ? null : new IRational(cPtr, false);
  }

/**
 * Gets the value of this property, and returns as a boolean  
 * @param	name name of option  
 * @return	boolean value of property, or false on error.  
 */
  public boolean getPropertyAsBoolean(String name) {
    return XugglerJNI.IVideoResampler_getPropertyAsBoolean(swigCPtr, this, name);
  }

/**
 * Get a new video resampler. Returns null if {@link #isSupported(Feature)} 
 * returns false.  
 * @param	outputWidth The width in pixels you want to output frame to 
 *		 have.  
 * @param	outputHeight The height in pixels you want to output frame 
 *		 to have.  
 * @param	outputFmt The pixel format of the output frame.  
 * @param	inputWidth The width in pixels the input frame will be in. 
 *		  
 * @param	inputHeight The height in pixels the input frame will be in. 
 *		  
 * @param	inputFmt The pixel format of the input frame.  
 * @return	a new object, or null if we cannot allocate one.  
 */
  public static IVideoResampler make(int outputWidth, int outputHeight, IPixelFormat.Type outputFmt, int inputWidth, int inputHeight, IPixelFormat.Type inputFmt) {
    long cPtr = XugglerJNI.IVideoResampler_make(outputWidth, outputHeight, outputFmt.swigValue(), inputWidth, inputHeight, inputFmt.swigValue());
    return (cPtr == 0) ? null : new IVideoResampler(cPtr, false);
  }

/**
 * Returns true if the asked for feature is supported.  
 * @param	feature The feature you want to find out is supported.  
 * @return	true if the IVideoResampler supports this feature; false 
 *		 otherwise.  
 */
  public static boolean isSupported(IVideoResampler.Feature feature) {
    return XugglerJNI.IVideoResampler_isSupported(feature.swigValue());
  }

  public enum Feature {
  /**
   * Features that the VideoResampler may optionally support.
   */
    FEATURE_IMAGERESCALING,
    FEATURE_COLORSPACECONVERSION;

    public final int swigValue() {
      return swigValue;
    }

    public static Feature swigToEnum(int swigValue) {
      Feature[] swigValues = Feature.class.getEnumConstants();
      if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
        return swigValues[swigValue];
      for (Feature swigEnum : swigValues)
        if (swigEnum.swigValue == swigValue)
          return swigEnum;
      throw new IllegalArgumentException("No enum " + Feature.class + " with value " + swigValue);
    }

    @SuppressWarnings("unused")
    private Feature() {
      this.swigValue = SwigNext.next++;
    }

    @SuppressWarnings("unused")
    private Feature(int swigValue) {
      this.swigValue = swigValue;
      SwigNext.next = swigValue+1;
    }

    @SuppressWarnings("unused")
    private Feature(Feature swigEnum) {
      this.swigValue = swigEnum.swigValue;
      SwigNext.next = this.swigValue+1;
    }

    private final int swigValue;

    private static class SwigNext {
      private static int next = 0;
    }
  }

}
