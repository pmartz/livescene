// Copyright 2011 Skew Matrix Software and AlphaPixel

#include <liblivescene/Version.h>
#include <liblivescene/DeviceManager.h>
#include <liblivescene/DeviceCapabilities.h>
#include <liblivescene/osgGeometry.h>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/WriteFile>
#include <osgGA/TrackballManipulator>
#include <osg/Geometry>

#include <iostream>
#include <osg/io_utils>


osg::ref_ptr< osg::Vec3Array > vecArray;
osg::ref_ptr< osg::Geometry > geom;

osg::Node* createScene()
{
    osg::ref_ptr< osg::Group > root = new osg::Group;

    osg::ref_ptr< osg::Geode > geode = new osg::Geode;
    root->addChild( geode.get() );
    osg::StateSet* stateSet = geode->getOrCreateStateSet();
    stateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

    geom = new osg::Geometry;
    geom->setDataVariance( osg::Object::DYNAMIC );
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );

    vecArray = new osg::Vec3Array;
    vecArray->setDataVariance( osg::Object::DYNAMIC );
    geom->setVertexArray( vecArray.get() );

    osg::Vec4Array* c = new osg::Vec4Array;
    c->push_back( osg::Vec4( 1., 1., 1., 1. ) );
    geom->setColorArray( c );
    geom->setColorBinding( osg::Geometry::BIND_OVERALL );

    geom->addPrimitiveSet( new osg::DrawArrays( GL_POINTS, 0, 640*480 ) );
    geode->addDrawable( geom.get() );

    return( root.release() );
}


int main()
{
    std::cout << livescene::getVersionString() << std::endl;


    livescene::DeviceManager *deviceManager = new livescene::DeviceManager();
    livescene::FactoryCollection collection;
    livescene::DeviceBase *genericDevice(NULL);
    livescene::DeviceCapabilitiesImage *imageCapabilitiesZ(NULL);

    livescene::StringContainer capabilityCriteria;
    capabilityCriteria.push_back("IMAGE_Z_RESOLUTION_640x480"); // Kinect basic Z res
    if( !( deviceManager->enumDevicesByCapability( collection, capabilityCriteria ) ) )
    {
        std::cerr << "Can't find DeviceFreenect." << std::endl;
        return( 1 );
    }
    if( collection.empty() )
    {
        std::cerr << "Empty collection." << std::endl;
        return( 1 );
    }

    genericDevice = collection[0]->createDevice( 0, capabilityCriteria );
    if( genericDevice == NULL )
    {
        std::cerr << "Can't create generic device." << std::endl;
        return( 1 );
    }
    imageCapabilitiesZ = static_cast<livescene::DeviceCapabilitiesImage *> (genericDevice->requestCapabilityInterface("IMAGE_Z"));


    osg::ref_ptr< osg::Node > root = createScene();

    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgViewer::StatsHandler() );
    viewer.setThreadingModel( osgViewer::ViewerBase::SingleThreaded );
    viewer.setUpViewInWindow( 30, 30, 800, 600 );
    viewer.setSceneData( root.get() );

    osgGA::TrackballManipulator* tbm = new osgGA::TrackballManipulator();
    viewer.setCameraManipulator( tbm );

    const int nominalFrameW( 640 ), nominalFrameH( 480 ), nominalFrameD( 1024 );
    osg::Matrix d2w = livescene::makeDeviceToWorldMatrix( nominalFrameW, nominalFrameH, nominalFrameD /*, TBD Device device */ );

    bool firstFrame( true );
    while( !viewer.done() )
    {
        livescene::Image imageZ( nominalFrameW, nominalFrameH, 2, livescene::DEPTH_10BIT );
        imageCapabilitiesZ->getImageSync( imageZ );

        livescene::transform( vecArray.get(), d2w, imageZ );

        if( firstFrame )
        {
            osgDB::writeNodeFile( *root, "out.ive" );

            tbm->home( 0 );
            firstFrame = false;
        }
        viewer.frame();
    }

    if( imageCapabilitiesZ )
    {
        genericDevice->releaseCapabilityInterface( imageCapabilitiesZ );
        imageCapabilitiesZ = NULL;
    }

    delete genericDevice; genericDevice = NULL;
    delete deviceManager; deviceManager = NULL;

    return( 0 );
}
