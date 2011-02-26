// Copyright 2011 Skew Matrix Software and AlphaPixel

#include <liblivescene/Version.h>
#include <liblivescene/DeviceManager.h>
#include <liblivescene/DeviceCapabilities.h>
#include <liblivescene/GeometryBuilder.h>
#include <liblivescene/osgGeometry.h>
#include <liblivescene/Background.h>
#include <iostream>
#include <iomanip>

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

const int OSG_LIVESCENEVIEW_BACKGROUND_NOISE_SAMPLES(1500); // fewer than this many foreground samples in a frame are presumed to be background noise
const int OSG_LIVESCENEVIEW_INITIAL_BACKGROUND_FRAMES(5); // Assume the first n frames are empty background, for calibration

static const int NominalFrameW = 640, NominalFrameH = 480; // <<<>>> these should be made dynamic

osg::ref_ptr<osg::Group> topGroup;
// for HUD and any other screen-space elements
osg::ref_ptr<osg::Group> screenElementsGroup;
osg::ref_ptr<osg::Geode> screenTextGeode;
osg::ref_ptr<osg::Projection> screenProjection;
osg::ref_ptr<osg::MatrixTransform> screenMatrixTransform;
osg::ref_ptr<osgText::Text> textEntity;

osg::ref_ptr<osg::Geode>foreGroundMarker;
osg::ref_ptr<osg::PositionAttitudeTransform>fgMarkerPAT;

// some strings that get formatted together to make the HUD metadata
std::string fgInfoStr, minDepth, maxDepth;

class BoxApproveCallback : public livescene::ApproveCallback
{
public:
	BoxApproveCallback(osg::BoundingBox bbox) : _bbox(bbox) {}
	bool operator ()(const unsigned int &xCoord, const unsigned int &yCoord, const unsigned short &zCoord)
	{
		return(_bbox.contains(osg::Vec3f(xCoord, yCoord, zCoord)));
	} // operator ()
private:
	osg::BoundingBox _bbox;
}; // BoxApproveCallback 

void buildMarker(void)
{
	foreGroundMarker = new osg::Geode;
	fgMarkerPAT = new osg::PositionAttitudeTransform;
	//foreGroundMarker->addDrawable(osgwTools::makeWireAltAzSphere( 4.0, 16, 32 ));
	foreGroundMarker->addDrawable(osgwTools::makeWireBox( osg::Vec3(1.0, 1.0, 1.0)));
	fgMarkerPAT->addChild(foreGroundMarker);
} // buildMarker

void buildHUD(void)
{
	screenElementsGroup = new osg::Group();

	// setup screen-space entities
	screenTextGeode = new osg::Geode();
	screenProjection = new osg::Projection();
	screenProjection->setMatrix(osg::Matrix::ortho2D(0.0,1024,0,768));
	screenMatrixTransform = new osg::MatrixTransform();
	screenMatrixTransform->setMatrix(osg::Matrix::identity());
	screenMatrixTransform->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	screenElementsGroup->addChild(screenProjection);
	screenProjection->addChild(screenMatrixTransform);
	screenMatrixTransform->addChild(screenTextGeode);

	// For this state set, turn blending on (so alpha texture looks right)
	screenTextGeode->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
	// Disable depth testing so geometry is drawn regardless of depth values
	// of geometry already draw.
	screenTextGeode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
	screenTextGeode->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

	// Need to make sure this geometry is drawn last. RenderBins are handled
	// in numerical order so set bin number to 11
	screenTextGeode->getOrCreateStateSet()->setRenderBinDetails( 11, "RenderBin");

	textEntity = new osgText::Text();

	// Set up the parameters for the text we'll add to the HUD:
	textEntity->setCharacterSize(25.0);
	textEntity->setFont("arial.ttf");
	textEntity->setAlignment(osgText::TextBase::LEFT_TOP);
	textEntity->setAxisAlignment(osgText::Text::SCREEN);
	textEntity->setPosition( osg::Vec3(5.0, 762, 0.0)); // 5,5 from UL
	textEntity->setColor( osg::Vec4(1.0, 1.0, 1.0, 1.0) );

	screenTextGeode->addDrawable( textEntity );

} // buildHUD

