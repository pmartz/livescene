// Copyright 2011 Skew Matrix Software and Alpha Pixel

#include <libfreenect_sync.h>

#include <liblivescene/Version.h>
#include <iostream>

#ifdef OSGWORKS_FOUND
#  include <osgwTools/Shapes.h>
#  include <osgwTools/Version.h>
#endif

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osg/Texture2D>


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


int main()
{
    std::cout << livescene::getVersionString() << std::endl;

#ifdef OSGWORKS_FOUND
    std::cout << osgwTools::getVersionString() << std::endl;
#endif

    osg::ref_ptr< osg::Image > image = new osg::Image;

    osg::ref_ptr< osg::Texture2D > tex = new osg::Texture2D;
    tex->setResizeNonPowerOfTwoHint( false );
    tex->setTextureSize( FREENECT_FRAME_W, FREENECT_FRAME_H );
    tex->setImage( image.get() );

    osgViewer::Viewer viewer;
    viewer.setThreadingModel( osgViewer::ViewerBase::SingleThreaded );
    viewer.setUpViewInWindow( 30, 30, 800, 600 );
    viewer.setSceneData( createScene( tex.get() ) );

    osgGA::TrackballManipulator* tbm = new osgGA::TrackballManipulator();
    viewer.setCameraManipulator( tbm );

    while( !viewer.done() )
    {
        uint32_t timestamp = viewer.getFrameStamp()->getFrameNumber();
        unsigned char* buffer;
        freenect_sync_get_video( (void**)&buffer, &timestamp, 0, FREENECT_VIDEO_RGB );

        image->setImage( FREENECT_FRAME_W, FREENECT_FRAME_H, 0, GL_RGB,
            GL_RGB, GL_UNSIGNED_BYTE, buffer, osg::Image::USE_NEW_DELETE );

        viewer.frame();
    }

    return( 0 );
}
