// Copyright 2011 Skew Matrix Software and AlphaPixel

#include <libfreenect_sync.h>

#include <liblivescene/Version.h>
#include <iostream>
#include <deque>

// For testing:
//#undef OSGWORKS_FOUND

#ifdef OSGWORKS_FOUND
#  include <osgwTools/Shapes.h>
#  include <osgwTools/Version.h>
#endif

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osg/Texture2D>
#include <osg/Geometry>
#include <osg/MatrixTransform>

osg::Node* createScene( osg::Texture2D* tex )
{
    osg::ref_ptr< osg::Geode > geode = new osg::Geode;
    osg::Geometry* geom = new osg::Geometry;

    osg::Vec3Array* v = new osg::Vec3Array;
    osg::Vec2Array* tc = new osg::Vec2Array;

#ifdef OSGWORKS_FOUND
    geom = osgwTools::makePlane( osg::Vec3( -1., 0., -1. ),
        osg::Vec3( 2., 0., 0. ), osg::Vec3( 0., 0., 2. ) );

    // It's a tri strip
    (*v).push_back( osg::Vec3( 0., 0., 0. ) );
    (*v).push_back( osg::Vec3( 640., 0., 0. ) );
    (*v).push_back( osg::Vec3( 0., 480., 0. ) );
    (*v).push_back( osg::Vec3( 640., 480., 0. ) );

    (*tc).push_back( osg::Vec2( 0., 1. ) );
    (*tc).push_back( osg::Vec2( 1., 1. ) );
    (*tc).push_back( osg::Vec2( 0., 0. ) );
    (*tc).push_back( osg::Vec2( 1., 0. ) );
#else
    geom = osg::createTexturedQuadGeometry( osg::Vec3( -1., 0., -1. ),
        osg::Vec3( 2., 0., 0. ), osg::Vec3( 0., 0., 2. ) );

    // It's a quad
    (*v).push_back( osg::Vec3( 0., 0., 0. ) );
    (*v).push_back( osg::Vec3( 640., 0., 0. ) );
    (*v).push_back( osg::Vec3( 640., 480., 0. ) );
    (*v).push_back( osg::Vec3( 0., 480., 0. ) );

    (*tc).push_back( osg::Vec2( 0., 1. ) );
    (*tc).push_back( osg::Vec2( 1., 1. ) );
    (*tc).push_back( osg::Vec2( 1., 0. ) );
    (*tc).push_back( osg::Vec2( 0., 0. ) );
#endif
    geom->setVertexArray( v );
    geom->setTexCoordArray( 0, tc );

    geode->addDrawable( geom );

    osg::StateSet* stateSet = geode->getOrCreateStateSet();
    stateSet->setTextureAttributeAndModes( 0, tex );
    stateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

    return( geode.release() );
}

osg::Node* square()
{
    osg::ref_ptr< osg::Geode > geode = new osg::Geode;
    osg::Geometry* geom = new osg::Geometry;
    double extent( 10. );
#ifdef OSGWORKS_FOUND
    geom = osgwTools::makePlane( osg::Vec3( -extent, -extent, 0. ),
        osg::Vec3( 2.*extent, 0., 0. ), osg::Vec3( 0., 2.*extent, 0. ) );
#else
    geom = osg::createTexturedQuadGeometry( osg::Vec3( -extent, -extent., 0. ),
        osg::Vec3( 2.*extent, 0., 0. ), osg::Vec3( 0., 2.*extent, 0. ) );
#endif
    geode->addDrawable( geom );

    osg::StateSet* stateSet = geode->getOrCreateStateSet();
    stateSet->setRenderBinDetails( 1, "RenderBin" );
    stateSet->setMode( GL_DEPTH_TEST, false );
    stateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

    return( geode.release() );
}


