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


bool Background::accumulateBackgroundFromCleanPlate(const livescene::Image &cleanPlateRGB, const livescene::Image &cleanPlateZ, AccumulateMode mode, livescene::Image *foreZ)
{
	bool successZ(false), successRGB(false);
	successRGB = accumulateRGBBackgroundFromCleanPlate(cleanPlateRGB, mode);
	successZ = accumulateZBackgroundFromCleanPlate(cleanPlateZ, mode, foreZ);
	return(successRGB && successZ);
} // Background::accumulateBackgroundFromCleanPlate


bool Background::accumulateRGBBackgroundFromCleanPlate(const livescene::Image &cleanPlateRGB, AccumulateMode mode)
{
return(false); // <<<>>> not implemented
} // Background::accumulateRGBBackgroundFromCleanPlate


bool Background::accumulateZBackgroundFromCleanPlate(const livescene::Image &cleanPlateZ, AccumulateMode mode, livescene::Image *foreZ)
{
	// Once this exceeds 1/n where n is the largest reasonable Z value, it has no more effect and we'll skip it
	if(_bgZ.getAccumulation() > 1023) return(false);

	const int maxSample = _bgZ.getSamples();
	const float accumWeight = 1.0f / (_bgZ.getAccumulation() + 1); // only used in AVERAGE mode
	unsigned short *bgZData = (unsigned short *)_bgZ.getData();
	unsigned short bgzZnull = (unsigned short)_bgZ.getNull();
	unsigned short *foreZData = NULL;
	unsigned short *cleanZData = (unsigned short *)cleanPlateZ.getData();
	unsigned short cleanZnull = (unsigned short)cleanPlateZ.getNull();
	unsigned short foreZnull = 0;

	if(foreZ)
	{
		foreZData = (unsigned short *)foreZ->getData();
		foreZnull = (unsigned short)foreZ->getNull();
	} // if

	int width(cleanPlateZ.getWidth()), height(cleanPlateZ.getHeight());


	int sample = 0;
	for(int line = 0; line < height; ++line)
	{
		for(int column = 0; column < width; ++column)
		{
			sample = column + line * width; // precacluate array subscript
			const int cleanZsample = cleanZData[sample];
			if(cleanZsample != cleanZnull && cleanZsample != 0)
			{
				long delta = 0;
				const int cleanZepsilon = (int)(cleanZsample * _discriminationEpsilonPercent); // margin of noise/error
				switch(mode)
				{
				case MIN_Z:
				case MIN_Z_ADJACENT:
					{
						// is the sample from the clean plate nearer to the sensor
						// if mode == _ADJACENT, it must also be close enough in Z to an adjacent sample in the background
						if(cleanZsample < bgZData[sample] && (mode == MIN_Z || (_bgZ.minimumDeltaToNeighbors(column, line, cleanZsample, delta) && delta <= cleanZepsilon)))
						{
							bgZData[sample] = cleanZsample;
							if(foreZ)
							{ // knock it out of foreground
								foreZData[sample] = foreZnull;
							} // if
						} // if
						break;
					} // MIN_Z
				case MAX_Z:
				case MAX_Z_ADJACENT:
					{
						// is the sample from the clean plate further from the sensor
						// if mode == _ADJACENT, it must also be close enough in Z to an adjacent sample in the background
						if(cleanZsample > bgZData[sample] && (mode == MAX_Z || (_bgZ.minimumDeltaToNeighbors(column, line, cleanZsample, delta) && delta <= cleanZepsilon)))
						{
							bgZData[sample] = cleanZsample;
							if(foreZ)
							{ // knock it out of foreground
								foreZData[sample] = foreZnull;
							} // if
						} // if
						break;
					} // MAX_Z
				case AVERAGE_Z:
				case AVERAGE_Z_ADJACENT:
					{
						// if mode == _ADJACENT, is the sample from the clean plate close enough in Z to an adjacent sample in the background?
						if(mode == AVERAGE_Z || (_bgZ.minimumDeltaToNeighbors(column, line, cleanZsample, delta) && delta <= cleanZepsilon))
						{
							if(bgZData[sample] == cleanZnull) // is it a clean replacement -- no existing samples at this location
							{ // just overwrite no-data value
								bgZData[sample] = cleanZsample; 
							} // if
							else
							{ // average together existing data values using weight and inverse weight derived from accumuation (truncate to short after add)
								bgZData[sample] = (unsigned short)((accumWeight * (float)cleanZsample) + ((1.0f - accumWeight) * (float)bgZData[sample]));
							} // else
							if(foreZ)
							{ // knock it out of foreground
								foreZData[sample] = foreZnull;
							} // if
						} // if part of background
						break;
					} // AVERAGE_Z
				} // mode
			} // if not null
		} // for column
	} // for lines

	_bgZ.increaseAccumulation();

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
	for(int sample = 0; sample < maxSample; ++sample)
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
