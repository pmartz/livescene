// Copyright 2011 Skew Matrix Software and AlphaPixel

#ifndef __LIVESCENE_DEVICEMANAGER_H__
#define __LIVESCENE_DEVICEMANAGER_H__ 1

#include "liblivescene/Export.h"
#include "liblivescene/Device.h"
#include "liblivescene/DeviceFactory.h"
#include <string>
#include <utility>
#include <vector>

namespace livescene {


/** \defgroup Device Device Management */
/*@{*/


/** \brief A collection of DeviceFactories. Used by DeviceManager.
*/
typedef std::vector<DeviceFactory *> FactoryCollection;


/** \brief Singleton to manage access to LiveScene Devices. This singleton
initializes when first used and persists until program exit, and the initialization
is potentially not thread-safe. Other Singleton patterns may be better, and could
be utilized in the future.

*/

class LIVESCENE_EXPORT DeviceManager
{
	public:
		DeviceManager();
		~DeviceManager();

		/** List all devices meeting a name criteria. Returns true for success. */
		bool enumDevicesByName(FactoryCollection &factoryCollection, std::string name);
		bool enumDevicesByName(FactoryCollection &factoryCollection, const StringContainer nameCriteria);

		/** List all devices meeting capability criteria. Returns true for success. */
		bool enumDevicesByCapability(FactoryCollection &factoryCollection, const StringContainer capabilityCriteria);

		/** Acquire a specific device by its name and unit number.
		unit = -1 signifies use next available unit. */
		DeviceBase *aquireDeviceByName(std::string name, int unit = -1);

		/** Acquire a generic by requesting capabilities. Always chooses first/next available unit. */
		DeviceBase *acquireDeviceByCapabilities(const StringContainer capabilityCriteria);

	private:
		static DeviceManager *_instance;
		FactoryCollection _availableFactories;

}; // DeviceManager

/*@}*/


// namespace livescene
}

// __LIVESCENE_DEVICEMANAGER_H__
#endif