int main()
{
	bool PolygonsMode(true), IsolateBackground(true), ShowBackground(true), textureForeground(false), textureBackground(true);
	osg::Vec4 foreColor(0.0, 0.7, 1.0, 1.0), backColor(1.0, 1.0, 1.0, 1.0);
	livescene::ImageStatistics statsX, statsY, statsZ;
	livescene::ImageStatistics statsBodyX, statsBodyY, statsBodyZ;
    const int nominalFrameD( 1024 ); // <<<>>> these should be made dynamic
    osg::Matrix d2w = livescene::makeDeviceToWorldMatrix( NominalFrameW, NominalFrameH, nominalFrameD /*, TBD Device device */ );

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

	topGroup = new osg::Group;

	osg::ref_ptr< osg::Image > imageFore = new osg::Image;
    imageFore->setDataVariance( osg::Object::DYNAMIC );

    osg::ref_ptr< osg::Texture2D > texFore = new osg::Texture2D;
    texFore->setDataVariance( osg::Object::DYNAMIC );
    texFore->setResizeNonPowerOfTwoHint( false );
    texFore->setTextureSize( NominalFrameW, NominalFrameH );
    texFore->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    texFore->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    texFore->setImage( imageFore.get() );

	osg::ref_ptr< osg::Image > imageBack = new osg::Image;
    imageBack->setDataVariance( osg::Object::DYNAMIC );

    osg::ref_ptr< osg::Texture2D > texBack = new osg::Texture2D;
    texBack->setDataVariance( osg::Object::DYNAMIC );
    texBack->setResizeNonPowerOfTwoHint( false );
    texBack->setTextureSize( NominalFrameW, NominalFrameH );
    texBack->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    texBack->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    texBack->setImage( imageBack.get() );

    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgViewer::StatsHandler() );
    viewer.setThreadingModel( osgViewer::ViewerBase::SingleThreaded );
    viewer.setUpViewInWindow( 30, 30, 800, 600 );
	//viewer.setUpViewOnSingleScreen();
	viewer.getCamera()->setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR);
    viewer.getCamera()->setProjectionMatrix( osg::Matrix::perspective( 35., 4./3., .001, 100. ) );

    int backgroundEstablished(0);
	livescene::Background background;

	livescene::Geometry geometryBuilderFore;
	livescene::Geometry geometryBuilderBack;

    osg::ref_ptr<osg::Group> kinectGroup = new osg::Group;
    kinectGroup->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
	kinectGroup->getOrCreateStateSet()->setAttribute( new osg::Point( 3.0f ), osg::StateAttribute::ON );
	topGroup->addChild(kinectGroup.get());

	// add HUD
	buildHUD();
	topGroup->addChild(screenElementsGroup.get());

	// add marker
	buildMarker();
	topGroup->addChild(fgMarkerPAT.get());

	// set scene data
	viewer.setSceneData( topGroup.get() );

	bool debugOneShot(false), firstFrame(true);
	
	// retain the scene data object from frame to frame to improve reuse
	osg::ref_ptr<osg::Geode> foreScene;
	osg::ref_ptr<osg::Geode> backScene;

	for(bool keepGoing(true); keepGoing && !viewer.done(); )
    {
		bool goodRGB(false), goodZ(false), noForeground(false);
		livescene::Image imageRGB(NominalFrameW, NominalFrameH, 3, livescene::VIDEO_RGB);
		livescene::Image imageZ(NominalFrameW, NominalFrameH, 2, livescene::DEPTH_10BIT);
		livescene::Image foreZ(NominalFrameW, NominalFrameH, 2, livescene::DEPTH_10BIT); // only the foreground
		unsigned int numFiltered(0);

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
				++backgroundEstablished;
			} // if
			else if(backgroundEstablished < OSG_LIVESCENEVIEW_INITIAL_BACKGROUND_FRAMES) // accumulate some frames, averaged, for background calibration
			{ // store a background clean plate
				background.accumulateBackgroundFromCleanPlate(imageRGB, imageZ, livescene::Background::AVERAGE_Z);
				++backgroundEstablished;
			} // if

			if(IsolateBackground)
			{
				foreZ.preAllocate(); // need room to write processed data to (only done at start if needed, not on every frame)
				background.extractZBackground(imageZ, foreZ); // wipe out everything that is already in the background plate

				// calculate some stats
				foreZ.calcStatsXYZ(&statsX, &statsY, &statsZ);
				
				// we can only dynamically accumulate background when we don't think there's a foreground object in frame,
				// because foreground objects contacting background objects may get 'sucked into' the background
				if(backgroundEstablished >= 5 && statsZ.getNumSamples() < OSG_LIVESCENEVIEW_BACKGROUND_NOISE_SAMPLES) // very small amount of foreground
				{
					// add it to the background
					// make sure we're only adding it where it's Z-threshold adjacent to existing background (MIN_Z_ADJACENT)
					background.accumulateBackgroundFromCleanPlate(imageRGB, imageZ, livescene::Background::MIN_Z_ADJACENT, &foreZ);
					noForeground = true; // <<<>>> somehow we could completely avoid drawing the foreground in this case, but we don't yet
				} // if
				if(!noForeground)
				{ // these operations only make sense if we have a forefround
					// filter noise
					numFiltered = foreZ.filterNoise(2);
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
					kinectGroup->addChild(foreScene);
					if(textureForeground)
					{
						// setup texturing
						osg::StateSet* stateSetFore = foreScene->getOrCreateStateSet();
						stateSetFore->setTextureAttributeAndModes( 0, texFore );
					} // if

					if(ShowBackground)
					{
						kinectGroup->addChild(backScene);
						if(textureBackground)
						{
							// setup texturing
							osg::StateSet* stateSetBack = backScene->getOrCreateStateSet();
							stateSetBack->setTextureAttributeAndModes( 0, texBack );
							imageBack->setImage( NominalFrameW, NominalFrameH, 0, GL_RGB,
								GL_RGB, GL_UNSIGNED_BYTE, static_cast<unsigned char *>(background.getBackgroundRGB().getData()), osg::Image::NO_DELETE );
						} // if
					} // if

                    // Go to the initial position.
                    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );

					firstFrame = false;
				} // if


				if(textureForeground)
				{ // update live foreground texture
					imageFore->setImage( NominalFrameW, NominalFrameH, 0, GL_RGB,
						GL_RGB, GL_UNSIGNED_BYTE, static_cast<unsigned char *>(imageRGB.getData()), osg::Image::NO_DELETE );
				} // if

			} // oneshot

			float worldBoxXScale(0.0f), worldBoxYScale(0.0f), worldBoxZScale(0.0f);
			if(noForeground)
			{
				statsX.clear(); statsY.clear(); statsZ.clear();
				statsBodyX.clear(); statsBodyY.clear(); statsBodyZ.clear();
				fgMarkerPAT->setNodeMask(0);
			} // if
			else
			{
				fgMarkerPAT->setNodeMask(~0);
				// update the marker
				osg::Vec3 worldForegroundMean(livescene::transformPoint(d2w, statsX.getMean(), statsY.getMean(), statsZ.getMean()));
				osg::Vec3 worldForegroundXStdDev, worldForegroundYStdDev, worldForegroundZStdDev;
				worldForegroundXStdDev = livescene::transformPoint(d2w, statsX.getMean() + statsX.getStdDev(), statsY.getMean(), statsZ.getMean());
				worldForegroundYStdDev = livescene::transformPoint(d2w, statsX.getMean(), statsY.getMean() + statsY.getStdDev(), statsZ.getMean());
				worldForegroundZStdDev = livescene::transformPoint(d2w, statsX.getMean(), statsY.getMean(), statsZ.getMean() + statsZ.getStdDev());

				// recalculate stats of body, using bounds of nth standard deviation to exclude extraneous noise
				const float nthStdDev(2.8);
				BoxApproveCallback stdDevBoxApprove(osg::BoundingBox(
					statsX.getMean() - statsX.getStdDev() * nthStdDev, // xmin
					statsY.getMean() - statsY.getStdDev() * nthStdDev, // ymin
					statsZ.getMean() - statsZ.getStdDev() * nthStdDev, // zmin
					statsX.getMean() + statsX.getStdDev() * nthStdDev, // xmax
					statsY.getMean() + statsY.getStdDev() * nthStdDev, // ymax
					statsZ.getMean() + statsZ.getStdDev() * nthStdDev // zmax
					));
				// <<<>>> this could be made faster by passing the XY bounding box as a limiter
				foreZ.calcStatsXYZ(&statsBodyX, &statsBodyY, &statsBodyZ, &stdDevBoxApprove);

				osg::Vec3 worldBodyMean(livescene::transformPoint(d2w, statsBodyX.getMean(), statsBodyY.getMean(), statsBodyZ.getMean()));
				osg::Vec3 worldBodyXStdDev, worldBodyYStdDev, worldBodyZStdDev;
				worldBodyXStdDev = livescene::transformPoint(d2w, statsBodyX.getMean() + statsBodyX.getStdDev(), statsBodyY.getMean(), statsBodyZ.getMean());
				worldBodyYStdDev = livescene::transformPoint(d2w, statsBodyX.getMean(), statsBodyY.getMean() + statsBodyY.getStdDev(), statsBodyZ.getMean());
				worldBodyZStdDev = livescene::transformPoint(d2w, statsBodyX.getMean(), statsBodyY.getMean(), statsBodyZ.getMean() + statsBodyZ.getStdDev());

				// because we only see the front half of objects, their mass is baised forward 1/2 in Z
				// To compensate, we can average their position half/half with the worldBodyZStdDev, which
				// re-biases it backwards to where it ought to be
				//fgMarkerPAT->setPosition((worldBodyMean + worldBodyZStdDev) * 0.5);
				fgMarkerPAT->setPosition(worldBodyMean);

				worldBoxXScale = 2 * (worldBodyXStdDev - worldBodyMean).x(); // multiply by two, since stddev is sort of a radius-like, on one side of mean
				worldBoxYScale = 2 * (worldBodyYStdDev - worldBodyMean).y(); // multiply by two, since stddev is sort of a radius-like, on one side of mean
				worldBoxZScale = 2 * (worldBodyZStdDev - worldBodyMean).z(); // multiply by two, since stddev is sort of a radius-like, on one side of mean
				fgMarkerPAT->setScale(osg::Vec3(worldBoxXScale, worldBoxYScale, worldBoxZScale));
			} // else
			
			// update the HUD
			std::ostringstream textHUD;

			if(noForeground) fgInfoStr = "foreground not detected"; else fgInfoStr = "* FOREGROUND DETECTED *";
			textHUD.setf(std::ios::fixed,std::ios::floatfield);
			textHUD << std::setprecision(0) <<
				"Foreground Samples: " << statsZ.getNumSamples() << std::endl <<
				fgInfoStr << std::endl <<
				"Noise Filtered: " << numFiltered << std::endl <<
				"Body Samples: " << statsBodyZ.getNumSamples() << std::endl <<
				"zMin: " << statsZ.getMin() << std::endl <<
				"zMed: " << statsZ.getMedian() << std::endl <<
				"zMax: " << statsZ.getMax() << std::endl <<
				"zMN: " << statsZ.getMean() << std::endl <<
				"zSD: " << statsZ.getStdDev() << std::endl <<
				"xMN: " << statsX.getMean() << std::endl <<
				"xSD: " << statsX.getStdDev() << std::endl <<
				"yMN: " << statsY.getMean() << std::endl <<
				"ySD: " << statsY.getStdDev();
			textEntity->setText(textHUD.str());

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
