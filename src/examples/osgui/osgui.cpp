// Copyright 2011 Skew Matrix Software and AlphaPixel

#include <liblivescene/Version.h>
#include <liblivescene/DeviceManager.h>
#include <liblivescene/DeviceCapabilities.h>
#include <liblivescene/UserInteraction.h>
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
#include <osgGA/GUIEventHandler>
#include <osg/Texture2D>


class TestEventHandler : public osgGA::GUIEventHandler
{
public:
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        bool handled( false );
        switch( ea.getEventType() )
        {
        case osgGA::GUIEventAdapter::PUSH:
            std::cout << "Got PUSH  ";
            std::cout << ea.getX() << ", " << ea.getY() << std::endl;
            break;
        case osgGA::GUIEventAdapter::DRAG:
            //std::cout << "Got DRAG  ";
            //std::cout << ea.getX() << ", " << ea.getY() << std::endl;
            break;
        case osgGA::GUIEventAdapter::RELEASE:
            std::cout << "Got RELEASE  ";
            std::cout << ea.getX() << ", " << ea.getY() << std::endl;
            break;
        case osgGA::GUIEventAdapter::KEYDOWN:
            std::cout << "Got KEYDOWN  ";
            std::cout << ea.getKey() << std::endl;
            break;
        case osgGA::GUIEventAdapter::KEYUP:
            std::cout << "Got KEYUP  ";
            std::cout << ea.getKey() << std::endl;
            break;
        }
        return( handled );
    }
};


static const int nominalFrameW = 640, nominalFrameH = 480;


osg::Node* createScene( osg::Texture2D* tex )
{
    osg::ref_ptr< osg::Group > root = new osg::Group;

    root->addChild( osgDB::readNodeFile( "cow.osg" ) );

    // HUD Camera
    osg::ref_ptr< osg::Camera > cam = new osg::Camera;
    root->addChild( cam.get() );

    cam->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
#if( OSGWORKS_OSG_VERSION >= 20910 )
    // Not sure when this change took place, so the check against 2.9.10
    // might need to be refined.
    // Parent camera clear mask inherits down to child cameras. But we want
    // child camera (pre_render, so renders first) to do the clearing, and
    // parent camera to not clear. So we must explicitly set the clear mask.
    cam->setClearMask( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
    // In 2.8.3, camera clear mask did not inherit, and the child clear mask
    // has a default value of depth | clear, so this code isn't necessary in 2.8.3.
#endif
    cam->setProjectionMatrix( osg::Matrix::identity() );
    cam->setViewMatrix( osg::Matrix::identity() );
    cam->setRenderOrder( osg::Camera::PRE_RENDER );
    cam->setAllowEventFocus( false );

    osg::ref_ptr< osg::Geode > geode = new osg::Geode;
    cam->addChild( geode.get() );

#ifdef OSGWORKS_FOUND
    geode->addDrawable( osgwTools::makePlane( osg::Vec3( -1., 0., 0. ),
        osg::Vec3( 1., 0., 0. ), osg::Vec3( 0., -1., 0. ) ) );
#else
    geode->addDrawable( osg::createTexturedQuadGeometry( osg::Vec3( -1., 0., 0. ),
        osg::Vec3( 1., 0., 0. ), osg::Vec3( 0., -1., 0. ) ) );
#endif

    osg::StateSet* stateSet = geode->getOrCreateStateSet();
    stateSet->setTextureAttributeAndModes( 0, tex );
    stateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    stateSet->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );

    return( root.release() );
}


