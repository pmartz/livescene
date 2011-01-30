// Copyright 2011 Skew Matrix Software and AlphaPixel

#ifndef __LIVESCENE_DEVICE_H__
#define __LIVESCENE_DEVICE_H__ 1

#include "liblivescene/Export.h"
#include <string>
#include <vector>


namespace livescene {


/** \defgroup Device Device Management */
/*@{*/

/** \brief Container of strings. Used by device methods.
*/
typedef std::vector<std::string> StringContainer;


/** \brief Base class all Devices are derived from. Provides basic capability query interfaces.

*/


class LIVESCENE_EXPORT DeviceBase
{
	public:
		/** */
		DeviceBase(int unit) : _unit(unit) {}
		virtual ~DeviceBase() {};

		int getUnit(void) const {return(_unit);}

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
		virtual bool testCapabilities(const StringContainer &stringContainer) = 0;

		/** request a pointer to an interface object that implements the desired
		capability. Returns NULL for failure. dynamic_cast it to the desired interface
		if successful.
		<<<>>> TBD: Do we need a releaseCapabilityInterface() ?
		TBD: Should we return some sort of abstract interface base class so we can safely dynamic_cast from it?*/
		virtual void *requestCapabilityInterface(std::string capability) = 0;


	private:
		int _unit;
}; // DeviceBase


/*@}*/


// namespace livescene
}

// __LIVESCENE_DEVICE_H__
#endif
