// Copyright 2011 Skew Matrix Software and AlphaPixel

#include "liblivescene/osgGeometry.h"
#include <osg/PrimitiveSet>
//#include <cassert>
#include <osg/io_utils>

namespace livescene {

	osg::Vec3 transformPointOSG( const osg::Matrix& m, const int &coordX, const int &coordY, const unsigned short &valueZ )
{
    osg::Vec4 deviceCoord( coordX, coordY, valueZ, 1.0 );
    osg::Vec4 clipCoord = deviceCoord * m;
	// clipCoord[2] always seems to equal -1.0
	//assert(clipCoord[2] == -1.0);
	float invCoordThree = 1.0f / clipCoord[3]; // multiply by inverse optimization
    osg::Vec3 eyeCoord( clipCoord[0] * invCoordThree,
        clipCoord[1] * invCoordThree, clipCoord[2] * invCoordThree );
    return(eyeCoord);
}

int transformOSG( osg::Vec3Array* vec, const osg::Matrix& m, const livescene::Image imageZ, const unsigned short invalid )
{
    const int width = imageZ.getWidth();
    const int height = imageZ.getHeight();
    const int numVectors = width * height;
    vec->resize( numVectors );

    // TBD How do we know this is unsigned short.
    unsigned short* dataPtr = ( unsigned short* )( imageZ.getData() );
    int sdx, tdx, vdx( 0 );
    for( tdx=0; tdx<height; ++tdx )
    {
        for( sdx=0; sdx<width; ++sdx )
        {
            unsigned short value = *dataPtr++;
            if( ( value != imageZ.getNull() ) && ( value != invalid ) )
            {
				(*vec)[ vdx++ ] = transformPointOSG(m, sdx, tdx, value);
            }
        }
    }
    vec->dirty();
    return( vdx );
}



// <<<>>> this function does NOT currently work!

LIVESCENE_EXPORT osg::Geode* buildOSGPointCloud(const livescene::Geometry &lsgeometry, osg::Vec4 baseColor)
{
	osg::ref_ptr<osg::Geode> geode;

	// create Geode
	if(geode = new osg::Geode())
	{
		// create Geometry
		osg::ref_ptr<osg::Geometry> geom(new osg::Geometry());
		if(geom.valid())
		{
			// add vertices using WrappedArrayShort3 class
			unsigned int numVertices = lsgeometry.getNumVertices();
			geom->setVertexArray(new WrappedArrayShort3(numVertices,lsgeometry.getVertices()));

			// add PrimitiveSet
			unsigned int numIndices = lsgeometry.getNumIndices();
			geom->addPrimitiveSet(new osg::DrawElementsUShort(osg::PrimitiveSet::POINTS,
															  numIndices,
															  // <<<>>> this should probably have been getIndices()
															  reinterpret_cast<unsigned short *>(lsgeometry.getVertices())));
		} // if

		geode->addDrawable( geom.get() );
	} // if

    return(geode.release());

} // buildOSGPointCloud






LIVESCENE_EXPORT osg::Geode* buildOSGPointCloudCopy(const livescene::Geometry &lsgeometry, osg::ref_ptr<osg::Geode> &geode, osg::Vec4 baseColor)
{
	osg::ref_ptr<osg::Geometry> osggeometry;
	osg::ref_ptr<osg::Vec3Array> vertices;
	osg::ref_ptr<osg::Vec2Array> texCoords;
	osg::ref_ptr<osg::DrawElementsUInt> elements;

	// create the Geode (Geometry Node) to contain all our osg::Geometry objects.
	if(!geode) // are we creating a fresh geode, as opposed to recycling one?
	{
		geode = new osg::Geode();
		// create Geometry object to store all the vertices and primitives.
		osggeometry = new osg::Geometry();
		// add the geometry to the geode.
		geode->addDrawable( osggeometry.get() );

		// create Arrays and elements.
		vertices = new osg::Vec3Array;
		//vertices->getVertexBufferObject()->setProfile(GL_DYNAMIC_DRAW);
		texCoords = new osg::Vec2Array;
		//texCoords->getVertexBufferObject()->setProfile(GL_DYNAMIC_DRAW);
		elements = new osg::DrawElementsUInt(osg::PrimitiveSet::POINTS);

		// create the color of the geometry, one single for the whole geometry.
		osg::Vec4Array* colors = new osg::Vec4Array;
		colors->push_back(baseColor);

		// pass the color array to geometry, note the binding to tell the geometry
		// that only use one color for the whole object.
		osggeometry->setColorArray(colors);
		osggeometry->setColorBinding(osg::Geometry::BIND_OVERALL);        

		// set the normal in the same way as the color.
		osg::Vec3Array* normals = new osg::Vec3Array;
		normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
		osggeometry->setNormalArray(normals);
		osggeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

		osggeometry->setUseDisplayList( false ); // Turn off; on by default.
		osggeometry->setUseVertexBufferObjects(true); // makes a significant difference in performance

		// pass the created vertex array to the geometry object.
		osggeometry->setVertexArray( vertices.get() );

		// pass the created texCoord array
		osggeometry->setTexCoordArray( 0, texCoords.get() );

		osggeometry->addPrimitiveSet( elements.get() );
	} // if
	else
	{
		// access existing geometry object
		osggeometry = dynamic_cast<osg::Geometry*>(geode->getDrawable(0));

		// use existing vertices, texCoords
		vertices = dynamic_cast<osg::Vec3Array *>(osggeometry->getVertexArray());
		texCoords = dynamic_cast<osg::Vec2Array *>(osggeometry->getTexCoordArray(0));
		vertices->clear();
		vertices->dirty();
		vertices->getVertexBufferObject()->dirty();
		texCoords->clear();
		texCoords->dirty();
		texCoords->getVertexBufferObject()->dirty();

		// use existing primitiveset
		osg::ref_ptr<osg::PrimitiveSet> primitiveSet;
		primitiveSet = osggeometry->getPrimitiveSet(0);

		if(primitiveSet.valid() && primitiveSet->getType() == osg::PrimitiveSet::DrawElementsUIntPrimitiveType)
		{
			// get access to drawelements
			elements = dynamic_cast<osg::DrawElementsUInt *>(primitiveSet->getDrawElements());
			// clear existing data
			elements->clear();
		} // if
		primitiveSet->dirty();
	} // else

	// create TRIANGLES
	if(lsgeometry.getEntityType() == livescene::Geometry::GEOMETRY_POINTS)
	{
		// pre-size arrays for better performance
		vertices->reserve(lsgeometry.getNumVertices());
		texCoords->reserve(lsgeometry.getNumTexCoords());
		elements->reserve(lsgeometry.getNumIndices());

		short *shortVertices = lsgeometry.getVertices();
		float *floatTexCoords = lsgeometry.getTexCoord();
		for(unsigned int vertexLoop(0); vertexLoop < lsgeometry.getNumVertices(); ++vertexLoop)
		{ // <<<>>> can this be made faster in bulk?
			vertices->push_back(osg::Vec3(shortVertices[vertexLoop * 3], shortVertices[vertexLoop * 3 + 1], shortVertices[vertexLoop * 3 + 2]));
			texCoords->push_back(osg::Vec2(floatTexCoords[vertexLoop * 2], floatTexCoords[vertexLoop * 2 + 1]));
			elements->push_back(vertexLoop);
		} // for
	} // if

	return(geode.get());

} // buildOSGPointCloudCopy




LIVESCENE_EXPORT osg::Geode* buildOSGPolyMeshCopy(const livescene::Geometry &lsgeometry, osg::ref_ptr<osg::Geode> &geode, osg::Vec4 baseColor)
{
	osg::ref_ptr<osg::Geometry> osggeometry;
	osg::ref_ptr<osg::Vec3Array> vertices;
	osg::ref_ptr<osg::Vec2Array> texCoords;
	osg::ref_ptr<osg::DrawElementsUInt> elements;

    // create the Geode (Geometry Node) to contain all our osg::Geometry objects.
	if(!geode) // are we creating a fresh geode, as opposed to recycling one?
	{
	    geode = new osg::Geode();
        // create Geometry object to store all the vertices and primitives.
        osggeometry = new osg::Geometry();
		// add the geometry to the geode.
        geode->addDrawable( osggeometry.get() );

		// create Arrays and elements.
		vertices = new osg::Vec3Array;
		//vertices->getVertexBufferObject()->setProfile(GL_DYNAMIC_DRAW);
		texCoords = new osg::Vec2Array;
		//texCoords->getVertexBufferObject()->setProfile(GL_DYNAMIC_DRAW);
		elements = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);

		// create the color of the geometry, one single for the whole geometry.
		osg::Vec4Array* colors = new osg::Vec4Array;
		colors->push_back(baseColor);

		// pass the color array to geometry, note the binding to tell the geometry
		// that only use one color for the whole object.
		osggeometry->setColorArray(colors);
		osggeometry->setColorBinding(osg::Geometry::BIND_OVERALL);        

		// set the normal in the same way as the color.
		osg::Vec3Array* normals = new osg::Vec3Array;
		normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
		osggeometry->setNormalArray(normals);
		osggeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

		osggeometry->setUseDisplayList( false ); // Turn off; on by default.
		osggeometry->setUseVertexBufferObjects(true); // makes a significant difference in performance

		// pass the created vertex array to the geometry object.
		osggeometry->setVertexArray( vertices.get() );

		// pass the created texCoord array
		osggeometry->setTexCoordArray( 0, texCoords.get() );

		osggeometry->addPrimitiveSet( elements.get() );
	} // if
	else
	{
		// access existing geometry object
		osggeometry = dynamic_cast<osg::Geometry*>(geode->getDrawable(0));

		// use existing vertices, texCoords
		vertices = dynamic_cast<osg::Vec3Array *>(osggeometry->getVertexArray());
		texCoords = dynamic_cast<osg::Vec2Array *>(osggeometry->getTexCoordArray(0));
		vertices->clear();
		vertices->dirty();
		vertices->getVertexBufferObject()->dirty();
		texCoords->clear();
		texCoords->dirty();
		texCoords->getVertexBufferObject()->dirty();

		// use existing primitiveset
		osg::ref_ptr<osg::PrimitiveSet> primitiveSet;
		primitiveSet = osggeometry->getPrimitiveSet(0);

		if(primitiveSet.valid() && primitiveSet->getType() == osg::PrimitiveSet::DrawElementsUIntPrimitiveType)
		{
			// get access to drawelements
			elements = dynamic_cast<osg::DrawElementsUInt *>(primitiveSet->getDrawElements());
			// clear existing data
			elements->clear();
		} // if
		primitiveSet->dirty();
	} // else

