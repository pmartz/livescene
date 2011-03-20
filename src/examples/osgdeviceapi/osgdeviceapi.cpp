// Copyright 2011 Skew Matrix Software and AlphaPixel

#include <liblivescene/Version.h>
#include <liblivescene/DeviceManager.h>
#include <liblivescene/DeviceCapabilities.h>
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


	// first, you'll need a DeviceManager
	livescene::DeviceManager *deviceManager = new livescene::DeviceManager();

	// these are DeviceFactory collections, the results of enumerations by the DeviceManager
	// only one is needed, we have an extra for demonstration purposes
	livescene::FactoryCollection collectionA, collectionB;
	// StringContainer is just a container (vector) of strings, used for storing a list
	// of string properties -- names or capabilities -- that you might want to use to help
	// choose a device
	livescene::StringContainer capabilityCriteria;

	// The DeviceBase is the reprsentation of an actual LiveScene input device. From it, you can
	// request capabilities interfaces, like and image-input interface
	livescene::DeviceBase *genericDevice(NULL);

	// The most interesting interface is the image-grabbing interface. Technically we need one
	// for each, RGB and Z, though they are probably going to resolve to the same interface
	livescene::DeviceCapabilitiesImage *ImageCapabilitiesRGB(NULL),  *ImageCapabilitiesZ(NULL);

	// see what devices are available to meet various criteria
	// You only need one of these, probabvly the last, or even the first if you want to be really non-specific
	deviceManager->enumDevicesByName(collectionA, ""); // "" means any device, no specific name criteria, give us everything
	deviceManager->enumDevicesByName(collectionA, "FooBar"); // Look for a device named FooBar. This will fail.
	deviceManager->enumDevicesByName(collectionA, "DeviceFreenect"); // Look for the DeviceFreenect device type. This should succeed.

	// now let's try asking for a device not by its specific type, but by what it says it can do for us
	capabilityCriteria.clear(); // to be safe, clear the capabilityCriteria container
	capabilityCriteria.push_back("WORLD_PEACE"); // setup some unreasonable criteria that can't be met
	capabilityCriteria.push_back("COOKS_REALLY_GOOD_STEAK"); // these strings are matched against strings the device publishes
	deviceManager->enumDevicesByCapability(collectionB, capabilityCriteria); // We're asking for a device that can satisfy both of the above criteria. None will.

	// let's start over and ask fo realistic criteria, and we'll be rewarded with success
	capabilityCriteria.clear(); // to be safe, clear the capabilityCriteria container
	capabilityCriteria.push_back("IMAGE_RGB_RESOLUTION_640x480"); // Kinect basic RGB res
	capabilityCriteria.push_back("IMAGE_Z_RESOLUTION_640x480"); // Kinect basic Z res
	deviceManager->enumDevicesByCapability(collectionB, capabilityCriteria); // should succeed if you have a Kinect device implementation, as it provide both criteria

	// now, from a collection of DeviceFactory's, you can pick one and ask for a device (DeviceBase)
	// to be created from it. The DeviceBase will give you access to the Interfaces you need to do work.
	if(!collectionA.empty()) // are there some qualifying device types in collectionA?
	{
		// we'll ask for unit #-1, which means next-available unit.
		// if you care about unit numbers, you can specify them
		genericDevice = collectionA[0]->createDevice(-1, capabilityCriteria); // ask the first entry to instantiate a DeviceBase for us
		delete genericDevice; // that was just for testing, we'll delete it now
		genericDevice = NULL;
	} // if

	// another way to get a DeviceBase is to just ask to acquire one based on criteria
	// without getting a collection involved. This is sort of a shortcut for what happens
	// in the collectionB code above, and in fact we use the exact same criteria
	// when you do this, unit #-1, or next-available is implied.
	if(genericDevice = deviceManager->acquireDeviceByCapabilities(capabilityCriteria))
	{
		// Now that we have the device, we need to ask it for an interface that performs the
		// operations we want from it. Here all we ask for is the RGB and Z image grabbing.
		// Kinect supports many other interfaces, not all of which may be implemented by it, or other devices
		//
		// On Kinect, same interface will provide RGB and Z, but we should be careful to obtain two
		// distinct interfaces just in case. Make sure to cast to the interface object type you need
		// also, make sure to release the interface at the end when you're done with it
		ImageCapabilitiesRGB = static_cast<livescene::DeviceCapabilitiesImage *> (genericDevice->requestCapabilityInterface("IMAGE_RGB"));
		ImageCapabilitiesZ = static_cast<livescene::DeviceCapabilitiesImage *> (genericDevice->requestCapabilityInterface("IMAGE_Z"));
	} // if


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

    while( !viewer.done() )
    {
		// these are the objects that image data is stored into
		livescene::Image imageRGB(NominalFrameW, NominalFrameH, 3, livescene::VIDEO_RGB);
		livescene::Image imageZ(NominalFrameW, NominalFrameH, 2, livescene::DEPTH_10BIT);

		if(ImageCapabilitiesRGB) // make sure the interface is available
		{
			ImageCapabilitiesRGB->getImageSync(imageRGB);
		} // if
		if(ImageCapabilitiesZ) // make sure the interface is available
		{
			ImageCapabilitiesZ->getImageSync(imageZ);
			imageZ.rewriteZeroToNull(); // we do this explicitly to make null/valid handling quicker, later
		} // if

		// the images returned by getImage*() are non-persistent -- the data buffer in them
		// belongs to the Device, and will get overwritten during the next getImage*() call
		// You can use the copy constructor below to make a persistant copy of the Image with
		// its own allocated data block that nobody else will mess with
		livescene::Image persistentImageRGB(imageRGB, true); // try making a persistent copy

        // jam that persistent image data into a texture for display
		image->setImage( NominalFrameW, NominalFrameH, 0, GL_RGB,
            GL_RGB, GL_UNSIGNED_BYTE, static_cast<unsigned char *>(persistentImageRGB.getData()), osg::Image::NO_DELETE );
		// here you might do something with the Z value

        viewer.frame();
    }

	// Release the interfaces
	if(ImageCapabilitiesRGB)
	{
		genericDevice->releaseCapabilityInterface(ImageCapabilitiesRGB); ImageCapabilitiesRGB = NULL;
	} // if
	if(ImageCapabilitiesZ)
	{
		genericDevice->releaseCapabilityInterface(ImageCapabilitiesZ); ImageCapabilitiesZ = NULL;
	} // if

	// Need to destroy the device and manager
	// delete genericDevice doesn't work 100% right yet, as the unitsAvailable
	// in the factory doesn't decrease and libfreenect can't track that itself
	delete genericDevice; genericDevice = NULL;
	delete deviceManager; deviceManager = NULL;

    return( 0 );
}
