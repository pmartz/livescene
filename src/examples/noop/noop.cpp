// Copyright 2011 Skew Matrix Software and Alpha Pixel

#include <libfreenect.h>
#include <libfreenect_sync.h>

#include <liblivescene/Version.h>
#include <iostream>

#ifdef OSGWORKS_FOUND
#  include <osgwTools/Version.h>
#endif


int main()
{
    std::cout << livescene::getVersionString() << std::endl;

#ifdef OSGWORKS_FOUND
    std::cout << osgwTools::getVersionString() << std::endl;
#endif

    return( 0 );
}