    // create TRIANGLES
	if(lsgeometry.getEntityType() == livescene::Geometry::GEOMETRY_FACES)
    {
		// pre-size arrays for better performance
		vertices->reserve(lsgeometry.getNumVertices());
		texCoords->reserve(lsgeometry.getNumTexCoords());
		elements->reserve(lsgeometry.getNumIndices());
		
		short *shortVertices = lsgeometry.getVertices();
		float *floatTexCoords = lsgeometry.getTexCoord();
		unsigned int *intIndices = lsgeometry.getIndices();
		for(unsigned int vertexLoop(0); vertexLoop < lsgeometry.getNumVertices(); ++vertexLoop)
		{ // <<<>>> can this be made faster in bulk?
			vertices->push_back(osg::Vec3(shortVertices[vertexLoop * 3], shortVertices[vertexLoop * 3 + 1], shortVertices[vertexLoop * 3 + 2]));
			texCoords->push_back(osg::Vec2(floatTexCoords[vertexLoop * 2], floatTexCoords[vertexLoop * 2 + 1]));
		} // for
		for(unsigned int idxLoop(0); idxLoop < lsgeometry.getNumIndices(); ++idxLoop) // <<<>>> can this be made faster in bulk?
		{
			elements->push_back(intIndices[idxLoop]);
		} // for      
    } // if

