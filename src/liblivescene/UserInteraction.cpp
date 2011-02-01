// Copyright 2011 Skew Matrix Software and AlphaPixel

#include "liblivescene/UserInteraction.h"
#include "liblivescene/Image.h"

#include <osg/Vec2s>
#include <osg/io_utils>

#include <iostream>


namespace livescene {


UserInteraction::UserInteraction()
{
}
UserInteraction::~UserInteraction()
{
}

void UserInteraction::detection( const livescene::Image& imageRGB, const livescene::Image& imageZ )
{
    unsigned short* ptr = ( unsigned short* )( imageZ.getData() );
    int width = imageZ.getWidth();
    int height = imageZ.getHeight();

    unsigned short minVal( 0xffff );
    osg::Vec2s minLoc;
    int sdx, tdx;
    for( tdx=0; tdx<height; tdx++ )
    {
        for( sdx=0; sdx<width; sdx++ )
        {
            if( *ptr < minVal )
            {
                minVal = *ptr;
                minLoc.set( sdx, tdx );
            }
            ptr++;
        }
    }
    std::cout << "Min: " << minVal << " at " << minLoc << std::endl;
}


// livescene
}