# Locate osgWorks.
#
# This script defines:
#   OSGWORKS_FOUND, set to 1 if found
#   OSGWORKS_LIBRARIES
#   OSGWORKS_INCLUDE_DIR
#   OSGWORKS_INCLUDE_DIRS
#   OSGWCONTROLS_LIBRARY
#   OSGWTOOLS_LIBRARY
#
# This script will look in standard locations for installed osgWorks. However, if you
# install osgWorks into a non-standard location, you can use the OSGWORKS_ROOT
# variable (in environment or CMake) to specify the location.
#
# You can also use osgWorks out of a source tree by specifying OSGWORKS_SOURCE_DIR
# and OSGWORKS_BUILD_DIR (in environment or CMake).


set( _osgWorksSearchPathS
    /usr/local
    /usr
    /sw/ # Fink
    /opt/local # DarwinPorts
    /opt/csw # Blastwave
    /opt
    "C:/Program Files/osgWorks"
    "C:/Program Files (x86)/osgWorks"
    ~/Library/Frameworks
    /Library/Frameworks
)

find_path( OSGWORKS_INCLUDE_DIR
    osgwTools/FindNamedNode.h
    HINTS
        ${OSGWORKS_ROOT}
        $ENV{OSGWORKS_ROOT}
        ${OSGWORKS_SOURCE_DIR}
        $ENV{OSGWORKS_SOURCE_DIR}
    PATH_SUFFIXES
        include
    PATHS
        ${_osgWorksSearchPathS}
)

unset( OSGWORKS_INCLUDE_DIRS )
list( APPEND OSGWORKS_INCLUDE_DIRS
    ${OSGWORKS_INCLUDE_DIR}
)
mark_as_advanced( OSGWORKS_INCLUDE_DIR )
mark_as_advanced( OSGWORKS_INCLUDE_DIRS )
# message( STATUS ${OSGWORKS_INCLUDE_DIR} )



macro( FIND_OSGWORKS_LIBRARY MYLIBRARY MYLIBRARYNAME )
    mark_as_advanced( ${MYLIBRARY} )
    mark_as_advanced( ${MYLIBRARY}_debug )
    find_library( ${MYLIBRARY}
        NAMES
            ${MYLIBRARYNAME}
        HINTS
            ${OSGWORKS_ROOT}
            $ENV{OSGWORKS_ROOT}
            ${OSGWORKS_BUILD_DIR}
            $ENV{OSGWORKS_BUILD_DIR}
        PATH_SUFFIXES
            lib
            bin
            bin/Release
        PATHS
            ${_osgWorksSearchPathS}
    )
    find_library( ${MYLIBRARY}_debug
        NAMES
            ${MYLIBRARYNAME}d
        HINTS
            ${OSGWORKS_ROOT}
            $ENV{OSGWORKS_ROOT}
            ${OSGWORKS_BUILD_DIR}
            $ENV{OSGWORKS_BUILD_DIR}
        PATH_SUFFIXES
            lib
            bin
            bin/Debug
        PATHS
            ${_osgWorksSearchPathS}
    )
#    message( STATUS ${${MYLIBRARY}} ${${MYLIBRARY}_debug} )
#    message( STATUS ${MYLIBRARYNAME} )
    if( ${MYLIBRARY} )
        list( APPEND OSGWORKS_LIBRARIES
            "optimized" ${${MYLIBRARY}}
        )
    endif()
    if( ${MYLIBRARY}_debug )
        list( APPEND OSGWORKS_LIBRARIES
            "debug" ${${MYLIBRARY}_debug}
        )
    endif()
#    message( STATUS ${OSGWORKS_LIBRARIES} )
endmacro()

unset( OSGWORKS_LIBRARIES )
FIND_OSGWORKS_LIBRARY( OSGWTOOLS_LIBRARY osgwTools )
FIND_OSGWORKS_LIBRARY( OSGWCONTROLS_LIBRARY osgwControls )

# handle the QUIETLY and REQUIRED arguments and set FMOD_FOUND to TRUE if all listed variables are TRUE
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args(
    OSGWorks
    DEFAULT_MSG 
    OSGWORKS_LIBRARIES 
    OSGWORKS_INCLUDE_DIR
)
