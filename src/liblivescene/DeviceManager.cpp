// Copyright 2011 Skew Matrix Software and AlphaPixel

#include "liblivescene/DeviceManager.h"
#include <string>
#include <algorithm>

// Device types
#include "liblivescene/DeviceFreenect.h"

namespace livescene {


DeviceManager::DeviceManager()
{
	// populate internal list with Factories for all known device types
	_availableFactories.push_back(new DeviceFreenectFactory());
} // DeviceManager::DeviceManager


DeviceManager::~DeviceManager()
{
	// dispose of factories
	for(FactoryCollection::iterator removal = _availableFactories.begin(); removal != _availableFactories.end(); removal++)
	{
		// just delete the object referred to, no need to remove from the container
		// because the container will be destroyed in a moment.
		delete *removal;
	} // for
} // DeviceManager::~DeviceManager






// check name of provided DeviceFactory and if it matches, add it to a provided container of DeviceFactory's
// used by enumDevicesByName
class DeviceNameEnumeratorFunctor
{
public:
	DeviceNameEnumeratorFunctor(FactoryCollection &factoryCollection, const StringContainer &stringCriteria) : _factoryCollection(factoryCollection), _stringCriteria(stringCriteria) {}
	void operator ()(DeviceFactory *currentFactory)
	{
		std::string factoryType = currentFactory->getType();
		// manual iteration, not going to try to embed a second functor here so I can use for_each
		for(StringContainer::const_iterator currentString = _stringCriteria.begin(); currentString != _stringCriteria.end(); currentString++)
		{
			if(currentString->empty() || currentString->compare(factoryType) == 0)
			{ // matched, add it
				_factoryCollection.push_back(currentFactory);
			} // if
		} // for
	} // operator ()

private:
	FactoryCollection &_factoryCollection;
	const StringContainer &_stringCriteria;

}; // DeviceNameEnumeratorFunctor

bool DeviceManager::enumDevicesByName(FactoryCollection &factoryCollection, std::string name)
{
	StringContainer nameCriteria;
	nameCriteria.push_back(name);
	return(enumDevicesByName(factoryCollection, nameCriteria));
} // DeviceManager::enumDevicesByName


bool DeviceManager::enumDevicesByName(FactoryCollection &factoryCollection, const StringContainer nameCriteria)
{
	// call testCapabilities on each DeviceFactory and if it passes, add it to factoryCollection
	factoryCollection.clear();
	DeviceNameEnumeratorFunctor deviceNameEnumeratorFunctor(factoryCollection, nameCriteria);
	for_each(_availableFactories.begin(), _availableFactories.end(), deviceNameEnumeratorFunctor);
	return(true);
} // DeviceManager::enumDevicesByName





// call testCapabilities on provided DeviceFactory and if it passes, add it to a provided container of DeviceFactory's
// used by enumDevicesByCapability
class DeviceCapabilityEnumeratorFunctor
{
public:
	DeviceCapabilityEnumeratorFunctor(FactoryCollection &factoryCollection, const StringContainer &stringCriteria) : _factoryCollection(factoryCollection), _stringCriteria(stringCriteria) {}
	void operator ()(DeviceFactory *currentFactory)
	{
		if(currentFactory->testCapabilities(_stringCriteria))
			_factoryCollection.push_back(currentFactory);
	} // operator ()

private:
	FactoryCollection &_factoryCollection;
	const StringContainer &_stringCriteria;

}; // DeviceCapabilityEnumeratorFunctor


bool DeviceManager::enumDevicesByCapability(FactoryCollection &factoryCollection, const StringContainer capabilityCriteria)
{
	// call testCapabilities on each DeviceFactory and if it passes, add it to factoryCollection
	factoryCollection.clear();
	DeviceCapabilityEnumeratorFunctor deviceCapabilityEnumeratorFunctor(factoryCollection, capabilityCriteria);
	for_each(_availableFactories.begin(), _availableFactories.end(), deviceCapabilityEnumeratorFunctor);
	return(true);
} // DeviceManager::enumDevicesByCapability


DeviceBase *DeviceManager::aquireDeviceByName(std::string name, int unit)
{
	StringContainer nameCriteria, capabilityCriteria;
	FactoryCollection collection;
	nameCriteria.push_back(name); // load the name we'll search for
	enumDevicesByName(collection, nameCriteria); // search for a match
	if(!collection.empty())
	{
		int prefUnit = unit;
		if(prefUnit == -1)
		{
			prefUnit = collection[0]->getTotalUnits() - collection[0]->getAvailableUnits();
		} // if
		return(collection[0]->createDevice(prefUnit, capabilityCriteria));
	} // if

	return(NULL); // failure
} // DeviceManager::aquireDeviceByName


DeviceBase *DeviceManager::acquireDeviceByCapabilities(const StringContainer capabilityCriteria)
{
	FactoryCollection collection;
	enumDevicesByCapability(collection, capabilityCriteria); // search for a match
	if(!collection.empty())
	{
		// just grab the next available unit
		int prefUnit = collection[0]->getTotalUnits() - collection[0]->getAvailableUnits();
		return(collection[0]->createDevice(prefUnit, capabilityCriteria));
	} // if

	return(NULL); // failure
} // DeviceManager::acquireDeviceByCapabilities

// namespace livescene
}
