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
	_bgRGB = livescene::Image(cleanPlateRGB, true); // clone image for persistent storage. Note this does two copies, which is inefficient
	_backgroundAvailable = true; // technically not true until you load the Z too
	return(true);
} // Background::loadRGBBackgroundFromCleanPlate

bool Background::loadZBackgroundFromCleanPlate(const livescene::Image &cleanPlateZ)
{
	_bgZ = livescene::Image(cleanPlateZ, true); // clone image for persistent storage. Note this does two copies, which is inefficient
	_backgroundAvailable = true; // technically not true until you load the RGB too
	return(true);
} // Background::loadZBackgroundFromCleanPlate


bool Background::accumulateBackgroundFromCleanPlate(const livescene::Image &cleanPlateRGB, const livescene::Image &cleanPlateZ, AccumulateMode mode)
{
	bool successZ(false), successRGB(false);
	successRGB = accumulateRGBBackgroundFromCleanPlate(cleanPlateRGB, mode);
	successZ = accumulateZBackgroundFromCleanPlate(cleanPlateZ, mode);
	return(successRGB && successZ);
} // Background::accumulateBackgroundFromCleanPlate


bool Background::accumulateRGBBackgroundFromCleanPlate(const livescene::Image &cleanPlateRGB, AccumulateMode mode)
{
return(false); // <<<>>> not implemented
} // Background::accumulateRGBBackgroundFromCleanPlate


bool Background::accumulateZBackgroundFromCleanPlate(const livescene::Image &cleanPlateZ, AccumulateMode mode)
{
	const int maxSample = _bgZ.getSamples();
	const float accumWeight = 1.0f / (cleanPlateZ.getAccumulation() + 1.0f); // only used in AVERAGE mode
	unsigned short *bgZData = (unsigned short *)_bgZ.getData();
	unsigned short *cleanZData = (unsigned short *)cleanPlateZ.getData();
	unsigned short cleanZnull = (unsigned short)cleanPlateZ.getNull();
	for(int sample = 0; sample < maxSample; sample++)
	{
		const int cleanZsample = cleanZData[sample];
		if(cleanZsample != cleanZnull)
		{
			switch(mode)
			{
			case MIN_Z:
				{
					if(cleanZsample < bgZData[sample])
					{
						bgZData[sample] = cleanZsample;
					} // if
					break;
				} // MIN_Z
			case MAX_Z:
				{
					if(cleanZsample > bgZData[sample])
					{
						bgZData[sample] = cleanZsample;
					} // if
					break;
				} // MAX_Z
			case AVERAGE_Z:
				{
					if(bgZData[sample] == cleanZnull)
					{ // just overwrite no-data value
						bgZData[sample] = cleanZsample; 
					} // if
					else
					{ // average together existing data values
						bgZData[sample] = (unsigned short)(accumWeight * ((float)bgZData[sample] + (float)cleanZsample));
					} // else
					break;
				} // AVERAGE_Z
			} // mode
		} // if not null
	} // for

return(true);
} // Background::accumulateZBackgroundFromCleanPlate


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
	
	const int maxSample = _bgZ.getSamples();
	for(int sample = 0; sample < maxSample; sample++)
	{
		const int liveZsample = liveZData[sample];
		const int liveZepsilon = (int)(liveZsample * _discriminationEpsilonPercent); // margin of noise/error
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
