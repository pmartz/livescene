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
	bool PolygonsMode(false), IsolateBackground(true), ShowBackground(true), textureForeground(false), textureBackground(true);
	osg::Vec4 foreColor(1.0, 0.0, 0.0, 1.0), backColor(1.0, 1.0, 1.0, 1.0);

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


	osg::ref_ptr< osg::Image > imageFore = new osg::Image;
    imageFore->setDataVariance( osg::Object::DYNAMIC );

    osg::ref_ptr< osg::Texture2D > texFore = new osg::Texture2D;
    texFore->setDataVariance( osg::Object::DYNAMIC );
    texFore->setResizeNonPowerOfTwoHint( false );
    texFore->setTextureSize( NominalFrameW, NominalFrameH );
    texFore->setImage( imageFore.get() );

	osg::ref_ptr< osg::Image > imageBack = new osg::Image;
    imageBack->setDataVariance( osg::Object::DYNAMIC );

    osg::ref_ptr< osg::Texture2D > texBack = new osg::Texture2D;
    texBack->setDataVariance( osg::Object::DYNAMIC );
    texBack->setResizeNonPowerOfTwoHint( false );
    texBack->setTextureSize( NominalFrameW, NominalFrameH );
    texBack->setImage( imageBack.get() );

    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgViewer::StatsHandler() );
    viewer.setThreadingModel( osgViewer::ViewerBase::SingleThreaded );
    viewer.setUpViewInWindow( 30, 30, 800, 600 );
	//viewer.setUpViewOnSingleScreen();
	viewer.getCamera()->setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR);
    viewer.getCamera()->setProjectionMatrix( osg::Matrix::perspective( 35., 4./3., .001, 100. ) );

    osgGA::TrackballManipulator* tbm = new osgGA::TrackballManipulator();
    viewer.setCameraManipulator( tbm );

    int backgroundEstablished(0);
	livescene::Background background;

	livescene::Geometry geometryBuilderFore;
	livescene::Geometry geometryBuilderBack;

	osg::ref_ptr<osg::PositionAttitudeTransform> centeringTransform = new osg::PositionAttitudeTransform();
	centeringTransform->setPosition(osg::Vec3( 0.109038, -0.0146183, 1.74733));
	// trying to get it onscreen where we want it, unsuccessful
	//osg::Quat attitude;
	//centeringTransform->setPivotPoint(osg::Vec3( 0.109038, -0.0146183, 1.74733));
	//attitude.makeRotate(1.5707, 1.0, 0.0, 0.0);
	//centeringTransform->setAttitude(attitude);
	osg::ref_ptr<osg::MatrixTransform> kinectTransform;
	kinectTransform = livescene::buildOSGGeometryMatrixTransform();
    kinectTransform->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
	kinectTransform->getOrCreateStateSet()->setAttribute( new osg::Point( 3.0f ), osg::StateAttribute::ON );

	centeringTransform->addChild(kinectTransform);
	viewer.setSceneData(centeringTransform);

	bool debugOneShot(false), firstFrame(true);
	
	// retain the scene data object from frame to frame to improve reuse
	osg::ref_ptr<osg::Geode> foreScene;
	osg::ref_ptr<osg::Geode> backScene;

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

			if(backgroundEstablished == 0) // load initial frame
			{ // store a background clean plate
				background.loadBackgroundFromCleanPlate(imageRGB, imageZ);
				backgroundEstablished++;
			} // if
			else if(backgroundEstablished < 5) // accumulate 5 frames averaged
			{ // store a background clean plate
				background.accumulateBackgroundFromCleanPlate(imageRGB, imageZ, livescene::Background::AVERAGE_Z);
				backgroundEstablished++;
			} // if

			//livescene::Image persistentImageRGB(imageRGB, true); // test making a persistent copy

			if(IsolateBackground)
			{
				foreZ.preAllocate(); // need room to write processed data to
				background.extractZBackground(imageZ, foreZ); // wipe out everything that is in the background plate

				// test calculating some stats
				livescene::ImageStatistics statsX, statsY, statsZ;
				foreZ.calcStatsXYZ(&statsX, &statsY, &statsZ);
				double stddev = statsX.getStdDev();
				//std::cout << "Samples=" << statsZ.getNumSamples() << " ZMax=" << statsZ.getMax() << " ZMin=" << statsZ.getMin() << " ZMean=" << statsZ.getMean() << " ZSD=" << statsZ.getStdDev() << std::endl;
				//std::cout << " XMax=" << statsX.getMax() << " XMin=" << statsX.getMin() << " XMean=" << statsX.getMean() << " XSD=" << statsX.getStdDev() << std::endl;
				//std::cout << " YMax=" << statsY.getMax() << " YMin=" << statsY.getMin() << " YMean=" << statsY.getMean() << " YSD=" << statsY.getStdDev() << std::endl << std::endl;
				if(backgroundEstablished >= 5 && statsZ.getNumSamples() < 500) // very small amount of foreground
				{
					// add it to the background
					background.accumulateBackgroundFromCleanPlate(imageRGB, imageZ, livescene::Background::MIN_Z);
				} // if
			} // if






			if(!debugOneShot)
			{
				//debugOneShot = true;
				if(!PolygonsMode)
				{ // point cloud mode
					if(IsolateBackground)
					{
						geometryBuilderFore.buildPointCloud(foreZ, &imageRGB); // using isolated background
						if(ShowBackground)
						{
							geometryBuilderBack.buildPointCloud(background.getBackgroundZ(), &background.getBackgroundRGB()); // build only the BG data
							backScene = livescene::buildOSGPointCloudCopy(geometryBuilderBack, backScene, backColor);
						} // if
					} // if
					else
					{
						geometryBuilderFore.buildPointCloud(imageZ, &imageRGB); // do it without background isolation
					} // else
					foreScene = livescene::buildOSGPointCloudCopy(geometryBuilderFore, foreScene, foreColor);
				} // if
				else
				{ // polygon mesh mode
					if(IsolateBackground)
					{
						geometryBuilderFore.buildFaces(foreZ, &imageRGB); // using isolated background
						if(ShowBackground)
						{
							geometryBuilderBack.buildFaces(background.getBackgroundZ(), &background.getBackgroundRGB()); // build only the BG data
							backScene = livescene::buildOSGPolyMeshCopy(geometryBuilderBack, backScene, backColor);
						} // if
					} // 
					else
					{
						geometryBuilderFore.buildFaces(imageZ, &imageRGB); // do it without background isolation
					} // else
					foreScene = livescene::buildOSGPolyMeshCopy(geometryBuilderFore, foreScene, foreColor);
				} // else


				// link the new data into the scene graph and setup texturing if this is the first frame
				if(firstFrame)
				{
					kinectTransform->addChild(foreScene);
					if(textureForeground)
					{
						// setup texturing
						osg::StateSet* stateSetFore = foreScene->getOrCreateStateSet();
						stateSetFore->setTextureAttributeAndModes( 0, texFore );
					} // if

					if(ShowBackground)
					{
						kinectTransform->addChild(backScene);
						if(textureBackground)
						{
							// setup texturing
							osg::StateSet* stateSetBack = backScene->getOrCreateStateSet();
							stateSetBack->setTextureAttributeAndModes( 0, texBack );
							imageBack->setImage( NominalFrameW, NominalFrameH, 0, GL_RGB,
								GL_RGB, GL_UNSIGNED_BYTE, static_cast<unsigned char *>(background.getBackgroundRGB().getData()), osg::Image::NO_DELETE );
						} // if
					} // if
					firstFrame = false;
				} // if


				if(textureForeground)
				{ // update live foreground texture
					imageFore->setImage( NominalFrameW, NominalFrameH, 0, GL_RGB,
						GL_RGB, GL_UNSIGNED_BYTE, static_cast<unsigned char *>(imageRGB.getData()), osg::Image::NO_DELETE );
				} // if

				//osgDB::writeNodeFile(*viewer.getSceneData(), "largescene.osg");
			} // oneshot

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
