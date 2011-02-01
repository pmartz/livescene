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
            std::cout << "Got DRAG  ";
            std::cout << ea.getX() << ", " << ea.getY() << std::endl;
            break;
        case osgGA::GUIEventAdapter::RELEASE:
            std::cout << "Got RELEASE  ";
            std::cout << ea.getX() << ", " << ea.getY() << std::endl;
            break;
        }
        return( handled );
    }
};


static const int NominalFrameW = 640, NominalFrameH = 480;


osg::Node* createScene( osg::Texture2D* tex )
{
    osg::ref_ptr< osg::Geode > geode = new osg::Geode;
#ifdef OSGWORKS_FOUND
    geode->addDrawable( osgwTools::makePlane( osg::Vec3( -1., 0., -1. ),
        osg::Vec3( NominalFrameW, 0., 0. ), osg::Vec3( 0., 0., -NominalFrameH ) ) );
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


    livescene::DeviceManager *deviceManager = new livescene::DeviceManager();
    livescene::FactoryCollection collection;
    livescene::StringContainer capabilityCriteria;
    livescene::DeviceBase *genericDevice(NULL);
    livescene::DeviceCapabilitiesImage *imageCapabilitiesRGB(NULL),  *imageCapabilitiesZ(NULL);

    if( !( deviceManager->enumDevicesByName( collection, "DeviceFreenect" ) ) )
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
    tex->setTextureSize( NominalFrameW, NominalFrameH );
    tex->setImage( image.get() );


    osgViewer::Viewer viewer;
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
        livescene::Image imageRGB(NominalFrameW, NominalFrameH, 3, livescene::VIDEO_RGB);
        livescene::Image imageZ(NominalFrameW, NominalFrameH, 2, livescene::DEPTH_10BIT);
        imageCapabilitiesRGB->getImageSync(imageRGB);
        imageCapabilitiesZ->getImageSync(imageZ);

        // Detect user interaction.
        ui.detectAndSendEvents( imageRGB, imageZ );

        livescene::Image persistentImageRGB(imageRGB, true); // try making a persistent copy

        image->setImage( NominalFrameW, NominalFrameH, 0, GL_RGB,
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
