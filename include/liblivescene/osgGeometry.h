// Copyright 2011 Skew Matrix Software and AlphaPixel

#ifndef __LIVESCENE_OSGGEOMETRY_H__
#define __LIVESCENE_OSGGEOMETRY_H__ 1

#include <osg/Array>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include "liblivescene/Export.h"
#include "liblivescene/GeometryBuilder.h"


namespace livescene {


/** \defgroup Geometry Geometry Building */
/*@{*/



/** \brief custom osg::Array class to wrap existing float*3 arrays without copying.
		Based on code from osgsharedarray.
*/

class WrappedArrayFloat3 : public osg::Array { 
public:
    /** Default ctor. Creates an empty array. */
    WrappedArrayFloat3() :
        osg::Array(osg::Array::Vec3ArrayType,3,GL_FLOAT),
        _numElements(0),
        _ptr(NULL) {
    }

    /** "Normal" ctor. 
      *
      * @param no The number of elements in the array.
      * @param ptr Pointer to the data. This class just keeps that 
      * pointer. It doesn't manage the memory.
      */
    WrappedArrayFloat3(unsigned int no, float * ptr) :
        osg::Array(osg::Array::Vec3ArrayType,3,GL_FLOAT),
        _numElements(no),
        _ptr(ptr) {
    }

    /** Copy ctor. */
    WrappedArrayFloat3(const WrappedArrayFloat3& other, const osg::CopyOp& copyop) :
        osg::Array(osg::Array::Vec3ArrayType,3,GL_FLOAT),
        _numElements(other._numElements),
        _ptr(other._ptr) {
    }

    /** What type of object would clone return? */
    virtual Object* cloneType() const { 
        return new WrappedArrayFloat3(); 
    }
    
    /** Create a copy of the object. */ 
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { 
        return new WrappedArrayFloat3(*this,copyop); 
    }        
        
    /** Accept method for ArrayVisitors.
      *
      * @note This will end up in ArrayVisitor::apply(osg::Array&).
      */
    virtual void accept(osg::ArrayVisitor& av) {
        av.apply(*this);
    }

    /** Const accept method for ArrayVisitors.
      *
      * @note This will end up in ConstArrayVisitor::apply(const osg::Array&).
      */
    virtual void accept(osg::ConstArrayVisitor& cav) const {
        cav.apply(*this);
    }

    /** Accept method for ValueVisitors. */
    virtual void accept(unsigned int index, osg::ValueVisitor& vv) {
        vv.apply(_ptr[index]);
    }

    /** Const accept method for ValueVisitors. */
    virtual void accept(unsigned int index, osg::ConstValueVisitor& cvv) const {
        cvv.apply(_ptr[index]);
    }

    /** Compare method. 
      * Return -1 if lhs element is less than rhs element, 0 if equal,
      * 1 if lhs element is greater than rhs element. 
	  * In this implementation, only compares the first of the three floats
      */
    virtual int compare(unsigned int lhs,unsigned int rhs) const {
        const float& elem_lhs = _ptr[lhs * 3];
        const float& elem_rhs = _ptr[rhs * 3];
        if (elem_lhs<elem_rhs) return -1;
        if (elem_rhs<elem_lhs) return  1;
        return 0;
    }

    /** Returns a pointer to the first element of the array. */
    virtual const GLvoid* getDataPointer() const {
        return _ptr;
    }

    /** Returns the number of elements in the array. */
    virtual unsigned int getNumElements() const {
        return _numElements;
    }

    /** Returns the number of bytes of storage required to hold 
      * all of the elements of the array.
      */
    virtual unsigned int getTotalDataSize() const {
        return _numElements * sizeof(float) * 3;
    }

private:
    unsigned int _numElements;
    float*   _ptr;
};





/** \brief custom osg::Array class to wrap existing float*3 arrays without copying.
		Based on code from osgsharedarray.
*/

class WrappedArrayShort3 : public osg::Array { 
public:
    /** Default ctor. Creates an empty array. */
    WrappedArrayShort3() :
        osg::Array(osg::Array::Vec3ArrayType,3,GL_SHORT),
        _numElements(0),
        _ptr(NULL) {
    }

    /** "Normal" ctor. 
      *
      * @param no The number of elements in the array.
      * @param ptr Pointer to the data. This class just keeps that 
      * pointer. It doesn't manage the memory.
      */
    WrappedArrayShort3(unsigned int no, short * ptr) :
        osg::Array(osg::Array::Vec3ArrayType,3,GL_SHORT),
        _numElements(no),
        _ptr(ptr) {
    }

    /** Copy ctor. */
    WrappedArrayShort3(const WrappedArrayShort3& other, const osg::CopyOp& copyop) :
        osg::Array(osg::Array::Vec3ArrayType,3,GL_SHORT),
        _numElements(other._numElements),
        _ptr(other._ptr) {
    }

    /** What type of object would clone return? */
    virtual Object* cloneType() const { 
        return new WrappedArrayShort3(); 
    }
    
    /** Create a copy of the object. */ 
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { 
        return new WrappedArrayShort3(*this,copyop); 
    }        
        
    /** Accept method for ArrayVisitors.
      *
      * @note This will end up in ArrayVisitor::apply(osg::Array&).
      */
    virtual void accept(osg::ArrayVisitor& av) {
        av.apply(*this);
    }

    /** Const accept method for ArrayVisitors.
      *
      * @note This will end up in ConstArrayVisitor::apply(const osg::Array&).
      */
    virtual void accept(osg::ConstArrayVisitor& cav) const {
        cav.apply(*this);
    }

    /** Accept method for ValueVisitors. */
    virtual void accept(unsigned int index, osg::ValueVisitor& vv) {
        vv.apply(_ptr[index]);
    }

