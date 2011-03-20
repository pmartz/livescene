// Copyright 2011 Skew Matrix Software and AlphaPixel

#include "liblivescene/Detect.h"
#include <algorithm> // std::min/max
#include <limits> // numeric_limits::max
#include <cassert>
#include <iostream>

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
	zmin(foreZ.getInternalStatsZ().getMean() - foreZ.getInternalStatsZ().getStdDev() * nthStdDev), 
	xmax(foreZ.getInternalStatsX().getMean() + foreZ.getInternalStatsX().getStdDev() * nthStdDev), 
	ymax(foreZ.getInternalStatsY().getMean() + foreZ.getInternalStatsY().getStdDev() * nthStdDev), 
	zmax(foreZ.getInternalStatsZ().getMean() + foreZ.getInternalStatsZ().getStdDev() * nthStdDev);
const signed int // std::min and max get angry if the type of the two args is not identical
	xminIntSigned(xmin),
	yminIntSigned(ymin),
	xmaxIntSigned(xmax),
	ymaxIntSigned(ymax);
const signed int
	xminIntClamped(std::max(xminIntSigned, 0)),
	yminIntClamped(std::max(yminIntSigned, 0)),
	xmaxIntClamped(std::min(xmaxIntSigned, (signed)foreZ.getWidth() - 1)),
	ymaxIntClamped(std::min(ymaxIntSigned, (signed)foreZ.getHeight() - 1));
ZApproveCallback stdDevZApprove(zmin, zmax);

// troubleshooting asserts, remove later
assert(xminIntClamped >= 0);
assert(yminIntClamped >= 0);
assert(xmaxIntClamped >= 0);
assert(ymaxIntClamped >= 0);
assert(xminIntClamped < (signed)foreZ.getWidth());
assert(yminIntClamped < (signed)foreZ.getHeight());
assert(xmaxIntClamped < (signed)foreZ.getWidth());
assert(ymaxIntClamped < (signed)foreZ.getHeight());

// find the actual body mass using filtered input
// this is faster by passing the XY bounding box as a limiter
foreZ.calcStatsXYZBounded(xminIntClamped, yminIntClamped, xmaxIntClamped, ymaxIntClamped,
	&statsBodyX, &statsBodyY, &statsBodyZ, &stdDevZApprove);

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
const float outlyingSearchRatioM(2.2f * 2.0f), // 2.2 times M distance to either side of body centroid
			outlyingSearchRatioN(1.5f * 2.0f), // 1.5 times N above centroid is an additional half of N distance above top of head
			outlyingSearchRatioO(0.5f * 2.0f); // .5 times N below centroid allows hands to extend just below the belt

if(!getBodyPresent()) return(0); // need to know where the body is to start detecting hands

// determine body-plausible region bounds
const signed int
	bodySearchMinX(getBodyCentroid(0)[0] - (getBodyHalfExtent(0)[0] * outlyingSearchRatioM)),
	bodySearchMaxX(getBodyCentroid(0)[0] + (getBodyHalfExtent(0)[0] * outlyingSearchRatioM)),
	bodySearchMinY(getBodyCentroid(0)[1] - (getBodyHalfExtent(0)[1] * outlyingSearchRatioN)),
	bodySearchMaxY(getBodyCentroid(0)[1] + (getBodyHalfExtent(0)[1] * outlyingSearchRatioO)),
	bodyThresholdZ(getBodyCentroid(0)[2] - (getBodyHalfExtent(0)[2] * 2.5)); // anything nearer than body front Z margin qualifies

// clamp to image edge bounds
const signed int
	bodySearchMinXClamped(std::max(bodySearchMinX, 0)),
	bodySearchMaxXClamped(std::min(bodySearchMaxX, (signed)foreZ.getWidth())),
	bodySearchMinYClamped(std::max(bodySearchMinY, 0)),
	bodySearchMaxYClamped(std::min(bodySearchMaxY, (signed)foreZ.getHeight())),
	bodyThresholdZClamped(std::max(bodyThresholdZ, 0));

// make an expendable copy of the foreground buffer
livescene::Image foreZtoDeplete(foreZ);

for(unsigned int handSearch = 0; handSearch < 2; handSearch++)
{
	unsigned int resultX(0), resultY(0), resultZ(0);
	// search body region for nearest remaining Z point closer than body front
	if(findMinimumZLocation(foreZtoDeplete, bodySearchMinXClamped, bodySearchMinYClamped,
		bodySearchMaxXClamped, bodySearchMaxYClamped, bodyThresholdZClamped, resultX, resultY, resultZ))
	{
		// ensure there's at least a little bit of range between resultZ and bodyThresholdZClamped
		// avoids divide-by-zero and poor weighting later
		if((signed)resultZ < bodyThresholdZClamped - 2)
		{
			static int count;
			if(sampleAndDepleteAdjacentThresholded(foreZtoDeplete, bodyThresholdZClamped,
				resultX, resultY, resultZ,
				_handCentroid[handSearch][0], _handCentroid[handSearch][1], _handCentroid[handSearch][2]))
			{
				++HandsDetected;
			} // if
		} // if
	} // if
} // for

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

