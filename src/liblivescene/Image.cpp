// Copyright 2011 Skew Matrix Software and AlphaPixel

#include "liblivescene/Image.h"
#include <stdlib.h> // malloc/free
#include <malloc.h> // malloc/free
#include <memory.h> // memcpy

namespace livescene {


Image::Image(const Image &image, bool cloneData)
: _data(0), _dataSelfAllocated(false), _nullValue(0)
{
	_width = image._width;
	_height = image._height;
	_depth = image._depth;
	_format = image._format;
	_timestamp = image._timestamp;
	_nullValue = image._nullValue;
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



Image & Image::operator= (const Image & rhs)
{ // be careful of resources during assignment
	_width = rhs._width;
	_height = rhs._height;
	_depth = rhs._depth;
	_format = rhs._format;
	_timestamp = rhs._timestamp;
	_nullValue = rhs._nullValue;
	if(rhs._dataSelfAllocated) // rhs owns its data, make our own copy of it
	{
		allocData(); // create a new image buffer
		if(_data)
		{
			memcpy(_data, rhs._data, getImageBytes()); 
		} // if
	} // if
	else // rhs doesn't own it, we don't need to own it or make our own copy
	{
		_data = rhs._data;
	} // else

return *this;
} // Image::operator=

bool Image::preAllocate(void)
{
	if(_data)
	{
		if(_dataSelfAllocated)
		{
			return(true); // good to go
		} // if
		_data = 0; // zero out pointer to non-owned buffer
	} // if
	allocData();
	if(_data) return(true); // success
	return(false); // failed
} // Image::preAllocate

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
			_data = 0;
			_dataSelfAllocated = false;
		} // if
	} // if
} // // Image::freeData


// namespace livescene
}
