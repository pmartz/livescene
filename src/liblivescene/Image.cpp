// Copyright 2011 Skew Matrix Software and AlphaPixel

#include "liblivescene/Image.h"
#include <string>
#include <algorithm>

#include <memory.h> // malloc/free

namespace livescene {


Image::Image(const Image &image, bool cloneData)
: _data(NULL), _dataSelfAllocated(false)
{
	_width = image._width;
	_height = image._height;
	_depth = image._depth;
	_format = image._format;
	_timestamp = image._timestamp;
	if(cloneData)
	{
		allocData(); // create a new image buffer
		if(_data)
		{
			memcpy(_data, image._data, getImageBytes()); 
		} // if
	} // if
	else
	{
		_data = image._data;
	} // else
} // Image::Image copy constructor


Image::~Image()
{

	freeData();

} // Image::~Image

void Image::allocData(void)
{
	freeData();
	_data = malloc(getImageBytes());
	if(_data)
	{
		_dataSelfAllocated = true;
	} // if
} // // Image::allocData

void Image::freeData(void)
{
	if(_dataSelfAllocated)
	{
		if(_data)
		{
			free(_data);
			_data = NULL;
			_dataSelfAllocated = false;
		} // if
	} // if
} // // Image::freeData


// namespace livescene
}
