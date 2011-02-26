// Copyright 2011 Skew Matrix Software and AlphaPixel

#include "liblivescene/Image.h"
#include <stdlib.h> // malloc/free
#include <malloc.h> // malloc/free
#include <memory.h> // memcpy
#include <limits> // max()

namespace livescene {


void ImageStatistics::addSample(double sample)
{
	_samples ++;
	if(sample < _min) _min = sample;
	if(sample > _max) _max = sample;

	// stddev refer to http://www.johndcook.com/standard_deviation.html
    // See Knuth TAOCP vol 2, 3rd edition, page 232
    if (_samples == 1)
    {
        m_oldM = _mean = sample;
        m_oldS = 0.0;
    }
    else
    {
        _mean = m_oldM + (sample - m_oldM) / _samples;
        m_newS = m_oldS + (sample - m_oldM) * (sample - _mean);

        // set up for next iteration
        m_oldM = _mean; 
        m_oldS = m_newS;
    }
} // ImageStatistics::addSample

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


unsigned int Image::patchNulls(const unsigned int &numPasses)
{
	unsigned int numPatched(0);
	if(!(_format == DEPTH_10BIT || _format == DEPTH_11BIT))
	{
		return(false);
	} // if

	/*

	int width(getWidth()), height(getHeight());
	short *depthBuffer = (short *)getData();

	// we intentionally skip scanning the outermost two lines/columns because
	// our in-fill algorigthm can't operate out there properly. Actually, it can work
	// on the top and left edge but there's no reason to do those edges if we're not doing
	// the bottom and right. Skipping them eliminates repetitive error-checking in the loop.
	unsigned int startLine(2), endLine(height - 2), // initial loop range
		firstLine(height), lastLine(0); // extrema for next loop, initialized to inverse
	unsigned int startCol(2), endCol(width - 2), // initial loop range
		firstCol(width), lastCol(0); // extrema for next loop, initialized to inverse

	// iterate passes
	for(unsigned int pass(0); pass < numPasses; ++pass)
	{
		// reinitialize extrema-checking
		firstLine = height; lastLine = 0;
		fisrtCol = width; lastCol = 0;

		unsigned int lineSub(0);
		// vertically scan restricted box, which is initially entire raster
		for(int line = startLine; line < endLine; ++line)
		{
			lineSub = line * width;
			// horizontally scan restricted box, which is initially entire raster
			for(int column = startCol; column < endCol; ++column)
			{
				// valid samples are assumed to be in the minority, so trigger on
				// them, rather than on invalid samples
				if(isCellValueValid(depthBuffer[lineSub + column]))
				{
					// does it have any invalid cells below/right of it that we can try to
					// expand into?

					// right
					if(!isCellValueValid(depthBuffer[lineSub + column + 1]))
					{
						// is the cell past it valid, to allow a spanning?
						if(isCellValueValid(depthBuffer[lineSub + column + 2]))
						{
						} // if
					} // if
					// down
					if(!isCellValueValid(depthBuffer[lineSub + width + column]))
					{
						// is the cell past it valid, to allow a spanning?
						if(isCellValueValid(depthBuffer[lineSub + width + width + column]))
						{
						} // if
					} // if
					// down-right
					if(!isCellValueValid(depthBuffer[lineSub + width + column + 1]))
					{
						// is the cell past it valid, to allow a spanning?
						if(isCellValueValid(depthBuffer[lineSub + width + width + column + 2]))
						{
						} // if
					} // if
				} // if
			} // for
		} // for lines
	} // for passes
	*/

	return(numPatched);
} // Image::patchNulls


unsigned int Image::filterNoise(const unsigned int &numNeighbors)
{
	unsigned int numFiltered(0);
	if(!(_format == DEPTH_10BIT || _format == DEPTH_11BIT))
	{
		return(false);
	} // if

	int width(getWidth()), height(getHeight());
	short *depthBuffer = (short *)getData();

	unsigned int startLine(1), endLine(height); // initial loop range
	unsigned int startCol(1), endCol(width); // initial loop range

	unsigned int lineSub(0);
	// vertically scan 
	for(unsigned int line = startLine; line < endLine; ++line)
	{
		lineSub = line * width;
		// horizontally scan
		for(unsigned int column = startCol; column < endCol; ++column)
		{
			// valid samples are assumed to be in the minority, so trigger on
			// them, rather than on invalid samples
			if(isCellValueValid(depthBuffer[lineSub + column]))
			{
				// does it have fewer valid neighbors than the threshold?
				if(countValidNeighbors(column, line) < numNeighbors)
				{
					depthBuffer[lineSub + column] = _nullValue; // null it out
					++numFiltered;
				} // if
			} // if
		} // for
	} // for lines

	return(numFiltered);
} // Image::filterNoise

unsigned int Image::countValidNeighbors(const unsigned int &X, const unsigned int &Y) const
{
	unsigned int validNeighbors(0);
	unsigned int width = getWidth();
	short *depthBuffer = (short *)getData();

	if(Y > 0)
	{
		// UL
		if(X > 0 && isCellValueValid(depthBuffer[(Y - 1) * width + (X - 1)])) ++validNeighbors;
		// UC
		if(isCellValueValid(depthBuffer[(Y - 1) * width + (X)])) ++validNeighbors;
		// UR
		if(X < width && isCellValueValid(depthBuffer[(Y - 1) * width + (X + 1)])) ++validNeighbors;
	} // if

	// ML
	if(X > 0 && isCellValueValid(depthBuffer[(Y) * width + (X - 1)])) ++validNeighbors;
	// MR
	if(X < width && isCellValueValid(depthBuffer[(Y) * width + (X + 1)])) ++validNeighbors;

	if(Y < getHeight())
	{
		// LL
		if(X > 0 && isCellValueValid(depthBuffer[(Y + 1) * width + (X - 1)])) ++validNeighbors;
		// LC
		if(isCellValueValid(depthBuffer[(Y + 1) * width + (X)])) ++validNeighbors;
		// LR
		if(X < width && isCellValueValid(depthBuffer[(Y + 1) * width + (X + 1)])) ++validNeighbors;
	} // if

	return(validNeighbors);
} // Image::countValidNeighbors


bool Image::minimumDeltaToNeighbors(const unsigned int &X, const unsigned int &Y, short cellValue, long &result) const
{
	unsigned int validNeighbors(0);
	unsigned int width = getWidth();
	short *depthBuffer = (short *)getData();
	short depthValue, absDelta(std::numeric_limits<short>::max());
	result = std::numeric_limits<short>::max();

	// if each neighbor is valid, calculate the delta between it and the current cell, and look for the minimum delta of all of them

	if(Y > 0)
	{
		// UL
		if(X > 0 && isCellValueValid(depthValue = depthBuffer[(Y - 1) * width + (X - 1)])) {++validNeighbors; absDelta = abs(depthValue - cellValue); if(absDelta < result) result = absDelta; }
		// UC
		if(isCellValueValid(depthValue = depthBuffer[(Y - 1) * width + (X)])) {++validNeighbors; absDelta = abs(depthValue - cellValue); if(absDelta < result) result = absDelta; }
		// UR
		if(X < width && isCellValueValid(depthValue = depthBuffer[(Y - 1) * width + (X + 1)])) {++validNeighbors; absDelta = abs(depthValue - cellValue); if(absDelta < result) result = absDelta; }
	} // if
	// ML
	if(X > 0 && isCellValueValid(depthValue = depthBuffer[(Y) * width + (X - 1)])) {++validNeighbors; absDelta = abs(depthValue - cellValue); if(absDelta < result) result = absDelta; }
	// MR
	if(X < width && isCellValueValid(depthValue = depthBuffer[(Y) * width + (X + 1)])) {++validNeighbors; absDelta = abs(depthValue - cellValue); if(absDelta < result) result = absDelta; }
	if(Y < getHeight())
	{
		// LL
		if(X > 0 && isCellValueValid(depthValue = depthBuffer[(Y + 1) * width + (X - 1)])) {++validNeighbors; absDelta = abs(depthValue - cellValue); if(absDelta < result) result = absDelta; }
		// LC
		if(isCellValueValid(depthValue = depthBuffer[(Y + 1) * width + (X)])) {++validNeighbors; absDelta = abs(depthValue - cellValue); if(absDelta < result) result = absDelta; }
		// LR
		if(X < width && isCellValueValid(depthValue = depthBuffer[(Y + 1) * width + (X + 1)])) {++validNeighbors; absDelta = abs(depthValue - cellValue); if(absDelta < result) result = absDelta; }
	} // if

	if(validNeighbors > 0)
	{
		result = absDelta;
		return(true);
	} // if
	else
	{
		return(false);
	} // else
} // 



bool Image::calcStatsXYZ(livescene::ImageStatistics *destStatsX, livescene::ImageStatistics *destStatsY, livescene::ImageStatistics *destStatsZ, ApproveCallback *approveCallback)
{
	if(!(_format == DEPTH_10BIT || _format == DEPTH_11BIT))
	{
		return(false);
	} // if

	if(destStatsX) destStatsX->clear();
	if(destStatsY) destStatsY->clear();
	if(destStatsZ) destStatsZ->clear();

	int width(getWidth()), height(getHeight());
	short *depthBuffer = (short *)getData();

	unsigned int loopSub(0), vertSub(0), indexSub(0), texSub(0), normSub(0);
	for(int line = 0; line < height; ++line)
	{
		for(int column = 0; column < width; ++column)
		{
			short originalDepth = depthBuffer[loopSub];
			if(isCellValueValid(originalDepth) && (!approveCallback || (*approveCallback)(column, line, originalDepth)))
			{
				if(destStatsX) destStatsX->addSample(column);
				if(destStatsY) destStatsY->addSample(line);
				if(destStatsZ) destStatsZ->addSample(originalDepth);
			} // if
			++loopSub;
		} // for
	} // for lines

return(true);
} // Image::calcStatsZ



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
