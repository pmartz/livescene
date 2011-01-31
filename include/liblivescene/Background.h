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
		Background() : _backgroundAvailable(false), _discriminationEpsilon(.03f) {};
		~Background() {};

		bool getBackgroundAvailable(void) const {return(_backgroundAvailable);}

		void setDiscriminationEpsilon(const float discriminationEpsilon) {_discriminationEpsilon = discriminationEpsilon;}
		float getDiscriminationEpsilon(void) const {return(_discriminationEpsilon);}

		bool loadBackgroundFromCleanPlate(const livescene::Image &cleanPlateRGB, const livescene::Image &cleanPlateZ);
		bool loadRGBBackgroundFromCleanPlate(const livescene::Image &cleanPlateRGB);
		bool loadZBackgroundFromCleanPlate(const livescene::Image &cleanPlateZ);

		// <<<>>> not yet implemented, accumulate non-changing pixels from live stream into background
		bool accumulateBackgroundFromLive(const livescene::Image &liveRGB, const livescene::Image &liveZ);
		bool accumulateRGBBackgroundFromLive(const livescene::Image &liveRGB);
		bool accumulateZBackgroundFromLive(const livescene::Image &liveZ);

		// extracts the foreground from the background plate.
		// foregroundZ must be prepped with Image::preAllocate
		bool extractZBackground(const livescene::Image &liveZ, livescene::Image &foregroundZ);

	private:
		livescene::Image _bgRGB, _bgZ;
		bool _backgroundAvailable;
		float _discriminationEpsilon;

}; // Background



/*@}*/


// namespace livescene
}

// __LIVESCENE_BACKGROUND_H__
#endif
