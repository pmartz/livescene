set( LIB_NAME liblivescene )

if( BUILD_SHARED_LIBS )
    add_definitions( -DLIVESCENE_LIBRARY )
endif()


set( HEADER_PATH ${PROJECT_SOURCE_DIR}/include/${LIB_NAME} )
set( LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/Export.h
    ${HEADER_PATH}/Version.h
)

set( _livesceneSourceFiles
    Version.cpp
)

add_library( ${LIB_NAME}
    ${LIB_PUBLIC_HEADERS}
    ${_livesceneSourceFiles}
)
if( BUILD_SHARED_LIBS )
    target_link_libraries( ${LIB_NAME}
        ${OSG_LIBRARIES}
    )
endif()

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${LIVESCENE_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${LIVESCENE_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Lib ${LIB_NAME}" )

include( ModuleInstall REQUIRED )
