// Copyright 2011 Skew Matrix Software and AlphaPixel

#ifndef __LIVESCENE_DETECT_H__
#define __LIVESCENE_DETECT_H__ 1

#include "liblivescene/Export.h"
#include "liblivescene/Image.h"

#include "osg/BoundingBox"


namespace livescene {

const int OSG_LIVESCENEVIEW_DETECT_BODY_SAMPLES(5000); // fewer than this many foreground samples in a frame are presumed to be less than a single body

/** \defgroup Detect Object Detection
*/

/** \brief Detect body mass(es)
* Currently only supports a single body mass, but API supports multiple
* in the future.
*/

class LIVESCENE_EXPORT BodyMass
{
public:
	BodyMass() {clear();}

	void clear(void) {_bodyPresent = false; _centroid[0] = _centroid[1] = _centroid[2] = 0.0f; _stdDev[0] = _stdDev[1] = _stdDev[2] = 0.0f;}
	const bool getBodyPresent(void) const {return(_bodyPresent);}
	const float *getCentroid(unsigned int bodyNum) const {return(_centroid);} // bodyNum is currently ignored
	const float *getExtent(unsigned int bodyNum) const {return(_stdDev);} // bodyNum is currently ignored

	unsigned int detect(livescene::Image &foreZ); // can only detect 0 or 1 bodies currently

private:
	bool _bodyPresent;
	float _centroid[3], // x,y,z
		_stdDev[3];  // x,y,z

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