    /** Const accept method for ValueVisitors. */
    virtual void accept(unsigned int index, osg::ConstValueVisitor& cvv) const {
        cvv.apply(_ptr[index]);
    }

    /** Compare method. 
      * Return -1 if lhs element is less than rhs element, 0 if equal,
      * 1 if lhs element is greater than rhs element. 
	  * In this implementation, only compares the first of the three floats
      */
    virtual int compare(unsigned int lhs,unsigned int rhs) const {
        const short& elem_lhs = _ptr[lhs * 3];
        const short& elem_rhs = _ptr[rhs * 3];
        if (elem_lhs<elem_rhs) return -1;
        if (elem_rhs<elem_lhs) return  1;
        return 0;
    }

    /** Returns a pointer to the first element of the array. */
    virtual const GLvoid* getDataPointer() const {
        return _ptr;
    }

    /** Returns the number of elements in the array. */
    virtual unsigned int getNumElements() const {
        return _numElements;
    }

    /** Returns the number of bytes of storage required to hold 
      * all of the elements of the array.
      */
    virtual unsigned int getTotalDataSize() const {
        return _numElements * sizeof(float) * 3;
    }

private:
    unsigned int _numElements;
    short*   _ptr;
};





/** \brief Build OSG geometry for a point cloud using a provided Geometry.
The supplied Geometry object must already have had buildPointCloud() called on it.
Utilizes the arrays already stored in the livescene::Geometry object without copying.
// <<<>>> this function does NOT currently work!
*/
LIVESCENE_EXPORT osg::Geode* buildOSGPointCloud(const livescene::Geometry &geometry, osg::Vec4 baseColor = osg::Vec4(1.0, 1.0, 1.0, 1.0));



/** \brief Build OSG geometry for a point cloud using a provided Geometry.
The supplied Geometry object must already have had buildPointCloud() called on it.
Copies the contents of the livescene::Geometry object's arrays, so it's slower, but simpler.
*/
LIVESCENE_EXPORT osg::Geode* buildOSGPointCloudCopy(const livescene::Geometry &geometry, osg::ref_ptr<osg::Geode> &geode, osg::Vec4 baseColor = osg::Vec4(1.0, 1.0, 1.0, 1.0));




/** \brief Build OSG geometry for a point cloud using a provided Geometry.
The supplied Geometry object must already have had buildPointCloud() called on it.
Utilizes the arrays already stored in the livescene::Geometry object without copying.
// <<<>>> this function does NOT currently work!
*/
LIVESCENE_EXPORT osg::Geode* buildOSGPolyMesh(const livescene::Geometry &geometry, osg::Vec4 baseColor = osg::Vec4(1.0, 1.0, 1.0, 1.0));



/** \brief Build OSG geometry for a point cloud using a provided Geometry.
The supplied Geometry object must already have had buildPointCloud() called on it.
Copies the contents of the livescene::Geometry object's arrays, so it's slower, but simpler.
*/
LIVESCENE_EXPORT osg::Geode* buildOSGPolyMeshCopy(const livescene::Geometry &geometry, osg::ref_ptr<osg::Geode> &geode, osg::Vec4 baseColor = osg::Vec4(1.0, 1.0, 1.0, 1.0));



/** \brief Make a matrix to convert from device coordinates to world space.
This code assumes world space is in meters.

This code is currently incomplete. Paramters for constructing the matrix should
come from the device, but are currently hardcoded.
\param width Width of a z image obtained from the camera (i.e., 640).
\param height Height of a z image obtained from the camera (i.e., 480).
\param depth Maximum depth value (i.e., 1024 for 10-bit, 2048 for 11-bit).
*/
LIVESCENE_EXPORT osg::Matrix makeDeviceToWorldMatrix( const int width, const int height, const int depth /*, TBD Device device */ );

/** \brief Transform x, y, z data point into OSG Vec3.
Compute a device coordinate vector (s, t, elementValue, 1. ),
then transform that by the specified matrix \c m. Results are assumed to be in clip coordinates,
and clip coord xyz values are divided by clip coord w to produve final eye coordinate xyz
values.

\param m Input. Typically obtained from the makeDeviceToWorldMatrix() function.
\param coordX Input. Raster X coordinate of sample.
\param coordY Input. Raster Y coordinate of sample.
\param valueZ Input. Depth value obtained from the z camera.
\see transform()
\return Contains the transformed output.
*/
LIVESCENE_EXPORT osg::Vec3 transformPoint( const osg::Matrix& m, const int &coordX, const int &coordY, const unsigned short &valueZ );

/** \brief Transform z image data into an OSG Vec3Array.
For each elements of \c imageZ, compute a device coordinate vector (s, t, elementValue, 1. ),
then transform that by the specified matrix \c m. Results are assumed to be in clip coordinates,
and clip coord xyz values are divided by clip coord w to produve final eye coordinate xyz
values. These are stored sequentially in \c vec.

This function resizes \c vec to hold the necessary number of vectors, as computed by
\c imageZ->getWidth() and \c imageZ->getHeight().

Calls \c transformPoint() to do the actual transform.

\param vec Output. Contains the transformed output. This function marks it as dirty.
\param m Input. Typically obtained from the makeDeviceToWorldMatrix() function.
\param imageZ Input. Array of depth values obtained from the z camera.
\param invalid Values in \c imageZ equal to this value are discarded.
\see transformPoint()
\return Number of valid values (not maxZ and not Image::NULL).
*/
LIVESCENE_EXPORT int transform( osg::Vec3Array* vec, const osg::Matrix& m, const livescene::Image imageZ, const unsigned short invalid=2047 );


// namespace livescene
}

// __LIVESCENE_OSGGEOMETRY_H__
#endif
