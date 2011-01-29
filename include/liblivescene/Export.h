// Copyright 2011 Skew Matrix Software and AlphaPixel

#ifndef __LIVESCENE_EXPORT__
#define __LIVESCENE_EXPORT__ 1


#if defined( _MSC_VER ) || defined( __CYGWIN__ ) || defined( __MINGW32__ ) || defined( __BCPLUSPLUS__ ) || defined( __MWERKS__ )
    #if defined( LIVESCENE_STATIC )
        #define LIVESCENE_EXPORT
    #elif defined( LIVESCENE_LIBRARY )
        #define LIVESCENE_EXPORT __declspec( dllexport )
    #else
        #define LIVESCENE_EXPORT __declspec( dllimport )
    #endif
#else
    #define LIVESCENE_EXPORT
#endif


// __LIVESCENE_EXPORT__
#endif