    return(geode.get());

} // buildOSGPolyMeshCopy



osg::Matrix makeDeviceToWorldMatrixOSG( const int width, const int height, const int depth /*, TBD Device device */ )
{
#if 0
    osg::Matrix freenectMatrix;
    {
	    // code from libfreenect's glpclview

	    // Do the projection from u,v,depth to X,Y,Z directly in an opengl matrix
	    // These numbers come from a combination of the ros kinect_node wiki, and
	    // nicolas burrus' posts.
        //
        // The hardcoded magic numbers  come from a computer vision calibration process.
        // fx,fy correspond to the focal distance and cx,cy to the estimated center
        // point of the images output by the camera. a and b are coefficients that cause
        // the results to be in meters. For more info:
        //   http://nicolas.burrus.name/index.php/Research/KinectCalibration

        float fx = 594.21f;
        float fy = 591.04f;
        float a = -0.0030711f;
        float b = 3.3309495f;
        float cx = 339.5f;
        float cy = 242.7f;
        GLfloat mat[16] = {
            1/fx,     0,  0, 0,
            0,    -1/fy,  0, 0,
            0,       0,  0, a,
            -cx/fx, cy/fy, -1, b
        };

        freenectMatrix = osg::Matrix( mat[ 0], mat[ 1], mat[ 2], mat[ 3],
		                        mat[ 4], mat[ 5], mat[ 6], mat[ 7],
							    mat[ 8], mat[ 9], mat[10], mat[11],
							    mat[12], mat[13], mat[14], mat[15] );
        osg::notify( osg::ALWAYS ) << "freenectMatrix: " << freenectMatrix;

        // Code to decompose their matrix into a simple projection.
        {
            osg::Matrix tW = osg::Matrix::scale( width/2., height/2., depth/2. );
            osg::Matrix tS = osg::Matrix::scale( 1., -1., 1. );
            osg::Matrix tT = osg::Matrix::translate( 1., -1., 1. );
            osg::Matrix tP = tW * freenectMatrix;
            tP = tS * tP;
            tP = tT * tP;
            tP = osg::Matrix::inverse( tP );
            osg::notify( osg::ALWAYS ) << "Depth: " << depth << std::endl;
            osg::notify( osg::ALWAYS ) << "Their proj " << tP << std::endl;
            double l, r, b, t, n, f;
            tP.getFrustum( l, r, b, t, n, f );
            osg::notify( osg::ALWAYS ) << "  Left: " << l << " Right: " << r << std::endl;
            osg::notify( osg::ALWAYS ) << "  Bottom: " << b << " Top: " << t << std::endl;
            osg::notify( osg::ALWAYS ) << "  Near: " << n << " Far: " << f << std::endl;
        }
    }
#endif


    // I derived the near and far values by subtracting the
    // post-projection transforms out of the freenect matrix,
    // then decomposing the result with osg::Matrix::getFrustum().
    // The negative far plane is quite unexpected.
    const bool y0IsUp( true ); // y==0 is at the top in Kinect, so this is true.
    const float fovy( 48.f ); // Empirical.
    const float near( 0.3f ); // Derived from freenect matrix.
#if 1
    const float far( (depth == 1023) ? 5.285f : -.338f ); // TBD Derived from freenect matrix.

    // Convert from window coords in range (0,0,0)-(width,height,depth)
    //   to biased NDC space in renage (0,0,0)-(2,2,2)
    osg::Matrix invWindow( osg::Matrix::scale( 2./width, 2./height, 2./depth ) );
    // Currently this works for 10- or 11-bit depth data. Warn otherwise.
    if( ( depth != 2047 ) && ( depth != 1023 ) )
        osg::notify( osg::WARN ) << "makeDeviceToWorldMatrixOSG: Depth values must be 1023 or 2047. Results are undefined." << std::endl;
#else
    // TBD switch to this code in the future. Appears to work for either
    //   10- or 11-bit depth data.
    //   TBD remove the depth parameter, don't need it anymore.
    // OK, this makes a little more sense: Always assume far plane it at
    // 5.285 meters, and always use a window transform with a device coord
    // z space of 0-1023.
    const float far( 5.285f ); // TBD Derived from freenect matrix.

    // Convert from window coords in range (0,0,0)-(width,height,1023)
    //   to biased NDC space in renage (0,0,0)-(2,2,2)
    osg::Matrix invWindow( osg::Matrix::scale( 2./width, 2./height, 2./1023. ) );
#endif

    osg::Matrix yNegativeScale, invNDC;
    if( y0IsUp )
    {
        // Kinect has y=0 at the top, so mirror y.
        //   The range is now (0,0,0)-(2,-2,2)
        yNegativeScale = osg::Matrix::scale( 1., -1., 1. );
        // Translate into NDC space (-1,-1,-1)-(1,1,1)
        invNDC = osg::Matrix::translate( -1., 1., -1. );
    }
    else
    {
        // No-op.
        yNegativeScale = osg::Matrix::identity();
        // Translate into NDC space (-1,-1,-1)-(1,1,1)
        invNDC = osg::Matrix::translate( -1., -1., -1. );
    }

    // Create what we think is the projection matrix:
    double aspect = (double)width / (double)height;
    osg::Matrix proj( osg::Matrix::perspective( fovy, aspect, near, far ) );
    // ...and invert it:
    osg::Matrix invProj( osg::Matrix::inverse( proj ) );

    osg::Matrix returnMatrix = invWindow * yNegativeScale * invNDC * invProj;
    //osg::notify( osg::ALWAYS ) << "Our matrix: " << returnMatrix;

    return( returnMatrix );
}


LIVESCENE_EXPORT osg::MatrixTransform* buildOSGTextureMatrixTransform(void)
{
	return(0); // <<<>>>
} // buildOSGTextureMatrixTransform


// namespace livescene
}
