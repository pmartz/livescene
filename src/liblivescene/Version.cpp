// Copyright 2011 Skew Matrix Software and Alpha Pixel

#include "liblivescene/Version.h"
#include <string>
#include <sstream>

namespace livescene {


unsigned int getVersionNumber()
{
    return( LIVESCENE_VERSION );
}


static std::string s_livescene_version( "" );

std::string getVersionString()
{
    if( s_livescene_version.empty() )
    {
        std::ostringstream oStr;
        oStr << std::string( "livescene version " ) <<
            LIVESCENE_MAJOR_VERSION << "." <<
            LIVESCENE_MINOR_VERSION << "." <<
            LIVESCENE_SUB_VERSION << " (" <<
            getVersionNumber() << ").";
        s_livescene_version = oStr.str();
    }
    return( s_livescene_version );
}


// namespace livescene
}
