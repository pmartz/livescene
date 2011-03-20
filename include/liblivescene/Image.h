// Copyright 2011 Skew Matrix Software and AlphaPixel

#ifndef __LIVESCENE_IMAGE_H__
#define __LIVESCENE_IMAGE_H__ 1

#include "liblivescene/Export.h"
#include <cmath> // sqrt


namespace livescene {


/** \defgroup Image Image Operations */
/*@{*/

class LIVESCENE_EXPORT ImageStatistics
{
public:
	ImageStatistics() {clear();}

	void addSample(double sample);
	double getMidVal(void) const {return((_min + _max) * 0.5);}
	double getMean(void) const {return(_mean);}
	double getMin(void) const {return(_min);}
	double getMax(void) const {return(_max);}
	double getStdDev(void) const {return(sqrt(getVariance()));}
	double getVariance(void) const {return ( (_samples > 1) ? m_newS / (_samples - 1) : 0.0 );}
	unsigned long int getNumSamples(void) const {return(_samples);}
	void clear(void) {_mean = 0.0; _min = 0.0; _max = 0.0; m_newS = 0.0; m_oldM = 0.0; m_oldS = 0.0; _samples = 0;}

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
	DEPTH_11BIT           = 7, /**< 11 bit depth information in one uint16_t/pixel */
	DEPTH_10BIT           = 8, /**< 10 bit depth information in one uint16_t/pixel */
	DEPTH_11BIT_PACKED    = 9, /**< 11 bit packed depth information */
	DEPTH_10BIT_PACKED    = 10, /**< 10 bit packed depth information */
	DEPTH_FLOAT_32_BIT    = 11, /**< single precision floats, only used for testing */
} VideoFormat;



/** \brief Approve/reject callback functor base class.

*/


class LIVESCENE_EXPORT ApproveCallback
{
	public:
		virtual ~ApproveCallback() {}
		virtual bool operator ()(const unsigned int &xCoord, const unsigned int &yCoord, const unsigned short &zCoord) = 0; // must be overridden
}; // ApproveCallback


/** \brief Image core object.

*/


class LIVESCENE_EXPORT Image
{
	public:

		/**  */
		Image(const Image &image, bool cloneData = false); // copy constructor can make a local, persistent copy of the data
		Image(int width = 640, int height = 480, int depth = 1, VideoFormat format = VIDEO_RGB)
			: _width(width), _height(height), _depth(depth), _format(format), _timestamp(0), _nullValue(0), _data(0), _accumulation(1), _dataSelfAllocated(false),
			_xStatValid(false), _yStatValid(false), _zStatValid(false)
		{}
		~Image();

		Image & operator= (const Image & rhs);

		void setData(void *data) {_data = data;}
		void *getData(void) const {return(_data);}

		unsigned int getWidth(void) const {return(_width);}
		unsigned int getHeight(void) const {return(_height);}
		/** Depth is the number of bytes per pixel, not actual number of bits utilized per pixel */
		unsigned int getDepth(void) const {return(_depth);}
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
		inline bool isCellValueValid(const short &value) const {return(value != _nullValue);}

		// eliminates zero valued samples by replacing them with the Null value
		// This allows for simpler NULL testing later as we only test against one value
		// only operates on Z buffer
		void rewriteZeroToNull(void);

		// eliminates NULL valued samples by interpolating from adjacent non-null data.
		// multiple passes may be needed, but take more time
		// only operates on Z buffer
		// return value indicates how many null values were patched
		unsigned int patchNulls(const unsigned int &numPasses);

		// filter out samples that have ferwer than numNeighbors non-null samples adjacent to them.
		// obviously numNeighbors makes little sense greater than 8, and probably little sense
		// when greater than 2 or even 3
		// only operates on Z buffer
		// return value indicates how many spurious samples were filtered
		unsigned int filterNoise(const unsigned int &numNeighbors);

		// counts number of non-NULL neighbors this cell has
		unsigned int countValidNeighbors(const unsigned int &X, const unsigned int &Y) const;

		// calculates minimum Z distance between a sample and its non-NULL neighbors
		// stores distance in result
		// returns true if successful, false if not
		bool minimumDeltaToNeighbors(const unsigned int &X, const unsigned int &Y, short cellValue, long &result) const;

		// calculate useful statistics, only operates on Z data, not RGB
		// approveCallback allows you to provide a callback functor that can custom approve/reject samples
		bool calcStatsXYZ(livescene::ImageStatistics *destStatsX, livescene::ImageStatistics *destStatsY, livescene::ImageStatistics *destStatsZ, ApproveCallback *approveCallback = 0);
		bool calcStatsXYZBounded(const unsigned int &Xlow, const unsigned int &Ylow, const unsigned int &Xhigh, const unsigned int &Yhigh, livescene::ImageStatistics *destStatsX, livescene::ImageStatistics *destStatsY, livescene::ImageStatistics *destStatsZ, ApproveCallback *approveCallback = 0);

		// this calculates full-frame XYZ stats and stores them in the cached internal stats object to avoid unnecessary recalcs
		bool calcInternalStatsXYZ(ApproveCallback *approveCallback = 0);
		// mark internal stats as invalid
		void invalidateInternalStats(void) {_xStatValid = _yStatValid = _zStatValid = false; _xStat.clear(); _yStat.clear(); _zStat.clear(); }
		const livescene::ImageStatistics &getInternalStatsX(void) const {return(_xStat);}
		const livescene::ImageStatistics &getInternalStatsY(void) const {return(_yStat);}
		const livescene::ImageStatistics &getInternalStatsZ(void) const {return(_zStat);}

		// this will ensure the image is allocated to the proper size and ready to write to
		bool preAllocate(void);

	private:
		void freeData(void);
		void allocData(void);

		unsigned int _width, _height, _depth;
		int _nullValue;
		unsigned short _accumulation;
		VideoFormat _format;
        unsigned long _timestamp;
		void *_data; // this is not resource tracked or freed, it's just a dumb pointer for transport
		bool _dataSelfAllocated;

		// image statistics
		livescene::ImageStatistics _xStat, _yStat, _zStat;
		bool _xStatValid, _yStatValid, _zStatValid;

}; // Image



/*@}*/


// namespace livescene
}

// __LIVESCENE_IMAGE_H__
#endif
