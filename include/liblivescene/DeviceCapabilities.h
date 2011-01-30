// Copyright 2011 Skew Matrix Software and AlphaPixel

#ifndef __LIVESCENE_DEVICECAPABILITIES_H__
#define __LIVESCENE_DEVICECAPABILITIES_H__ 1

#include "liblivescene/Export.h"
#include "liblivescene/Image.h"
#include <string>


namespace livescene {


/** \defgroup Device Device Management */
/*@{*/

/** \brief interfaces for different capabilities a device might expose.

*/


class LIVESCENE_EXPORT DeviceCapabilitiesImage
{
	public:

		/** Synchronously gets an image using the specified format and fills in width, height, depth and data.
		Returns true if successful. */
		virtual bool getImageSync(livescene::Image &image) = 0;

		/** Asynchronously sets an image using the specified format and fills in width, height, depth and data.
		<<<>>> Notification of completion is performed with the callback. */
		//virtual getImageAsync(livescene::Image &image, <<<>>> callback) = 0;

		/** Gets image attributes using the specified format and fills in width, height, depth. Returns true if successful. */
		virtual bool getCurrentImageInfo(livescene::Image &image) = 0;

		/** Gets current width for specified format */
		virtual int getCurrentImageWidth(const VideoFormat format) = 0;

		/** Gets current height for specified format */
		virtual int getCurrentImageHeight(const VideoFormat format) = 0;

		/** Gets current depth for specified format.
		Depth is the number of bytes per pixel, not actual number of bits utilized per pixel. */
		virtual int getCurrentImageDepth(const VideoFormat format) = 0;

		/** Sets image attributes using the specified format using width, height, depth.
		Returns true if successful. */
		virtual bool setCurrentImageInfo(const livescene::Image &image) = 0;

		/** Sets current width for specified format. Returns true if successful. */
		virtual bool setCurrentImageWidth(VideoFormat format) = 0;

		/** Sets current height for specified format. Returns true if successful. */
		virtual bool setCurrentImageHeight(VideoFormat format) = 0;

		/** Sets current depth for specified format. Returns true if successful. */
		virtual bool setCurrentImageDepth(VideoFormat format) = 0;


}; // DeviceCapabilitiesImage


class LIVESCENE_EXPORT DeviceCapabilitiesMount
{
	public:

		/**  */
		//virtual (void);


}; // DeviceCapabilitiesMount


class LIVESCENE_EXPORT DeviceCapabilitiesAudio
{
	public:

		/**  */
		//virtual (void);


}; // DeviceCapabilitiesAudio


class LIVESCENE_EXPORT DeviceCapabilitiesAccelerometer
{
	public:

		/**  */
		//virtual (void);


}; // DeviceCapabilitiesDeviceCapabilitiesAccelerometer


class LIVESCENE_EXPORT DeviceCapabilitiesLED
{
	public:

		/**  */
		//virtual (void);


}; // DeviceCapabilitiesLED

/*@}*/


// namespace livescene
}

// __LIVESCENE_DEVICECAPABILITIES_H__
#endif
