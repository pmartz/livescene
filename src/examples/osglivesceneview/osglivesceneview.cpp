// Copyright 2011 Skew Matrix Software and AlphaPixel

#include <liblivescene/Version.h>
#include <liblivescene/DeviceManager.h>
#include <liblivescene/DeviceCapabilities.h>
#include <liblivescene/GeometryBuilder.h>
#include <liblivescene/osgGeometry.h>
#include <liblivescene/Background.h>
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
#include <osgDB/WriteFile>


static const int NominalFrameW = 640, NominalFrameH = 480;


/*
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
*/

int main()
{

	std::cout << livescene::getVersionString() << std::endl;

#ifdef OSGWORKS_FOUND
    std::cout << osgwTools::getVersionString() << std::endl;
#endif


	livescene::DeviceManager *deviceManager = new livescene::DeviceManager();
	livescene::FactoryCollection collectionA, collectionB, collectionC;
	livescene::StringContainer capabilityCriteria;
	livescene::DeviceBase *genericDevice(NULL);
	livescene::DeviceCapabilitiesImage *ImageCapabilitiesRGB(NULL),  *ImageCapabilitiesZ(NULL);

	// see what devices are available to meet various criteria
	deviceManager->enumDevicesByName(collectionA, "");
	deviceManager->enumDevicesByName(collectionA, "FooBar"); // should fail
	deviceManager->enumDevicesByName(collectionA, "DeviceFreenect"); // should succeed

	capabilityCriteria.clear();
	capabilityCriteria.push_back("WORLD_PEACE"); // setup some unreasonable criteria that can't be met
	capabilityCriteria.push_back("COOKS_REALLY_GOOD_STEAK");
	deviceManager->enumDevicesByCapability(collectionB, capabilityCriteria); // should fail

	capabilityCriteria.clear();
	capabilityCriteria.push_back("IMAGE_RGB_RESOLUTION_640x480");
	capabilityCriteria.push_back("IMAGE_Z_RESOLUTION_640x480");
	deviceManager->enumDevicesByCapability(collectionB, capabilityCriteria); // should succeed

	if(!collectionA.empty())
	{
		genericDevice = collectionA[0]->createDevice(-1, capabilityCriteria);
		delete genericDevice;
		genericDevice = NULL;
	} // if

	if(!collectionB.empty())
	{
		if(genericDevice = deviceManager->acquireDeviceByCapabilities(capabilityCriteria))
		{
			// On Kinect, same interface will provide RGB and Z, but we should be careful to obtain two
			// distinct interfaces just in case. Make sure to cast to the interface object type you need
			ImageCapabilitiesRGB = static_cast<livescene::DeviceCapabilitiesImage *> (genericDevice->requestCapabilityInterface("IMAGE_RGB"));
			ImageCapabilitiesZ = static_cast<livescene::DeviceCapabilitiesImage *> (genericDevice->requestCapabilityInterface("IMAGE_Z"));
		} // if
	} // if


	/*
	osg::ref_ptr< osg::Image > image = new osg::Image;
    image->setDataVariance( osg::Object::DYNAMIC );

    osg::ref_ptr< osg::Texture2D > tex = new osg::Texture2D;
    tex->setDataVariance( osg::Object::DYNAMIC );
    tex->setResizeNonPowerOfTwoHint( false );
    tex->setTextureSize( NominalFrameW, NominalFrameH );
    tex->setImage( image.get() );
	*/

    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgViewer::StatsHandler() );
    viewer.setThreadingModel( osgViewer::ViewerBase::SingleThreaded );
    viewer.setUpViewInWindow( 30, 30, 800, 600 );
    //viewer.setSceneData( createScene( tex.get() ) );

    osgGA::TrackballManipulator* tbm = new osgGA::TrackballManipulator();
    viewer.setCameraManipulator( tbm );

    bool backgroundEstablished(false);
	livescene::Background background;

	livescene::Geometry geometryBuilder;

	osg::ref_ptr<osg::MatrixTransform> kinectTransform;
	kinectTransform = livescene::buildOSGGeometryMatrixTransform();
	viewer.setSceneData(kinectTransform);

	bool oneShot(false);
	
	while( !viewer.done() )
    {
		livescene::Image imageRGB(NominalFrameW, NominalFrameH, 3, livescene::VIDEO_RGB);
		livescene::Image imageZ(NominalFrameW, NominalFrameH, 2, livescene::DEPTH_10BIT);
		if(ImageCapabilitiesRGB)
		{
			//ImageCapabilitiesRGB->getImageSync(imageRGB);
		} // if
		if(ImageCapabilitiesZ)
		{
			ImageCapabilitiesZ->getImageSync(imageZ);
		} // if

		if(!backgroundEstablished)
		{ // store a background clean plate
			//background.loadBackgroundFromCleanPlate(imageRGB, imageZ);
		} // if

		//livescene::Image persistentImageRGB(imageRGB, true); // test making a persistent copy

		//background.extractZBackground(imageZ, foreZ); // <<<>>> can't do this yet, we'll need an output foreZ object

		geometryBuilder.buildPointCloud(imageZ, &imageRGB);

		// build a new cloud object on every frame
		osg::ref_ptr<osg::Geode> pointCloud;
		pointCloud = livescene::buildOSGPointCloudCopy(geometryBuilder);
		kinectTransform->removeChildren(0, 1); // this should throw away the removed child
		kinectTransform->addChild(pointCloud);

        /*
		image->setImage( NominalFrameW, NominalFrameH, 0, GL_RGB,
            GL_RGB, GL_UNSIGNED_BYTE, static_cast<unsigned char *>(persistentImageRGB.getData()), osg::Image::NO_DELETE );
		*/
		// <<<>>> do something with the Z value

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
