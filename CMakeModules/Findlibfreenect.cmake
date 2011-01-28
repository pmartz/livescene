# Locate libfreenect.
#
# This script defines:
#   LIBFREENECT_FOUND, set to 1 if found
#   FREENECT_LIBRARY
#   FREENECT_SYNC_LIBRARY
#   LIBFREENECT_LIBRARIES
#   FREENECT_INCLUDE_DIR
#   FREENECT_SYNC_INCLUDE_DIR
#   LIBFREENECT_INCLUDE_DIRS
#
# This script will look in standard locations for installed libfreenect. However, if you
# install libfreenect into a non-standard location, you can use the LIBFREENECT_ROOT
# variable (in environment or CMake) to specify the location.
#
# You can also use libfreenect out of a source tree by specifying LIBFREENECT_SOURCE_DIR
# and LIBFREENECT_BUILD_DIR (in environment or CMake).


set( _libfreenectSearchPaths
    /usr/local
    /usr
    /sw/ # Fink
    /opt/local # DarwinPorts
    /opt/csw # Blastwave
    /opt
    "C:/Program Files/libfreenect"
    "C:/Program Files (x86)/libfreenect"
    ~/Library/Frameworks
    /Library/Frameworks
)

macro( FIND_LIBFREENECT_HEADER MYHEADER MYHEADERNAME )
    mark_as_advanced( ${MYHEADER} )
    find_path( ${MYHEADER}
        ${MYHEADERNAME}
        HINTS
            ${LIBFREENECT_ROOT}
            $ENV{LIBFREENECT_ROOT}
            ${LIBFREENECT_SOURCE_DIR}
            $ENV{LIBFREENECT_SOURCE_DIR}
        PATH_SUFFIXES
            include
            wrappers/c_sync
        PATHS
            ${_libfreenectSearchPaths}
    )
    if( ${MYHEADER} )
        list( APPEND LIBFREENECT_INCLUDE_DIRS
            ${${MYHEADER}}
        )
    endif()
endmacro()

unset( LIBFREENECT_INCLUDE_DIRS )
FIND_LIBFREENECT_HEADER( FREENECT_INCLUDE_DIR libfreenect.h )
FIND_LIBFREENECT_HEADER( FREENECT_SYNC_INCLUDE_DIR libfreenect_sync.h )
# message( STATUS ${LIBFREENECT_INCLUDE_DIRS} )



macro( FIND_LIBFREENECT_LIBRARY MYLIBRARY MYLIBRARYNAME )
    mark_as_advanced( ${MYLIBRARY} )
    mark_as_advanced( ${MYLIBRARY}_debug )
    find_library( ${MYLIBRARY}
        NAMES
            ${MYLIBRARYNAME}
        HINTS
            ${LIBFREENECT_ROOT}
            $ENV{LIBFREENECT_ROOT}
            ${LIBFREENECT_BUILD_DIR}
            $ENV{LIBFREENECT_BUILD_DIR}
        PATH_SUFFIXES
            lib
            lib/Release
            bin
            bin/Release
        PATHS
            ${_libfreenectSearchPaths}
    )
    find_library( ${MYLIBRARY}_debug
        NAMES
            ${MYLIBRARYNAME}
        HINTS
            ${LIBFREENECT_ROOT}
            $ENV{LIBFREENECT_ROOT}
            ${LIBFREENECT_BUILD_DIR}
            $ENV{LIBFREENECT_BUILD_DIR}
        PATH_SUFFIXES
            lib
            lib/Debug
            bin
            bin/Debug
        PATHS
            ${_libfreenectSearchPaths}
    )
#    message( STATUS ${${MYLIBRARY}} ${${MYLIBRARY}_debug} )
#    message( STATUS ${MYLIBRARYNAME} )
    if( ${MYLIBRARY} )
        list( APPEND LIBFREENECT_LIBRARIES
            "optimized" ${${MYLIBRARY}}
        )
    endif()
    if( ${MYLIBRARY}_debug )
        list( APPEND LIBFREENECT_LIBRARIES
            "debug" ${${MYLIBRARY}_debug}
        )
    endif()
#    message( STATUS ${LIBFREENECT_LIBRARIES} )
endmacro()

unset( LIBFREENECT_LIBRARIES )
FIND_LIBFREENECT_LIBRARY( FREENECT_LIBRARY freenect )
FIND_LIBFREENECT_LIBRARY( FREENECT_SYNC_LIBRARY freenect_sync )

# handle the QUIETLY and REQUIRED arguments and set FMOD_FOUND to TRUE if all listed variables are TRUE
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args(
    libfreenect
    DEFAULT_MSG 
    LIBFREENECT_LIBRARIES 
    LIBFREENECT_INCLUDE_DIRS
)
