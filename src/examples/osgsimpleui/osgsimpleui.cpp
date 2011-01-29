// Copyright 2011 Skew Matrix Software and AlphaPixel

#include <libfreenect_sync.h>

#include <liblivescene/Version.h>
#include <iostream>

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
    viewer.setSceneData( createScene( tex.get() ) );

    viewer.setUpViewInWindow( 30, 30, 1280, 1024 );
    viewer.getCamera()->setProjectionMatrix( osg::Matrix::ortho( 0., 1279., 0., 1023., -1., 1. ) );
    viewer.getCamera()->setViewMatrix( osg::Matrix::identity() );

    while( !viewer.done() )
    {
        uint32_t ts;
        unsigned char* buffer( NULL );
        freenect_sync_get_video( (void**)&buffer, &ts, 0, FREENECT_VIDEO_RGB );

        image->setImage( FREENECT_FRAME_W, FREENECT_FRAME_H, 0, GL_RGB,
            GL_RGB, GL_UNSIGNED_BYTE, buffer, osg::Image::NO_DELETE );

        viewer.frame();
    }

    freenect_sync_stop();

    return( 0 );
}
