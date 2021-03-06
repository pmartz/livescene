// Copyright 2011 Skew Matrix Software and AlphaPixel

#include <libfreenect.h>
#include <libfreenect_sync.h>

#include <liblivescene/Version.h>
#include <osg/Version>
#include <iostream>

#ifdef OSGWORKS_FOUND
#  include <osgwTools/Version.h>
#endif


int main()
{
    std::cout << "livescene version string: " << livescene::getVersionString() << std::endl;

    std::cout << "osgWorks version string: ";
#ifdef OSGWORKS_FOUND
    std::cout << osgwTools::getVersionString() << std::endl;
#else
    std::cout << "UNAVAILABLE" << std::endl;
#endif
    // Interesting, osgGetVersion() is not in a namespace...
    std::cout << "OpenSceneGraph version: " << osgGetVersion() << std::endl;

    return( 0 );
}
