// Copyright 2011 Skew Matrix Software and AlphaPixel

#include "liblivescene/Background.h"

namespace livescene {

bool Background::loadBackgroundFromCleanPlate(const livescene::Image &cleanPlateRGB, const livescene::Image &cleanPlateZ)
{
	bool successZ(false), successRGB(false);
	successRGB = loadRGBBackgroundFromCleanPlate(cleanPlateRGB);
	successZ = loadZBackgroundFromCleanPlate(cleanPlateZ);
	return(successRGB && successZ);
} // Background::loadBackgroundFromCleanPlate

bool Background::loadRGBBackgroundFromCleanPlate(const livescene::Image &cleanPlateRGB)
{
	_bgRGB = cleanPlateRGB;
	_backgroundAvailable = true; // technically not true until you load the Z too
	return(false);
} // Background::loadRGBBackgroundFromCleanPlate

bool Background::loadZBackgroundFromCleanPlate(const livescene::Image &cleanPlateZ)
{
	_bgZ = cleanPlateZ;
	_backgroundAvailable = true; // technically not true until you load the RGB too
	return(false);
} // Background::loadZBackgroundFromCleanPlate


// <<<>>> not yet implemented, accumulate non-changing pixels from live stream into background
bool Background::accumulateBackgroundFromLive(const livescene::Image &liveRGB, const livescene::Image &liveZ)
{
	// <<<>>>
	return(false);
} // Background::accumulateBackgroundFromLive

bool Background::accumulateRGBBackgroundFromLive(const livescene::Image &liveRGB)
{
	// <<<>>>
	return(false);
} // Background::accumulateRGBBackgroundFromLive

bool Background::accumulateZBackgroundFromLive(const livescene::Image &liveZ)
{
	// <<<>>>
	return(false);
} // Background::accumulateZBackgroundFromLive


// extracts the foreground from the background plate
bool Background::extractZBackground(const livescene::Image &liveZ, livescene::Image &foregroundZ)
{
	if(!_backgroundAvailable) return(false);

	unsigned short *bgZData = (unsigned short *)_bgZ.getData();
	unsigned short *liveZData = (unsigned short *)liveZ.getData();
	unsigned short *foreZData = (unsigned short *)foregroundZ.getData();
	unsigned short foreZnull = (unsigned short)foregroundZ.getNull();
	
	const int maxSample = _bgZ.getWidth() * _bgZ.getHeight();
	for(int sample = 0; sample < maxSample; sample++)
	{
		const int liveZsample = liveZData[sample];
		const int liveZepsilon = (int)(liveZsample * _discriminationEpsilon); // margin of noise/error
		// is current sample at, beyond or just in front of known background depth?
		if(liveZsample + liveZepsilon >= bgZData[sample])
		{ // it's background
			foreZData[sample] = foreZnull; // mark it as null
		} // if
		else
		{ // it's foreground
			foreZData[sample] = liveZsample; // copy it over
		} // else
	} // for

	return(false);
} // Background::extractZBackground




// namespace livescene
}
