// Copyright 2011 Skew Matrix Software and AlphaPixel

#include "liblivescene/DeviceFreenect.h"
#include "liblivescene/DeviceCapabilities.h"
#include <string>
#include <algorithm>
#include <libfreenect.h>
#include <libfreenect_sync.h>


namespace livescene {

DeviceFreenectFactory::DeviceFreenectFactory() :
_numUnits(0), _allocatedUnits(0), _f_ctx(NULL)
{
	freenect_init(&_f_ctx, 0);

	// pre-configure all known capabilities
	_currentCapabilities.push_back("IMAGE_RGB");
	_currentCapabilities.push_back("IMAGE_RGB_FPS_30");
	_currentCapabilities.push_back("IMAGE_RGB_FPS_15");
	_currentCapabilities.push_back("IMAGE_RGB_RESOLUTION_640x480");
	_currentCapabilities.push_back("IMAGE_RGB_RESOLUTION_1280x1024");
	_currentCapabilities.push_back("IMAGE_YUV");
	_currentCapabilities.push_back("IMAGE_YUV_RESOLUTION_640x480");
	_currentCapabilities.push_back("IMAGE_YUV_RESOLUTION_1280x1024");
	_currentCapabilities.push_back("IMAGE_YUV_RAW");
	_currentCapabilities.push_back("IMAGE_YUV_RAW_RESOLUTION_640x480");
	_currentCapabilities.push_back("IMAGE_BAYER");
	_currentCapabilities.push_back("IMAGE_BAYER_RESOLUTION_640x480");
	_currentCapabilities.push_back("IMAGE_BAYER_RESOLUTION_1280x1024");
	_currentCapabilities.push_back("IMAGE_IR");
	_currentCapabilities.push_back("IMAGE_IR_RESOLUTION_640x480");
	_currentCapabilities.push_back("IMAGE_IR_DEPTH_8_BIT");
	_currentCapabilities.push_back("IMAGE_IR_DEPTH_10_BIT");
	_currentCapabilities.push_back("IMAGE_IR_DEPTH_10_BIT_PACKED");
	_currentCapabilities.push_back("IMAGE_IR_FPS_30");
	_currentCapabilities.push_back("IMAGE_IR_FPS_15");
	_currentCapabilities.push_back("IMAGE_Z");
	_currentCapabilities.push_back("IMAGE_Z_RESOLUTION_640x480");
	_currentCapabilities.push_back("IMAGE_Z_DEPTH_10_BIT");
	_currentCapabilities.push_back("IMAGE_Z_DEPTH_11_BIT");
	_currentCapabilities.push_back("IMAGE_Z_DEPTH_11_BIT_RLEDIFF");
	_currentCapabilities.push_back("IMAGE_Z_DEPTH_16_BIT");
	_currentCapabilities.push_back("IMAGE_Z_FPS_30");
	_currentCapabilities.push_back("IMAGE_Z_SMOOTHING");
	_currentCapabilities.push_back("IMAGE_Z_FLIP_H");
	_currentCapabilities.push_back("MOUNT_TILT");
	_currentCapabilities.push_back("AUDIO_INPUT");
	_currentCapabilities.push_back("ACCELEROMETER");
	_currentCapabilities.push_back("ACCELEROMETER_X");
	_currentCapabilities.push_back("ACCELEROMETER_Y");
	_currentCapabilities.push_back("ACCELEROMETER_Z");
	_currentCapabilities.push_back("LED");
	_currentCapabilities.push_back("LED_MULTI_COLOR");
	_currentCapabilities.push_back("LED_BLINK");
	_currentCapabilities.push_back("LED_COLOR_GREEN");
	_currentCapabilities.push_back("LED_COLOR_YELLOW");
	_currentCapabilities.push_back("LED_COLOR_RED");
	_currentCapabilities.push_back("LED_COLOR_GREEN_BLINK");
	_currentCapabilities.push_back("LED_COLOR_RED_YELLOW_BLINK");

	// no known extensions to pre-configure

	getTotalUnits();

} // DeviceFreenectFactory::DeviceFreenectFactory

DeviceFreenectFactory::~DeviceFreenectFactory()
{
	// free stuff, close freenect devices and context

	// <<<>>> we should freenect_close_device all outstanding freenect devices
	// but they're not being tracked yet

	// stop the sync
	freenect_sync_stop();

	// finally, close the context
	if(_f_ctx)
	{
		freenect_shutdown(_f_ctx);
		_f_ctx = NULL;
	} // if

} // DeviceFreenectFactory::~DeviceFreenectFactory

void DeviceFreenectFactory::getExtensions(StringContainer &stringContainer)
{
	stringContainer = _currentCapabilities;
} // DeviceFreenectFactory::getExtensions

void DeviceFreenectFactory::getCapabilities(StringContainer &stringContainer)
{
	stringContainer = _currentCapabilities;
} // DeviceFreenectFactory::getCapabilities

bool DeviceFreenectFactory::testCapability(const std::string capability)
{
	// <<<>>> in the future, this could support wildcards
	return(std::find(_currentCapabilities.begin(), _currentCapabilities.end(), capability) != _currentCapabilities.end());
} // DeviceFreenectFactory::testCapability

int DeviceFreenectFactory::getTotalUnits(void)
{
	_numUnits = freenect_num_devices(_f_ctx);
	return(_numUnits);
} // DeviceFreenectFactory::getTotalUnits

int DeviceFreenectFactory::getAvailableUnits(void)
{
	return(_numUnits - _allocatedUnits);
} // DeviceFreenectFactory::getAvailableUnits 


DeviceBase *DeviceFreenectFactory::createDevice(int unit, StringContainer capabilityCriteria)
{
	DeviceBase *newDevice = NULL;
	if(getAvailableUnits() > 0)
	{
		// create and return a DeviceFreenect
		newDevice = new DeviceFreenect(this, unit, capabilityCriteria); // this automatically calls back to DeviceFreenectFactory::increaseAllocatedUnits()
	} // if

	return(newDevice);
} // DeviceFreenectFactory::createDevice

void DeviceFreenectFactory::destroyDevice(DeviceBase *device)
{
	delete device; // this automatically calls back to DeviceFreenectFactory::decreaseAllocatedUnits()
} // DeviceFreenectFactory::destroyDevice










DeviceFreenect::DeviceFreenect(DeviceFreenectFactory *hostFactory, int unit, StringContainer capabilityCriteria) :
_hostFactory(hostFactory), DeviceBase(unit), _freenect_angle(0), _freenect_led(0), _f_dev(NULL), _defaultWidth(FREENECT_FRAME_W), _defaultHeight(FREENECT_FRAME_H), _defaultRGBdepth(3), _defaultZdepth(2)
{
	_hostFactory->increaseAllocatedUnits();
	// <<<>>> nothing to do at this point, sync interface is pretty spartan
} // DeviceFreenect::DeviceFreenect

DeviceFreenect::~DeviceFreenect()
{
	_hostFactory->decreaseAllocatedUnits();
	// <<<>>> close freenect device
	// sync interface is pretty sparse
	freenect_sync_stop();
} // DeviceFreenect::~DeviceFreenect

void *DeviceFreenect::requestCapabilityInterface(std::string capability)
{
	if(capability.compare(0, 9, "IMAGE_RGB") == 0)
		return(dynamic_cast<DeviceCapabilitiesImage *>(this));
	if(capability.compare(0, 7, "IMAGE_Z") == 0)
		return(dynamic_cast<DeviceCapabilitiesImage *>(this));
	// unimplemented so far
	/* 
	if(capability.compare(0, 5,  "MOUNT") == 0)
		return(this);
	if(capability.compare(0, 11, "AUDIO_INPUT") == 0)
		return(this);
	if(capability.compare(0, 7,  "ACCELEROMETER") == 0)
		return(this);
	if(capability.compare(0, 3,  "LED") == 0)
		return(this);
	*/
return(NULL); // we don't implement this interface
} // DeviceFreenect::requestCapabilityinterface









// DeviceCapabilitiesImage interface

bool DeviceFreenect::getImageSync(livescene::Image &image)
{
	if(image.getFormat() == livescene::VIDEO_RGB)
	{
        uint32_t ts;
        unsigned char* buffer( NULL );
        if(freenect_sync_get_video( (void**)&buffer, &ts, getUnit(), FREENECT_VIDEO_RGB ) == 0)
		{
			image.setTimestamp(ts);
			image.setData(buffer);
			image.invalidateInternalStats();
			return(true);
		} // if
	} // if
	else if(image.getFormat() == livescene::DEPTH_10BIT)
	{
        uint32_t ts;
        unsigned short* buffer( NULL );
        if(freenect_sync_get_depth( (void**)&buffer, &ts, getUnit(), FREENECT_DEPTH_10BIT ) == 0)
		{
			image.setTimestamp(ts);
			image.setData(buffer);
			image.setNull(1023);
			image.invalidateInternalStats();
			return(true);
		} // if
	} // else if
	else if(image.getFormat() == livescene::DEPTH_11BIT)
	{
        uint32_t ts;
        unsigned short* buffer( NULL );
        if(freenect_sync_get_depth( (void**)&buffer, &ts, getUnit(), FREENECT_DEPTH_11BIT ) == 0)
		{
			image.setTimestamp(ts);
			image.setData(buffer);
			image.setNull(2047);
			image.invalidateInternalStats();
			return(true);
		} // if
	} // else if
return(false);
} // DeviceFreenect::getImageSync

//DeviceFreenect::getImageAsync(livescene::Image &image, <<<>>> callback);
//{
//} // DeviceFreenect::getImageAsync

bool DeviceFreenect::getCurrentImageInfo(livescene::Image &image)
{
	// we can't currently change the settings and don't track current settings, so
	// this is something of a stub for when we can
	image = livescene::Image(getCurrentImageWidth(image.getFormat()), getCurrentImageHeight(image.getFormat()), getCurrentImageDepth(image.getFormat()), image.getFormat());
	return(false);
} // DeviceFreenect::getCurrentImageInfo

int DeviceFreenect::getCurrentImageWidth(const VideoFormat format)
{
	// we can't currently change the settings, so
	// this is something of a stub for when we can
	return(_defaultWidth);
} // DeviceFreenect::getCurrentImageWidth

int DeviceFreenect::getCurrentImageHeight(const VideoFormat format)
{
	// we can't currently change the settings, so
	// this is something of a stub for when we can
	return(_defaultHeight);
} // DeviceFreenect::getCurrentImageHeight

int DeviceFreenect::getCurrentImageDepth(const VideoFormat format)
{
	// we can't currently change the settings, so
	// this is something of a stub for when we can
	switch(format)
	{
	case livescene::VIDEO_RGB: return(_defaultRGBdepth); break;
	case livescene::DEPTH_10BIT: return(_defaultZdepth); break;
	default: return(0); break;
	} // switch format
} // DeviceFreenect::getCurrentImageDepth

bool DeviceFreenect::setCurrentImageInfo(const livescene::Image &image)
{
	// we can't currently change the settings, so
	// this is something of a stub for when we can
	// <<<>>> width, height, depth
	// <<<>>> format
	return(false);
} // DeviceFreenect::setCurrentImageInfo

bool DeviceFreenect::setCurrentImageWidth(VideoFormat format)
{
	// we can't currently change the settings, so
	// this is something of a stub for when we can
	// <<<>>> 
	return(false);
} // DeviceFreenect::setCurrentImageWidth

bool DeviceFreenect::setCurrentImageHeight(VideoFormat format)
{
	// we can't currently change the settings and don't track current settings, so
	// this is something of a stub for when we can
	// <<<>>> 
	return(false);
} // DeviceFreenect::setCurrentImageHeight

bool DeviceFreenect::setCurrentImageDepth(VideoFormat format)
{
	// we can't currently change the settings and don't track current settings, so
	// this is something of a stub for when we can
	// <<<>>> 
	return(false);
} // DeviceFreenect::setCurrentImageDepth




// namespace livescene
}
