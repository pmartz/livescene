// Copyright 2011 Skew Matrix Software and AlphaPixel

#ifndef __LIVESCENE_DETECT_H__
#define __LIVESCENE_DETECT_H__ 1

#include <stack>
#include <utility>
#include "liblivescene/Export.h"
#include "liblivescene/Image.h"

#include "osg/BoundingBox"


namespace livescene {

const int OSG_LIVESCENEVIEW_DETECT_BODY_SAMPLES(13000); // fewer than this many foreground samples in a frame are presumed to be less than a single body

/** \defgroup Detect Object Detection
*/


/** \brief Detect hand(s)
* Currently supports two hands, but more can be supported if multiple bodies are supported.
* Not implemented right now, a simpler API exists as part of BodyMass object.
*/

class LIVESCENE_EXPORT Hand
{
public:
	Hand() {clear();}

	void clear(void) {_centroid[0] = _centroid[1] = _centroid[2] = 0.0f;}
	const float *getHandCentroid(void) const {return(_centroid);}

private:
	float _centroid[3]; // x,y,z
}; // Hand


/** \brief Pair of coordinates, used in CoordStack */
typedef std::pair<unsigned int, unsigned int> UIntPair;
/** \brief Stack of coordinate pairs, used internally by sampleAndDepleteAdjacentThresholded(). */
typedef std::stack<UIntPair> CoordStack;


/** \brief Detect body mass(es)
* Currently only supports a single body mass, but API supports multiple
* in the future.
*/

class LIVESCENE_EXPORT BodyMass
{
public:
	BodyMass() {clear();}

	void clear(void) {_bodyPresent = false; _centroid[0] = _centroid[1] = _centroid[2] = 0.0f; _stdDev[0] = _stdDev[1] = _stdDev[2] = 0.0f;
	_handCentroid[0][0] = _handCentroid[0][1] = _handCentroid[0][2] = 0.0f; _handCentroid[1][0] = _handCentroid[1][1] = _handCentroid[1][2] = 0.0f;}
	const bool getBodyPresent(void) const {return(_bodyPresent);}
	const float *getBodyCentroid(unsigned int bodyNum) const {return(_centroid);} // bodyNum is currently ignored
	const float *getBodyHalfExtent(unsigned int bodyNum) const {return(_stdDev);} // bodyNum is currently ignored
	const float *getHandCentroid(unsigned int bodyNum, unsigned int handNum) const; // bodyNum is currently ignored

	unsigned int detect(livescene::Image &foreZ); // can only detect 0 or 1 bodies currently. Doesn't modify image, but modifies its internal stats
	unsigned int detectHands(const livescene::Image &foreZ); // can only detect one or two hands on one body at this time.

private:
	// returns true if it finds a minimum Z (closer than zThreshold) within the defined area
	bool findMinimumZLocation(const livescene::Image &foreZ,
		const unsigned int &Xlow, const unsigned int &Ylow, const unsigned int &Xhigh, const unsigned int &Yhigh,
		const short &zThreshold, unsigned int &minZlocX, unsigned int &minZlocY, unsigned int &minZvalueZ);
	// returns true if successful. WILL modify foreZtoDeplete during this process.
	bool BodyMass::sampleAndDepleteAdjacentThresholded(livescene::Image &foreZtoDeplete, const signed int &thresholdZ, 
		const unsigned int &X, const unsigned int &Y, const short &Z,
		float &weightedX, float &weightedY, float &weightedZ);
	// called by sampleAndDepleteAdjacentThresholded to process one cell and its neighbors
	void BodyMass::sampleAndDepleteOneCell(livescene::Image &foreZtoDeplete, CoordStack &searchStack,
		const unsigned int &X, const unsigned int &Y,
		const signed int &thresholdZ, const signed int &maxZ,
		float &runningX, float &runningY, float &runningZ, float &runningWeight);
	void addToCoordStack(const livescene::Image &foreZtoDeplete, CoordStack &stack, const signed int &thresholdZ, const unsigned int &X, const unsigned int &Y);
	bool _bodyPresent;
	float _centroid[3], // x,y,z
		_stdDev[3];  // x,y,z
	float _handCentroid[2][3]; // Hand0:x,y,z Hand1:x,y,z

}; // BodyMass


/** \brief ApproveCallback used to discard samples outside the plausible body region.
<<<>>> This should be OSG-nonspecific code, so I'd like to eliminate the usage of osg::BoundingBox here.
*/

class BoxApproveCallback : public livescene::ApproveCallback
{
public:
	BoxApproveCallback(osg::BoundingBox bbox) : _bbox(bbox) {}
	bool operator ()(const unsigned int &xCoord, const unsigned int &yCoord, const unsigned short &zCoord)
	{
		return(_bbox.contains(osg::Vec3f(xCoord, yCoord, zCoord)));
	} // operator ()
private:
	osg::BoundingBox _bbox;
}; // BoxApproveCallback 



/*@}*/


// namespace livescene
}

// __LIVESCENE_DETECT_H__
#endif
