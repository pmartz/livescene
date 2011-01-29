// Copyright 2011 Skew Matrix Software and AlphaPixel

#include <libfreenect_sync.h>

#include <liblivescene/Version.h>
#include <iostream>

#ifdef OSGWORKS_FOUND
#  include <osgwTools/Shapes.h>
#  include <osgwTools/Version.h>
#else
#  include <osg/Geometry>
#endif

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osg/Texture2D>
#include <osg/io_utils>


osg::Node* createScene( osg::Texture2D* tex )
{
    osg::ref_ptr< osg::Geode > geode = new osg::Geode;
#ifdef OSGWORKS_FOUND
    geode->addDrawable( osgwTools::makePlane( osg::Vec3( -1., 0., -1. ),
        osg::Vec3( 2., 0., 0. ), osg::Vec3( 0., 0., 2. ) ) );
#else
    geode->addDrawable( osg::createTexturedQuadGeometry( osg::Vec3( -1., 0., -1. ),
        osg::Vec3( 2., 0., 0. ), osg::Vec3( 0., 0., 2. ) ) );
#endif

    osg::StateSet* stateSet = geode->getOrCreateStateSet();
    stateSet->setTextureAttributeAndModes( 0, tex );
    stateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

    return( geode.release() );
}


void characteristics( unsigned short* buf )
{
    unsigned short* ptr = buf;
    unsigned short minVal( 0xffff );
    unsigned short maxVal( 0 );
    osg::Vec2s minLoc, maxLoc;

    // TBD ignore rightmost 8 columns, they are always max distance.
    unsigned short sdx, tdx;
    for( tdx=0; tdx<FREENECT_FRAME_H; tdx++ )
    {
        for( sdx=0; sdx<FREENECT_FRAME_W; sdx++ )
        {
            if( sdx > FREENECT_FRAME_W-8 )
            {
                // rightmost 8 columns are always max depth. Ignore them.
                ptr++;
                continue;
            }
            if( *ptr > maxVal )
            {
                maxVal = *ptr;
                maxLoc.set( sdx, tdx );
            }
            if( *ptr < minVal )
            {
                minVal = *ptr;
                minLoc.set( sdx, tdx );
            }
            ptr++;
        }
    }
    std::cout << "Max depth value: " << maxVal << " at " << maxLoc << std::endl;
    std::cout << "  Min depth value: " << minVal << " at " << minLoc << std::endl;
}
void scale( unsigned short* buf )
{
    unsigned short* ptr = buf;

    unsigned short sdx, tdx;
    for( tdx=0; tdx<FREENECT_FRAME_H; tdx++ )
    {
        for( sdx=0; sdx<FREENECT_FRAME_W; sdx++ )
        {
            *ptr <<= 6;
            ptr++;
        }
    }
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
    viewer.setSceneData( createScene( tex.get() ) );

    osgGA::TrackballManipulator* tbm = new osgGA::TrackballManipulator();
    viewer.setCameraManipulator( tbm );

    while( !viewer.done() )
    {
        uint32_t ts;
        unsigned short* buffer( NULL );
        freenect_sync_get_depth( (void**)&buffer, &ts, 0, FREENECT_DEPTH_10BIT );
        characteristics( buffer );
        scale( buffer );

        image->setImage( FREENECT_FRAME_W, FREENECT_FRAME_H, 0, GL_INTENSITY16,
            GL_LUMINANCE, GL_UNSIGNED_SHORT, reinterpret_cast< unsigned char* >( buffer ), osg::Image::NO_DELETE );

        viewer.frame();
    }

    freenect_sync_stop();

    return( 0 );
}
