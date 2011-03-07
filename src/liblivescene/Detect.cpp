// Copyright 2011 Skew Matrix Software and AlphaPixel

#include "liblivescene/Detect.h"
#include <algorithm> // std::min/max

namespace livescene {

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


// namespace livescene
}
