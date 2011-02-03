// Copyright 2011 Skew Matrix Software and AlphaPixel

#include "liblivescene/osgGeometry.h"

namespace livescene {

LIVESCENE_EXPORT osg::Geode* buildOSGPointCloud(const livescene::Geometry &geometry)
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
			unsigned int numVertices = geometry.getNumVertices();
			geom->setVertexArray(new WrappedArrayShort3(numVertices,geometry.getVertices()));

			// add PrimitiveSet
			unsigned int numIndices = geometry.getNumIndices();
			geom->addPrimitiveSet(new osg::DrawElementsUShort(osg::PrimitiveSet::POINTS,
															  numIndices,
															  reinterpret_cast<unsigned short *>(geometry.getVertices())));
		} // if

		geode->addDrawable( geom.get() );
	} // if

    return(geode.release());

} // buildOSGPointCloud


/*
	// create Geode
	if(geode = new osg::Geode())
	{
		// create Geometry
		osg::ref_ptr<osg::Geometry> geom(new osg::Geometry());
		if(geom.valid())
		{
			// add vertices using WrappedArrayShort3 class
			unsigned int numVertices = geometry.getNumVertices();
			geom->setVertexArray(new WrappedArrayShort3(numVertices,geometry.getVertices()));

			// add PrimitiveSet
			unsigned int numIndices = geometry.getNumIndices();
			geom->addPrimitiveSet(new osg::DrawElementsUShort(osg::PrimitiveSet::POINTS,
															  numIndices,
															  reinterpret_cast<unsigned short *>(geometry.getVertices())));
		} // if

		geode->addDrawable( geom.get() );
	} // if

*/

LIVESCENE_EXPORT osg::Geode* buildOSGPointCloudCopy(const livescene::Geometry &geometry)
{
	// code taken from osg's example "osggeometry"
	osg::ref_ptr<osg::Geode> geode;

    // create the Geode (Geometry Node) to contain all our osg::Geometry objects.
    geode = new osg::Geode();

    // create POINTS
    {
        // create Geometry object to store all the vertices and points primitive.
        osg::Geometry* pointsGeom = new osg::Geometry();


        // create a Vec3Array and add to it all my coordinates.
        // Like all the *Array variants (see include/osg/Array) , Vec3Array is derived from both osg::Array 
        // and std::vector<>.  osg::Array's are reference counted and hence sharable,
        // which std::vector<> provides all the convenience, flexibility and robustness
        // of the most popular of all STL containers.
        osg::Vec3Array* vertices = new osg::Vec3Array;
		osg::Vec2Array* texCoords = new osg::Vec2Array;
		short *shortVertices = geometry.getVertices();
		float *floatTexCoords = geometry.getTexCoord();
		for(unsigned int vertexLoop = 0; vertexLoop < geometry.getNumVertices(); vertexLoop++)
		{
	        vertices->push_back(osg::Vec3(shortVertices[vertexLoop * 3], shortVertices[vertexLoop * 3 + 1], shortVertices[vertexLoop * 3 + 2]));
			texCoords->push_back(osg::Vec2(floatTexCoords[vertexLoop * 2], floatTexCoords[vertexLoop * 2 + 1]));
		} // for
        
        // pass the created vertex array to the points geometry object.
        pointsGeom->setVertexArray(vertices);
        
        // pass the created texCoord array
        pointsGeom->setTexCoordArray(0, texCoords);

		// create the color of the geometry, one single for the whole geometry.
        // for consistency of design even one single color must added as an element
        // in a color array.
        osg::Vec4Array* colors = new osg::Vec4Array;
        // add a white color, colors take the form r,g,b,a with 0.0 off, 1.0 full on.
        colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
        
        // pass the color array to points geometry, note the binding to tell the geometry
        // that only use one color for the whole object.
        pointsGeom->setColorArray(colors);
        pointsGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
        
        
        // set the normal in the same way color.
        osg::Vec3Array* normals = new osg::Vec3Array;
        normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
        pointsGeom->setNormalArray(normals);
        pointsGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);


        // create and add a DrawArray Primitive (see include/osg/Primitive).  The first
        // parameter passed to the DrawArrays constructor is the Primitive::Mode which
        // in this case is POINTS (which has the same value GL_POINTS), the second
        // parameter is the index position into the vertex array of the first point
        // to draw, and the third parameter is the number of points to draw.
        pointsGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS,0,vertices->size()));
        
        
        // add the points geometry to the geode.
        geode->addDrawable(pointsGeom);
    }


    return(geode.release());

} // buildOSGPointCloud





LIVESCENE_EXPORT osg::MatrixTransform* buildOSGGeometryMatrixTransform(void)
{
#if 0
	// code from libfreenect's glpclview

	// Do the projection from u,v,depth to X,Y,Z directly in an opengl matrix
	// These numbers come from a combination of the ros kinect_node wiki, and
	// nicolas burrus' posts.
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

	osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
	osg::Matrix magicVertex(mat[ 0], mat[ 1], mat[ 2], mat[ 3],
		                    mat[ 4], mat[ 5], mat[ 6], mat[ 7],
							mat[ 8], mat[ 9], mat[10], mat[11],
							mat[12], mat[13], mat[14], mat[15]);
	mt->setMatrix(magicVertex);
	return(mt.release());
#else

    // These values are control parameters that will vary from
    // one device to the next. Currently putting in values that
    // closely approximate the Kinect, resulting in a net transform
    // matrix that is pretty close to the "magic matrix" used in
    // the OpenKinect project.
    const bool y0IsUp( true ); // y==0 is at the top in Kinect, so this is true.
    const float width( 640.f );
    const float height( 480.f );
    const float depth( 1024.f ); // assumes 10-bit! Change to 2048 for 11-bit.
    const float depthValuesPerMeter( 500.f ); // TBD a Guess
    const float fovy( 40.f ); // TBD a guess
    const float near( 0.1f ); // TBD a guess
    const float far( depth / depthValuesPerMeter );

    // Convert from window coords in range (0,0,0)-(width,height,depth)
    //   to biased NDC space in renage (0,0,0)-(2,2,2)
    osg::Matrix invWindow( osg::Matrix::scale( 2./width, 2./height, 2./depth ) );

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
    osg::Matrix proj( osg::Matrix::perspective( fovy, width/height, near, far ) );
    // ...and invert it:
    osg::Matrix invProj( osg::Matrix::inverse( proj ) );

    osg::Matrix matrix = invWindow * yNegativeScale * invNDC * invProj;

	osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
	mt->setMatrix( matrix );

	return( mt.release() );

#endif
} // buildOSGGeometryMatrixTransform

LIVESCENE_EXPORT osg::MatrixTransform* buildOSGTextureMatrixTransform(void)
{
	return(0); // <<<>>>
} // buildOSGTextureMatrixTransform


// namespace livescene
}
