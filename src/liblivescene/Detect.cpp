// Copyright 2011 Skew Matrix Software and AlphaPixel

#include "liblivescene/Detect.h"
#include <algorithm> // std::min/max
#include <limits> // numeric_limits::max

namespace livescene {

const float *BodyMass::getHandCentroid(unsigned int bodyNum, unsigned int handNum) const
{ // ignore bodyNum
	if(handNum == 1)
	{
		return(_handCentroid[1]);
	} // if
	else // all other hand values give you hand 0, you octopods.
	{
		return(_handCentroid[0]);
	} // else
} // BodyMass::getHandCentroid

unsigned int BodyMass::detect(livescene::Image &foreZ)
{
unsigned int BodiesDetected(0);
livescene::ImageStatistics statsBodyX, statsBodyY, statsBodyZ;

// calculate foreground stats
foreZ.calcInternalStatsXYZ();

// recalculate stats of body, using bounds of nth standard deviation of foreground to exclude extraneous noise
const float nthStdDev(2.8f);
const float
	xmin(foreZ.getInternalStatsX().getMean() - foreZ.getInternalStatsX().getStdDev() * nthStdDev),
	ymin(foreZ.getInternalStatsY().getMean() - foreZ.getInternalStatsY().getStdDev() * nthStdDev), 
	xmax(foreZ.getInternalStatsZ().getMean() - foreZ.getInternalStatsZ().getStdDev() * nthStdDev), 
	ymax(foreZ.getInternalStatsX().getMean() + foreZ.getInternalStatsX().getStdDev() * nthStdDev), 
	zmin(foreZ.getInternalStatsY().getMean() + foreZ.getInternalStatsY().getStdDev() * nthStdDev), 
	zmax(foreZ.getInternalStatsZ().getMean() + foreZ.getInternalStatsZ().getStdDev() * nthStdDev);
const signed int // std::min and max get angry if the type of the two args is not identical
	xminIntSigned(xmin),
	yminIntSigned(ymin),
	xmaxIntSigned(xmax),
	ymaxIntSigned(ymax);
const signed int
	xminIntClamped(std::max(xminIntSigned, 0)),
	yminIntClamped(std::max(yminIntSigned, 0)),
	xmaxIntClamped(std::min(xmaxIntSigned, (signed)foreZ.getWidth())),
	ymaxIntClamped(std::min(ymaxIntSigned, (signed)foreZ.getHeight()));
BoxApproveCallback stdDevBoxApprove(osg::BoundingBox(
	xmin, // xmin
	ymin, // ymin
	zmin, // zmin
	xmax, // xmax
	ymax, // ymax
	zmax // zmax
	));

// find the actual body mass using filtered input
// this is faster by passing the XY bounding box as a limiter
foreZ.calcStatsXYZBounded(xminIntClamped, yminIntClamped, xmaxIntClamped, ymaxIntClamped,
	&statsBodyX, &statsBodyY, &statsBodyZ, &stdDevBoxApprove);

if(statsBodyZ.getNumSamples() >= OSG_LIVESCENEVIEW_DETECT_BODY_SAMPLES)
{
	_centroid[0] = statsBodyX.getMean();
	_centroid[1] = statsBodyY.getMean();
	_centroid[2] = statsBodyZ.getMean();
	_stdDev[0] = statsBodyX.getStdDev();
	_stdDev[1] = statsBodyY.getStdDev();
	_stdDev[2] = statsBodyZ.getStdDev();
	_bodyPresent = true;
	BodiesDetected = 1;
} // if
else
{
	clear();
} // else

return(BodiesDetected);
} // BodyMass::detect


unsigned int BodyMass::detectHands(const livescene::Image &foreZ)
{ // this currently only understands one body and its two hands
unsigned int HandsDetected(0);
// for the following constants:
// M is the horizontal distance from the center of the body to the outside of one side
// regardless of whether the hands are extended to the sides or not.
// aka half of the body mass width
// N is the vertical distance from the waist to the top of the head (or bottom of feet)
// aka half of the body mass height
// These outlying search ratios are used to determine how far around/above the nominal body
// mass should be searched for extended hands.
const float outlyingSearchRatioM(2.2f), // 2.2 times M distance to either side of body centroid
			outlyingSearchRatioN(1.5f); // 1.5 times N above centroid is an additional half of N distance above top of head

bool keepSearching(true);

if(!getBodyPresent()) return(0); // need to know where the body is to start detecting hands

// determine body-plausible region bounds
const signed int
	bodySearchMinX(getBodyCentroid(0)[0] - (getBodyHalfExtent(0)[0] * outlyingSearchRatioM)),
	bodySearchMaxX(getBodyCentroid(0)[0] + (getBodyHalfExtent(0)[0] * outlyingSearchRatioM)),
	bodySearchMinY(getBodyCentroid(0)[1] - (getBodyHalfExtent(0)[1] * outlyingSearchRatioN)),
	bodySearchMaxY(getBodyCentroid(0)[1]),
	bodyThresholdZ(getBodyCentroid(0)[2] - (getBodyHalfExtent(0)[2] * 2.5)); // anything nearer than body front Z margin qualifies

// clamp to image edge bounds
const signed int
	bodySearchMinXClamped(std::max(bodySearchMinX, 0)),
	bodySearchMaxXClamped(std::min(bodySearchMaxX, (signed)foreZ.getWidth())),
	bodySearchMinYClamped(std::max(bodySearchMinY, 0)),
	bodySearchMaxYClamped(std::min(bodySearchMaxY, (signed)foreZ.getHeight())),
	bodyThresholdZClamped(std::max(bodyThresholdZ, 0));

while(keepSearching)
{
	unsigned int resultX(0), resultY(0), resultZ(0);
	// search body region for nearest Z point closer than body front
	if(findMinimumZLocation(foreZ, bodySearchMinXClamped, bodySearchMinYClamped,
		bodySearchMaxXClamped, bodySearchMaxYClamped, bodyThresholdZClamped, resultX, resultY, resultZ))
	{
		_handCentroid[0][0] = resultX;
		_handCentroid[0][1] = resultY;
		_handCentroid[0][2] = resultZ;

		// <<<>>> do some better filtering, and wipe out the hand data so a second hand can be searched for
		HandsDetected = 1;
	} // if
	break;
} // while

return(HandsDetected);
} // BodyMass::detectHands


bool BodyMass::findMinimumZLocation(const livescene::Image &foreZ,
									const unsigned int &Xlow, const unsigned int &Ylow, const unsigned int &Xhigh, const unsigned int &Yhigh,
									const short &zThreshold, unsigned int &minZlocX, unsigned int &minZlocY, unsigned int &minZvalueZ)
{
	bool thresholdFound(false);
	short minDistance(std::numeric_limits<short>::max());
	unsigned int width(foreZ.getWidth()), height(foreZ.getHeight());
	short *depthBuffer = (short *)foreZ.getData();

	unsigned int lineSub(0);
	for(unsigned int line = Ylow; line < Yhigh; ++line)
	{
		lineSub = line * width;
		for(unsigned int column = Xlow; column < Xhigh; ++column)
		{
			short originalDepth = depthBuffer[lineSub + column];
			// the order of these tests is immaterial, but they've been arranged in the order
			// of most likely to fail first to speed things up
			if(originalDepth <= zThreshold && originalDepth < minDistance && foreZ.isCellValueValid(originalDepth)) // is it valid, nearer than the threshold AND nearer than previous result?
			{
				// record it as the best location seen so far
				minDistance = originalDepth;
				thresholdFound = true;
				minZlocX = column;
				minZlocY = line;
			} // if
		} // for
	} // for lines

	if(thresholdFound)
	{
		minZvalueZ = minDistance;
	} // if

	return(thresholdFound);
} // BodyMass::findMinimumZLocation


// namespace livescene
}