osg::Vec2s getMinLocation( unsigned short* buf )
{
    unsigned short* ptr = buf;
    unsigned short minVal( 0xffff );
    osg::Vec2s minLoc;

    unsigned short sdx, tdx;
    for( tdx=0; tdx<FREENECT_FRAME_H; tdx++ )
    {
        for( sdx=0; sdx<FREENECT_FRAME_W; sdx++ )
        {
            if( *ptr < minVal )
            {
                minVal = *ptr;
                minLoc.set( sdx, tdx );
            }
            ptr++;
        }
    }
    return( minLoc );
}


typedef std::deque< osg::Vec2f > Vec2Vec;
Vec2Vec smooth;

osg::Vec2 scaleAndSmooth( const osg::Vec2s& loc, float width, float height )
{
    // Scale to full viewport.
    float sx = width / (float)FREENECT_FRAME_W;
    float sy = height / (float)FREENECT_FRAME_H;
    osg::Vec2f newLoc;
    newLoc[0] = (float)loc[0] * sx;
    newLoc[1] = (float)loc[1] * sy;

    // Invert into window space
    newLoc[0] = width - newLoc[0];
    newLoc[1] = height - newLoc[1];

    // Add to the smoothing vector (last n locations)
    const int n( 4 ); // 10 seemed a bit sluggish.
    smooth.push_back( newLoc );
    while( smooth.size() > n )
        smooth.pop_front();

    // Compute total of all locations
    osg::Vec2 total;
    Vec2Vec::const_iterator itr;
    for( itr=smooth.begin(); itr!=smooth.end(); itr++ )
        total += *itr;

    // Return average location
    return( total / smooth.size() );
}

int main()
{
    std::cout << livescene::getVersionString() << std::endl;

#ifdef OSGWORKS_FOUND
    std::cout << osgwTools::getVersionString() << std::endl;
#endif

    osg::ref_ptr< osg::Image > image = new osg::Image;
    image->setDataVariance( osg::Object::DYNAMIC );

    osg::ref_ptr< osg::Texture2D > tex = new osg::Texture2D;
    tex->setDataVariance( osg::Object::DYNAMIC );
    tex->setResizeNonPowerOfTwoHint( false );
    tex->setTextureSize( FREENECT_FRAME_W, FREENECT_FRAME_H );
    tex->setImage( image.get() );

    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgViewer::StatsHandler() );
    viewer.setThreadingModel( osgViewer::ViewerBase::SingleThreaded );

    viewer.setUpViewInWindow( 30, 30, 800, 600 );
    float width( 1280 ), height( 1024 );
    viewer.getCamera()->setProjectionMatrix( osg::Matrix::ortho( 0., width-1., 0., height-1., -1., 1. ) );
    viewer.getCamera()->setViewMatrix( osg::Matrix::identity() );

    osg::ref_ptr< osg::Group > root = new osg::Group;
    root->addChild( createScene( tex.get() ) );
    viewer.setSceneData( root.get() );

    osg::ref_ptr< osg::MatrixTransform > mt = new osg::MatrixTransform;
    mt->addChild( square() );
    root->addChild( mt.get() );

    while( !viewer.done() )
    {
        uint32_t ts;
        unsigned char* rgb( NULL );
        freenect_sync_get_video( (void**)&rgb, &ts, 0, FREENECT_VIDEO_RGB );

        unsigned short* depth( NULL );
        freenect_sync_get_depth( (void**)&depth, &ts, 0, FREENECT_DEPTH_11BIT );
        osg::Vec2s loc = getMinLocation( depth );
        osg::Vec2 pos = scaleAndSmooth( loc, width, height );
        mt->setMatrix( osg::Matrix::translate( pos.x(), pos.y(), 0. ) );

        image->setImage( FREENECT_FRAME_W, FREENECT_FRAME_H, 0, GL_RGB,
            GL_RGB, GL_UNSIGNED_BYTE, rgb, osg::Image::NO_DELETE );

        viewer.frame();
    }

    freenect_sync_stop();

    return( 0 );
}
