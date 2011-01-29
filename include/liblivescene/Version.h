// Copyright 2011 Skew Matrix Software and AlphaPixel

#ifndef __LIVESCENE_VERSION_H__
#define __LIVESCENE_VERSION_H__ 1

#include "liblivescene/Export.h"
#include <string>


namespace livescene {


/** \defgroup Version Version utilities */
/*@{*/

#define LIVESCENE_MAJOR_VERSION 0
#define LIVESCENE_MINOR_VERSION 0
#define LIVESCENE_SUB_VERSION 0

/** \brief livescene version number as an integer.

C preprocessor integrated version number.
The form is Mmmss, where:
   \li M is the major version
   \li mm is the minor version (zero-padded)
   \li ss is the sub version (zero padded)

Use this in version-specific code, for example:
\code
   #if( LIVESCENE_VERSION < 10500 )
      ... code specific to releases before v1.05
   #endif
\endcode
*/
#define LIVESCENE_VERSION ( \
        ( LIVESCENE_MAJOR_VERSION * 10000 ) + \
        ( LIVESCENE_MINOR_VERSION * 100 ) + \
        LIVESCENE_SUB_VERSION )

/** \brief Run-time access to the livescene version number.

Returns LIVESCENE_VERSION, the livescene version number as an integer.
\see LIVESCENE_VERSION
*/
unsigned int LIVESCENE_EXPORT getVersionNumber();

/** \brief livescene version number as a string

Example:
\code
livescene version 1.1.0 (10100)
\endcode
*/
std::string LIVESCENE_EXPORT getVersionString();

/*@}*/


// namespace livescene
}

// __LIVESCENE_VERSION_H__
#endif
