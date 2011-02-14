// Copyright 2011 Skew Matrix Software and AlphaPixel

#ifndef __LIVESCENE_IMAGE_H__
#define __LIVESCENE_IMAGE_H__ 1

#include "liblivescene/Export.h"
#include <cmath> // sqrt


namespace livescene {


/** \defgroup Image Image Operations */
/*@{*/

class ImageStatistics
{
public:
	ImageStatistics() : _mean(0.0), _min(0.0), _max(0.0), m_newS(0.0), m_oldM(0.0), m_oldS(0.0), _samples(0) {};

	void addSample(double sample);
	double getMedian(void) const {return((_min + _max) * 0.5);}
	double getMean(void) const {return(_mean);}
	double getMin(void) const {return(_min);}
	double getMax(void) const {return(_max);}
	double getStdDev(void) const {return(sqrt(getVariance()));}
	double getVariance(void) const {return ( (_samples > 1) ? m_newS / (_samples - 1) : 0.0 );}
	unsigned long int getNumSamples(void) const {return(_samples);}

private:

	double _mean, _min, _max;
	double m_newS, m_oldM, m_oldS;
	unsigned long int _samples;

}; // ImageStatistics

typedef enum {
	VIDEO_RGB             = 0, /**< Decompressed RGB mode (demosaicing already done) */
	VIDEO_BAYER_RG_GB     = 1, /**< Bayer compressed mode (raw information from camera) */
	VIDEO_IR_8BIT         = 2, /**< 8-bit IR mode  */
	VIDEO_IR_10BIT        = 3, /**< 10-bit IR mode */
	VIDEO_IR_10BIT_PACKED = 4, /**< 10-bit packed IR mode */
	VIDEO_YUV_RGB         = 5, /**< YUV RGB mode */
	VIDEO_YUV_RAW         = 6, /**< YUV Raw mode */
	DEPTH_11BIT           = 0, /**< 11 bit depth information in one uint16_t/pixel */
	DEPTH_10BIT           = 1, /**< 10 bit depth information in one uint16_t/pixel */
	DEPTH_11BIT_PACKED    = 2, /**< 11 bit packed depth information */
	DEPTH_10BIT_PACKED    = 3, /**< 10 bit packed depth information */
} VideoFormat;


/** \brief Image core object.

*/


class LIVESCENE_EXPORT Image
{
	public:

		/**  */
		Image(const Image &image, bool cloneData = false); // copy constructor can make a local, persistent copy of the data
		Image(int width = 640, int height = 480, int depth = 1, VideoFormat format = VIDEO_RGB)
			: _width(width), _height(height), _depth(depth), _format(format), _timestamp(0), _nullValue(0), _data(0), _accumulation(1), _dataSelfAllocated(false)
		{}
		~Image();

		Image & operator= (const Image & rhs);

		void setData(void *data) {_data = data;}
		void *getData(void) const {return(_data);}

		int getWidth(void) const {return(_width);}
		int getHeight(void) const {return(_height);}
		/** Depth is the number of bytes per pixel, not actual number of bits utilized per pixel */
		int getDepth(void) const {return(_depth);}
		VideoFormat getFormat(void) const {return(_format);}
		int getSamples(void) const {return(getWidth() * getHeight());}
		int getImageBytes(void) const {return(getWidth() * getHeight() * getDepth());}
		unsigned long getTimestamp(void) const {return(_timestamp);}
		void setTimestamp(const unsigned long Timestamp) {_timestamp = Timestamp;}

		// accumulation is used in averaging multiple frames together (limited by precision)
		int getAccumulation(void) const {return(_accumulation);}
		void increaseAccumulation(void) {++_accumulation;}
		void clearAccumulation(void) {_accumulation = 1;} // no sense to have zero data recorded

		int getNull(void) const {return(_nullValue);}
		void setNull(int newNull) {_nullValue = newNull;}
		inline bool isCellValueValid(const short &value) const {return(value != _nullValue && value != 0);}

		// calculate useful statistics, only operates on Z data, not RGB
		bool calcStatsXYZ(livescene::ImageStatistics *destStatsX, livescene::ImageStatistics *destStatsY, livescene::ImageStatistics *destStatsZ);

		// this will ensure the image is allocated to the proper size and ready to write to
		bool preAllocate(void);

	private:
		void freeData(void);
		void allocData(void);

		int _width, _height, _depth, _nullValue, _accumulation;
		VideoFormat _format;
        unsigned long _timestamp;
		void *_data; // this is not resource tracked or freed, it's just a dumb pointer for transport
		bool _dataSelfAllocated;

}; // Image



/*@}*/


// namespace livescene
}

// __LIVESCENE_IMAGE_H__
#endif
