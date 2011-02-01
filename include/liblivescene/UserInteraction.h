// Copyright 2011 Skew Matrix Software and AlphaPixel

#ifndef __LIVESCENE_USER_INTERACTION_H__
#define __LIVESCENE_USER_INTERACTION_H__ 1

#include "liblivescene/Export.h"
#include "liblivescene/Image.h"
#include <string>


namespace livescene {


/**
*/
class LIVESCENE_EXPORT UserInteraction
{
public:
    UserInteraction();
    ~UserInteraction();

    void detection( const livescene::Image& imageRGB, const livescene::Image& imageZ );

protected:
};


// livescene
}


// __LIVESCENE_USER_INTERACTION_H__
#endif
