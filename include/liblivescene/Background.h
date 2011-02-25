// Copyright 2011 Skew Matrix Software and AlphaPixel

#ifndef __LIVESCENE_BACKGROUND_H__
#define __LIVESCENE_BACKGROUND_H__ 1

#include "liblivescene/Export.h"
#include "liblivescene/Image.h"
#include <string>


namespace livescene {


/** \defgroup Background Background Isolation */
/*@{*/

/** \brief Background processor.

*/


class LIVESCENE_EXPORT Background
{
	public:
		Background() : _backgroundAvailable(false), _discriminationEpsilonPercent(.01f) {};
		~Background() {};

		bool getBackgroundAvailable(void) const {return(_backgroundAvailable);}

		// the discrimination epsilon is the amount of error tolerated when comparing the candidate Z value
		// against the stored background Z value. It is expressed as a percent of the Z value of the
		// sample being compared, since Z precision is presumed to degrade with distance.
		void setDiscriminationEpsilonPercent(const float discriminationEpsilonPercent) {_discriminationEpsilonPercent = discriminationEpsilonPercent;}
		float getDiscriminationEpsilonPercent(void) const {return(_discriminationEpsilonPercent);}

		/** Comparison/accumulation mode to use when accumulating background
		*/
		enum AccumulateMode {
			MIN_Z,
			MAX_Z,
			AVERAGE_Z,
			MIN_Z_ADJACENT,
			MAX_Z_ADJACENT,
			AVERAGE_Z_ADJACENT,
		};

		bool loadBackgroundFromCleanPlate(const livescene::Image &cleanPlateRGB, const livescene::Image &cleanPlateZ);
		bool loadRGBBackgroundFromCleanPlate(const livescene::Image &cleanPlateRGB);
		bool loadZBackgroundFromCleanPlate(const livescene::Image &cleanPlateZ);
		bool accumulateBackgroundFromCleanPlate(const livescene::Image &cleanPlateRGB, const livescene::Image &cleanPlateZ, AccumulateMode mode, livescene::Image *foreZ = NULL);
		bool accumulateRGBBackgroundFromCleanPlate(const livescene::Image &cleanPlateRGB, AccumulateMode mode);
		bool accumulateZBackgroundFromCleanPlate(const livescene::Image &cleanPlateZ, AccumulateMode mode, livescene::Image *foreZ = NULL);

		// <<<>>> not yet implemented, accumulate non-changing pixels from live stream into background
		bool accumulateBackgroundFromLive(const livescene::Image &liveRGB, const livescene::Image &liveZ);
		bool accumulateRGBBackgroundFromLive(const livescene::Image &liveRGB);
		bool accumulateZBackgroundFromLive(const livescene::Image &liveZ);

		// extracts the foreground from the background plate.
		// foregroundZ must be prepped with Image::preAllocate
		bool extractZBackground(const livescene::Image &liveZ, livescene::Image &foregroundZ);

		// these can be used to display the background independently
		const livescene::Image &getBackgroundZ(void) const {return(_bgZ);}
		const livescene::Image &getBackgroundRGB(void) const {return(_bgRGB);}

	private:
		livescene::Image _bgRGB, _bgZ;
		bool _backgroundAvailable;
		float _discriminationEpsilonPercent;

}; // Background



/*@}*/


// namespace livescene
}

// __LIVESCENE_BACKGROUND_H__
#endif
