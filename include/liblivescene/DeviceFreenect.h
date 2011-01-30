// Copyright 2011 Skew Matrix Software and AlphaPixel

#ifndef __LIVESCENE_DEVICEFREENECT_H__
#define __LIVESCENE_DEVICEFREENECT_H__ 1

#include "liblivescene/Export.h"
#include "liblivescene/Device.h"
#include "liblivescene/DeviceFactory.h"
#include "liblivescene/DeviceCapabilities.h"
#include <string>
#include <libfreenect.h>



namespace livescene {

// forward delcaration
class DeviceFreenect;

/** \defgroup Freenect Freenect Device */
/*@{*/

/** \brief DeviceFreenectFactory for the Microsoft Kinect via the libfreenect API.
Supports these capabilities interfaces: Image
Should also supports these capabilities interfaces later: Mount, Audio, Accelerometer, LED

*/



class DeviceFreenectFactory : public DeviceFactory
{
	friend class DeviceFreenect; // to allow use of increase/decrease allocated units
	public:
		/** */
		LIVESCENE_EXPORT DeviceFreenectFactory();
		LIVESCENE_EXPORT ~DeviceFreenectFactory();

		/** get Type of LiveScene Device, like "DeviceFreenect" */
		LIVESCENE_EXPORT std::string getType(void) {return("DeviceFreenect");}

		/** get API used by device, like "libfreenect" */
		LIVESCENE_EXPORT std::string getAPI(void) {return("libfreenect");}

		/** get Vendor string. Meaning is specific to the LiveScene Device. Could be "Microsoft XBox 360 Kinect" */
		LIVESCENE_EXPORT std::string getVendor(void) {return("Microsoft XBox 360 Kinect");}

		/** get Version string. Could be version of API library */
		LIVESCENE_EXPORT std::string getVersion(void) {return("");}

		/** get hardware description for this Device Unit. "XBox 360 Kinect V1" */
		LIVESCENE_EXPORT std::string getHardware(void) {return("XBox 360 Kinect V1");}

		/** get extension strings for this API/Device. Similar to OpenGL extensions. */
		LIVESCENE_EXPORT void getExtensions(StringContainer &stringContainer);
		
		/** get capability strings for this API/Device. Similar to OpenGL extensions. */
		LIVESCENE_EXPORT void getCapabilities(StringContainer &stringContainer);

		/** test capability strings against provided string. Wildcards may be permitted. */
		LIVESCENE_EXPORT bool testCapability(const std::string capability);

		/** How many total units are present? */
		LIVESCENE_EXPORT int getTotalUnits(void);

		/** How many units are still available? */
		LIVESCENE_EXPORT int getAvailableUnits(void);

		/** Create a device using the supplied unit number, and optionally some capabilities */
		LIVESCENE_EXPORT DeviceBase *createDevice(int unit, StringContainer capabilityCriteria);

		/** Destroy a device. */
		LIVESCENE_EXPORT void DeviceFreenectFactory::destroyDevice(DeviceBase *device);


	private:
		void increaseAllocatedUnits(void) {_allocatedUnits++;}
		void decreaseAllocatedUnits(void) {if(_allocatedUnits > 0) _allocatedUnits--;}
		int _numUnits;
		int _allocatedUnits;
		StringContainer _currentCapabilities;
		StringContainer _currentExtensions;
		freenect_context *_f_ctx;


}; // DeviceFreenectFactory


/** \brief implementation of a Device for the Microsoft Kinect via the libfreenect API.

*/



class LIVESCENE_EXPORT DeviceFreenect : public DeviceBase, public DeviceCapabilitiesImage
	/*, public DeviceCapabilitiesMount, public DeviceCapabilitiesAudio, public DeviceCapabilitiesAccelerometer, public DeviceCapabilitiesLED */
{
	public:
		/** */
		DeviceFreenect(DeviceFreenectFactory *hostFactory, int unit, StringContainer capabilityCriteria);
		~DeviceFreenect();

		// all these methods are wrappers for the same functionality on this Device's factory, which knows the answers

		/** get Type of LiveScene Device, like "DeviceFreenect" */
		std::string getType(void) {return(_hostFactory->getType());}

		/** get API used by device, like "libfreenect" */
		std::string getAPI(void) {return(_hostFactory->getAPI());}

		/** get Vendor string. Meaning is specific to the LiveScene Device. Could be "Microsoft XBox 360 Kinect" */
		std::string getVendor(void) {return(_hostFactory->getVendor());}

		/** get Version string. Could be version of API library */
		std::string getVersion(void) {return(_hostFactory->getVersion());}

		/** get hardware description for this Device Unit. "XBox 360 Kinect V1" */
		std::string getHardware(void) {return(_hostFactory->getHardware());}

		/** get extension strings for this API/Device. Similar to OpenGL extensions. */
		void getExtensions(StringContainer &stringContainer) {_hostFactory->getExtensions(stringContainer);}
		
		/** get capability strings for this API/Device. Similar to OpenGL extensions. */
		void getCapabilities(StringContainer &stringContainer) {_hostFactory->getCapabilities(stringContainer);}

		/** test capability strings against provided string. Wildcards may be permitted. */
		bool testCapability(const std::string capability) {return(_hostFactory->testCapability(capability));}

		/** test capability strings against provided strings. Wildcards may be permitted  */
		bool testCapabilities(const StringContainer &stringContainer) {return(_hostFactory->testCapabilities(stringContainer));}

		/** request a pointer to an interface object that implements the desired
		capability. Returns NULL for failure. dynamic_cast it to the desired interface
		if successful. */
		void *requestCapabilityInterface(std::string capability);

		// releases interfaces obtained with the above.
		virtual void releaseCapabilityInterface(void *interface) {}; // no-op currently


		// From DeviceCapabilitiesImage
		/** Synchronously gets an image using the specified format and fills in width, height, depth and data.
		Returns true if successful. */
		bool getImageSync(livescene::Image &image);

		/** Asynchronously sets an image using the specified format and fills in width, height, depth and data.
		<<<>>> Notification of completion is performed with the callback. */
		//getImageAsync(livescene::Image &image, <<<>>> callback);

		/** Gets image attributes using the specified format and fills in width, height, depth. Returns true if successful. */
		bool getCurrentImageInfo(livescene::Image &image);

		/** Gets current width for specified format */
		int getCurrentImageWidth(const VideoFormat format);

		/** Gets current height for specified format */
		int getCurrentImageHeight(const VideoFormat format);

		/** Gets current depth for specified format.
		Depth is the number of bytes per pixel, not actual number of bits utilized per pixel. */
		int getCurrentImageDepth(const VideoFormat format);

		/** Sets image attributes using the specified format using width, height, depth.
		Returns true if successful. */
		bool setCurrentImageInfo(const livescene::Image &image);

		/** Sets current width for specified format. Returns true if successful. */
		bool setCurrentImageWidth(VideoFormat format);

		/** Sets current height for specified format. Returns true if successful. */
		bool setCurrentImageHeight(VideoFormat format);

		/** Sets current depth for specified format. Returns true if successful. */
		bool setCurrentImageDepth(VideoFormat format);


	private:
		freenect_device *_f_dev;
		int _freenect_angle;
		int _freenect_led;
		DeviceFreenectFactory *_hostFactory;
		int _defaultWidth, _defaultHeight, _defaultRGBdepth, _defaultZdepth;

}; // DeviceFreenect

/*@}*/


// namespace livescene
}

// __LIVESCENE_DEVICEFREENECT_H__
#endif
