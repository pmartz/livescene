// Copyright 2011 Skew Matrix Software and AlphaPixel

#include "liblivescene/DeviceFactory.h"
#include <algorithm>

namespace livescene {


/** \brief call DeviceBase's testCapability on each item passed and mark if any fail
*/
class TestCapabilityFunctor
{
	public:
		TestCapabilityFunctor(DeviceFactory *deviceFactory, bool &failedFlag) : _deviceFactory(deviceFactory), _failedFlag(failedFlag) {}
		void operator ()(std::string stringToTest) {if(!_deviceFactory->testCapability(stringToTest)) _failedFlag = true;}

	private:
		DeviceFactory *_deviceFactory;
		bool &_failedFlag;

}; // TestCapabilityFunctor

bool DeviceFactory::testCapabilities(const StringContainer &stringContainer)
{
	bool failed = false;

	// call DeviceFactory's testCapability on each item in stringContainer and mark if any fail
	TestCapabilityFunctor testCapabilityFunctor(this, failed);
	for_each(stringContainer.begin(), stringContainer.end(), testCapabilityFunctor);

	return(!failed);
}; // DeviceFactory::testCapabilities

// namespace livescene
}
