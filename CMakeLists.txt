# # # # # #  project setup  # # # # # #
CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )
PROJECT( livescene )

set( LIVESCENE_VERSION 0.0 )

SET( CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}" )

SET( LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single directory for all libraries." )
MAKE_DIRECTORY( ${LIBRARY_OUTPUT_PATH} )
MARK_AS_ADVANCED( LIBRARY_OUTPUT_PATH )

SET( EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single directory for all executables." )
MAKE_DIRECTORY( ${EXECUTABLE_OUTPUT_PATH} )
MARK_AS_ADVANCED( EXECUTABLE_OUTPUT_PATH )

if( WIN32 AND MSVC )
    OPTION( LIVESCENE_WIN32_USE_MP "Build with multiple processes (/MP option; VS 2005 and above)." ON )
    MARK_AS_ADVANCED( LIVESCENE_WIN32_USE_MP )
    IF( LIVESCENE_WIN32_USE_MP )
        SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP" )
    ENDIF( LIVESCENE_WIN32_USE_MP )
endif()


include( CMakeMacros )

include( FindOSGHelper )


# Add OSG for entire project for include & lib paths
include_directories( 
    ${PROJECT_SOURCE_DIR}/include
    ${OSG_INCLUDE_DIRS}
)
# HACK: Not sure why this is necessary, but linker can't find library without it.
if( WIN32 AND MSVC )
    LINK_DIRECTORIES(
        ${EXECUTABLE_OUTPUT_PATH}/Debug
    )
endif()


option( BUILD_SHARED_LIBS "Set to ON to build livescene for dynamic linking. Use OFF for static." ON )
if( NOT BUILD_SHARED_LIBS )
    add_definitions( -DLIVESCENE_STATIC )
endif()


add_subdirectory( src )


#
# Doxygen
#INCLUDE( ${CMAKE_ROOT}/Modules/Documentation.cmake OPTIONAL )
option( LIVESCENE_DOCUMENTATION "Enable to create build target for Doxygen documentation." OFF )
mark_as_advanced( CLEAR LIVESCENE_DOCUMENTATION )
if( LIVESCENE_DOCUMENTATION )
    FIND_PACKAGE( Doxygen REQUIRED )
    SET( HAVE_DOT "NO" )
    if( DOXYGEN_DOT_PATH )
        SET( HAVE_DOT "YES" )
    endif()
    
    CONFIGURE_FILE( ${PROJECT_SOURCE_DIR}/doc/doxyfile.cmake
        ${PROJECT_BINARY_DIR}/doc/doxyfile
    )
    ADD_CUSTOM_TARGET( Documentation ${DOXYGEN_EXECUTABLE}
        ${PROJECT_BINARY_DIR}/doc/doxyfile
    )
endif()
