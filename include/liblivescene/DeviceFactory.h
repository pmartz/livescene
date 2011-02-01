// Copyright 2011 Skew Matrix Software and AlphaPixel

#ifndef __LIVESCENE_DEVICEFACTORY_H__
#define __LIVESCENE_DEVICEFACTORY_H__ 1

#include "liblivescene/Export.h"
#include "liblivescene/Device.h"
#include <string>
#include <utility>
#include <vector>

namespace livescene {


/** \defgroup Device Device Management */
/*@{*/


/** \brief Abstract base class for all Device Factories for different implementations.

*/

class LIVESCENE_EXPORT DeviceFactory
{
	public:

		/** get Type of LiveScene Device, like "DeviceFreenect" */
		virtual std::string getType(void) = 0;

		/** get API used by device, like "libfreenect" */
		virtual std::string getAPI(void) = 0;

		/** get Vendor string. Meaning is specific to the LiveScene Device. Could be "Microsoft XBox 360 Kinect" */
		virtual std::string getVendor(void) = 0;

		/** get Version string. Could be version of API library */
		virtual std::string getVersion(void) = 0;

		/** get hardware description for this Device Unit. "XBox 360 Kinect V1" */
		virtual std::string getHardware(void) = 0;

		/** get extension strings for this API/Device. Similar to OpenGL extensions. */
		virtual void getExtensions(StringContainer &stringContainer) = 0;
		
		/** get capability strings for this API/Device. Similar to OpenGL extensions. */
		virtual void getCapabilities(StringContainer &stringContainer) = 0;

		/** test capability strings against provided string. Wildcards may be permitted. */
		virtual bool testCapability(const std::string capability) = 0;

		/** test capability strings against provided strings. Wildcards may be permitted  */
		bool testCapabilities(const StringContainer &stringContainer);

		/** How many total units are present? */
		virtual int getTotalUnits(void) = 0;

		/** How many units are still available? */
		virtual int getAvailableUnits(void) = 0;

		/** Create a device using the supplied unit number, and optionally some capabilities */
		virtual DeviceBase *createDevice(int unit, StringContainer capabilityCriteria) = 0;

		/** Destroy a device. */
		virtual void destroyDevice(DeviceBase *device) = 0;

}; // DeviceFactory

/*@}*/


// namespace livescene
}

// __LIVESCENE_DEVICEFACTORY_H__
#endif