bool BodyMass::sampleAndDepleteAdjacentThresholded(livescene::Image &foreZtoDeplete, const signed int &thresholdZ, 
									const unsigned int &X, const unsigned int &Y, const short &Z,
									float &weightedX, float &weightedY, float &weightedZ)
{

	bool result(false);
	CoordStack searchStack;
	float runningX(0.0f), runningY(0.0f), runningZ(0.0f), runningWeight(0.0f);
	short minDistance(std::numeric_limits<short>::max());
	unsigned int width(foreZtoDeplete.getWidth()), height(foreZtoDeplete.getHeight());
	short *depthBuffer = (short *)foreZtoDeplete.getData();

	// seed the sampling/depletion recursive search
	addToCoordStack(searchStack, X, Y, Z);
	// obliterate this cell so it won't be re-processed or re-added to the stack in the future
	depthBuffer[Y * foreZtoDeplete.getWidth() + X] = foreZtoDeplete.getNull();


	while(!searchStack.empty())
	{
		coordTriplet currentElement = searchStack.entries[searchStack.size() - 1]; // get next element for processing
		searchStack.pop_back(); // remove it from stack before processing it
		// process this cell and add any neighbors that need processing
		sampleAndDepleteOneCell(foreZtoDeplete, searchStack,
			currentElement.X, currentElement.Y, currentElement.Z, 
			thresholdZ, Z,
			runningX, runningY, runningZ, runningWeight);
	} // while

	// normalize by runningWeight
	if(runningWeight > 0.0f)
	{
		float runningWeightInv = (1.0f / runningWeight);
		// multiply by inverse is faster
		weightedX = runningX * runningWeightInv;
		weightedY = runningY * runningWeightInv;
		weightedZ = runningZ * runningWeightInv;
		result = true; // success
	} // if

	return(result);
} // BodyMass::sampleAndDepleteAdjacentThresholded

void BodyMass::sampleAndDepleteOneCell(livescene::Image &foreZtoDeplete, CoordStack &searchStack,
									   const unsigned int &X, const unsigned int &Y, const short &Z, 
									   const signed int &thresholdZ, const signed int &maxZ,
									   float &runningX, float &runningY, float &runningZ, float &runningWeight)
{
	short *depthBuffer = (short *)foreZtoDeplete.getData();

	// calculate weight of this sample
	// samples nearer to the maxZ (the most-extended part of the limb) get more
	// weight, samples near the threshold plane get much less.
	// weight ranges from 0...1, and is squared
	float weight = (1.0f - (float)(Z - maxZ) / (float)(thresholdZ - maxZ));
	weight *= weight; // give it squared power, not linear
	// add the sample to the accumulator, scaled by its weight
	runningX += (X * weight);
	runningY += (Y * weight);
	runningZ += (Z * weight);
	// add the weight to the running total for later normalization
	runningWeight += weight;

	// now, flag any valid neighbors for processing
	const int spreadMargin(10);

	for(int xNeighbor = -spreadMargin; xNeighbor <= spreadMargin; xNeighbor++)
	{
		for(int yNeighbor = -spreadMargin; yNeighbor <= spreadMargin; yNeighbor++)
		{
			const int currentNeighborX(X + xNeighbor), currentNeighborY(Y + yNeighbor);
			if(currentNeighborX > 0 && currentNeighborX < (signed)foreZtoDeplete.getWidth()
				&& currentNeighborY > 0 && currentNeighborY < (signed)foreZtoDeplete.getHeight())
			{
				short neighborDepth = depthBuffer[currentNeighborY * foreZtoDeplete.getWidth() + currentNeighborX];
				if(neighborDepth < thresholdZ && foreZtoDeplete.isCellValueValid(neighborDepth))
				{
					addToCoordStack(searchStack, currentNeighborX, currentNeighborY, neighborDepth);
					// obliterate this cell so it won't be re-processed or re-added to the stack in the future
					depthBuffer[currentNeighborY * foreZtoDeplete.getWidth() + currentNeighborX] = foreZtoDeplete.getNull();

				} // if
			} // if
		} // for
	} // for


} // BodyMass::sampleAndDepleteOneCell


void BodyMass::addToCoordStack(CoordStack &stack, const unsigned int &X, const unsigned int &Y, const short &Z)
{

	if(stack.size() < OSG_LIVESCENEVIEW_DETECT_MAX_STACK)
	{
		stack.push_back(coordTriplet(X, Y, Z));
	} // if
} // BodyMass::addToCoordStack


// namespace livescene
}
