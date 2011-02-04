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
#include <osg/Point>
#include <osg/PositionAttitudeTransform>
#include <osgDB/WriteFile>


static const int NominalFrameW = 640, NominalFrameH = 480;


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
	//viewer.setUpViewOnSingleScreen();
	viewer.getCamera()->setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR);
    viewer.getCamera()->setProjectionMatrix( osg::Matrix::perspective( 35., 4./3., .001, 100. ) );

    osgGA::TrackballManipulator* tbm = new osgGA::TrackballManipulator();
    viewer.setCameraManipulator( tbm );

    bool backgroundEstablished(false);
	livescene::Background background;

	livescene::Geometry geometryBuilder;

	osg::ref_ptr<osg::PositionAttitudeTransform> centeringTransform = new osg::PositionAttitudeTransform();
	centeringTransform->setPosition(osg::Vec3( 0.109038, -0.0146183, 1.74733));
	osg::ref_ptr<osg::MatrixTransform> kinectTransform;
	kinectTransform = livescene::buildOSGGeometryMatrixTransform();
    kinectTransform->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
	kinectTransform->getOrCreateStateSet()->setAttribute( new osg::Point( 3.0f ), osg::StateAttribute::ON );

	centeringTransform->addChild(kinectTransform);
	viewer.setSceneData(centeringTransform);

	bool oneShot(false);
	
	for(bool keepGoing(true); keepGoing && !viewer.done(); )
    {
		bool goodRGB(false), goodZ(false);
		livescene::Image imageRGB(NominalFrameW, NominalFrameH, 3, livescene::VIDEO_RGB);
		livescene::Image imageZ(NominalFrameW, NominalFrameH, 2, livescene::DEPTH_10BIT);
		livescene::Image foreZ(NominalFrameW, NominalFrameH, 2, livescene::DEPTH_10BIT); // only the foreground
		if(ImageCapabilitiesRGB)
		{
			goodRGB = ImageCapabilitiesRGB->getImageSync(imageRGB);
		} // if
		if(ImageCapabilitiesZ)
		{
			goodZ = ImageCapabilitiesZ->getImageSync(imageZ);
		} // if

		// check to see if we got both types of data ok
		if(goodRGB && goodZ)
		{

			if(!backgroundEstablished)
			{ // store a background clean plate
				background.loadBackgroundFromCleanPlate(imageRGB, imageZ);
				backgroundEstablished = true;
			} // if

			livescene::Image persistentImageRGB(imageRGB, true); // test making a persistent copy

			//foreZ.preAllocate(); // need room to write processed data to
			//background.extractZBackground(imageZ, foreZ); // wipe out everything that is in the background plate
			//geometryBuilder.buildPointCloud(foreZ, &imageRGB); // using isolated background
			geometryBuilder.buildPointCloud(imageZ, &imageRGB); // do it without background isolation

			//geometryBuilder.buildFaces(foreZ, &imageRGB); // using isolated background
			//geometryBuilder.buildFaces(imageZ, &imageRGB); // do it without background isolation

			// build a new scene object on every frame
			osg::ref_ptr<osg::Geode> liveScene;
			liveScene = livescene::buildOSGPointCloudCopy(geometryBuilder);
			//liveScene = livescene::buildOSGPolyMeshCopy(geometryBuilder);

			// setup texturing
			osg::StateSet* stateSet = liveScene->getOrCreateStateSet();
			stateSet->setTextureAttributeAndModes( 0, tex );

			kinectTransform->removeChildren(0, 1); // this should throw away the removed child
			kinectTransform->addChild(liveScene);

			image->setImage( NominalFrameW, NominalFrameH, 0, GL_RGB,
				GL_RGB, GL_UNSIGNED_BYTE, static_cast<unsigned char *>(persistentImageRGB.getData()), osg::Image::NO_DELETE );
			viewer.frame();
		} // if
		else
		{
			keepGoing = false; // error exit
		} // else

    } // for keepGoing / !done

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