int main()
{

    std::cout << livescene::getVersionString() << std::endl;

#ifdef OSGWORKS_FOUND
    std::cout << osgwTools::getVersionString() << std::endl;
#endif


    livescene::DeviceManager *deviceManager = new livescene::DeviceManager();
    livescene::FactoryCollection collection;
    livescene::DeviceBase *genericDevice(NULL);
    livescene::DeviceCapabilitiesImage *imageCapabilitiesRGB(NULL),  *imageCapabilitiesZ(NULL);

	livescene::StringContainer capabilityCriteria;
	capabilityCriteria.push_back("IMAGE_RGB_RESOLUTION_640x480"); // Kinect basic RGB res
	capabilityCriteria.push_back("IMAGE_Z_RESOLUTION_640x480"); // Kinect basic Z res
    if( !( deviceManager->enumDevicesByCapability( collection, capabilityCriteria ) ) )
    {
        std::cerr << "Can't find DeviceFreenect." << std::endl;
        return( 1 );
    }

    if( !collection.empty() )
    {
        genericDevice = collection[0]->createDevice( 0, capabilityCriteria );
        if( genericDevice == NULL )
        {
            std::cerr << "Can't create generic device." << std::endl;
            return( 1 );
        }
        imageCapabilitiesRGB = static_cast<livescene::DeviceCapabilitiesImage *> (genericDevice->requestCapabilityInterface("IMAGE_RGB"));
        imageCapabilitiesZ = static_cast<livescene::DeviceCapabilitiesImage *> (genericDevice->requestCapabilityInterface("IMAGE_Z"));
        if( !imageCapabilitiesRGB || !imageCapabilitiesZ )
        {
            std::cerr << "No image capabilities." << std::endl;
            return( 1 );
        }
    }



    osg::ref_ptr< osg::Image > image = new osg::Image;
    image->setDataVariance( osg::Object::DYNAMIC );

    osg::ref_ptr< osg::Texture2D > tex = new osg::Texture2D;
    tex->setDataVariance( osg::Object::DYNAMIC );
    tex->setResizeNonPowerOfTwoHint( false );
    tex->setTextureSize( nominalFrameW, nominalFrameH );
    tex->setImage( image.get() );


    osgViewer::Viewer viewer;
    viewer.getCamera()->setClearMask( 0 ); // The HUD pre_render cam clears the framebuffer.
    viewer.addEventHandler( new osgViewer::StatsHandler() );
    viewer.setThreadingModel( osgViewer::ViewerBase::SingleThreaded );
    viewer.setUpViewInWindow( 30, 30, 800, 600 );
    viewer.setSceneData( createScene( tex.get() ) );

    osgGA::TrackballManipulator* tbm = new osgGA::TrackballManipulator();
    viewer.setCameraManipulator( tbm );
    viewer.addEventHandler( new TestEventHandler() );

    // Create an instance of the UserInteraction object.
    viewer.realize();
    osgViewer::ViewerBase::Windows windows;
    viewer.getWindows( windows );
    livescene::UserInteraction ui( *( windows[ 0 ] ) );

    while( !viewer.done() )
    {
        livescene::Image imageRGB( nominalFrameW, nominalFrameH, 3, livescene::VIDEO_RGB );
        livescene::Image imageZ( nominalFrameW, nominalFrameH, 2, livescene::DEPTH_10BIT );
        imageCapabilitiesRGB->getImageSync(imageRGB);
        imageCapabilitiesZ->getImageSync(imageZ);

        // Detect user interaction.
        ui.detectAndSendEvents( imageRGB, imageZ );

        livescene::Image persistentImageRGB(imageRGB, true); // try making a persistent copy

        image->setImage( nominalFrameW, nominalFrameH, 0, GL_RGB,
            GL_RGB, GL_UNSIGNED_BYTE, static_cast<unsigned char *>(persistentImageRGB.getData()), osg::Image::NO_DELETE );

        viewer.frame();
    }

    // Release the interfaces
    if( imageCapabilitiesRGB )
    {
        genericDevice->releaseCapabilityInterface( imageCapabilitiesRGB );
        imageCapabilitiesRGB = NULL;
    } // if
    if( imageCapabilitiesZ )
    {
        genericDevice->releaseCapabilityInterface( imageCapabilitiesZ );
        imageCapabilitiesZ = NULL;
    } // if

    // Need to destroy the device and manager
    // delete genericDevice doesn't work 100% right yet, as the unitsAvailable
    // in the factory doesn't decrease and libfreenect can't track that itself
    delete genericDevice; genericDevice = NULL;
    delete deviceManager; deviceManager = NULL;

    return( 0 );
}
