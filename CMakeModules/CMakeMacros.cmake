IF(WIN32)
    SET(CMAKE_DEBUG_POSTFIX d)
ENDIF(WIN32)


MACRO( ADD_SHARED_LIBRARY_INTERNAL TRGTNAME )
    ADD_LIBRARY( ${TRGTNAME} SHARED ${ARGN} )
    IF( WIN32 )
        SET_TARGET_PROPERTIES( ${TRGTNAME} PROPERTIES DEBUG_POSTFIX d )
    ENDIF( WIN32 )
    SET_TARGET_PROPERTIES( ${TRGTNAME} PROPERTIES PROJECT_LABEL "Lib ${TRGTNAME}" )
ENDMACRO( ADD_SHARED_LIBRARY_INTERNAL TRGTNAME )

MACRO( ADD_OSGPLUGIN TRGTNAME )
    if( BUILD_SHARED_LIBS )
        add_library( ${TRGTNAME} MODULE ${ARGN} )
    else()
        add_library( ${TRGTNAME} STATIC ${ARGN} )
    endif()

    link_internal( ${TRGTNAME}
        liblivescene
    )
    target_link_libraries( ${TRGTNAME}
        ${OSG_LIBRARIES}
        ${LIBFREENECT_LIBRARIES}
    )
    if( OSGWORKS_FOUND )
        target_link_libraries( ${TRGTNAME}
            ${OSGWORKS_LIBRARIES}
        )
    endif()

    IF( WIN32 )
        SET_TARGET_PROPERTIES( ${TRGTNAME} PROPERTIES DEBUG_POSTFIX d )
    ENDIF( WIN32 )
    SET_TARGET_PROPERTIES( ${TRGTNAME} PROPERTIES PREFIX "" )
    SET_TARGET_PROPERTIES( ${TRGTNAME} PROPERTIES PROJECT_LABEL "Plugin ${TRGTNAME}" )
ENDMACRO( ADD_OSGPLUGIN TRGTNAME )


MACRO( MAKE_EXECUTABLE EXENAME )
    ADD_EXECUTABLE_INTERNAL( ${EXENAME}
        ${ARGN}
    )
    LINK_INTERNAL( ${EXENAME}
        liblivescene
    )
    TARGET_LINK_LIBRARIES( ${EXENAME}
        ${OSG_LIBRARIES}
        ${LIBFREENECT_LIBRARIES}
    )
    if( OSGWORKS_FOUND )
        target_link_libraries( ${EXENAME}
            ${OSGWORKS_LIBRARIES}
        )
    endif()
    # Requires ${CATAGORY}
    SET_TARGET_PROPERTIES( ${EXENAME} PROPERTIES PROJECT_LABEL "${CATEGORY} ${EXENAME}" )
ENDMACRO( MAKE_EXECUTABLE EXENAME )

MACRO( ADD_EXECUTABLE_INTERNAL TRGTNAME )
    ADD_EXECUTABLE( ${TRGTNAME} ${ARGN} )
    IF( WIN32 )
        SET_TARGET_PROPERTIES( ${TRGTNAME} PROPERTIES DEBUG_POSTFIX d )
    ENDIF(WIN32)
ENDMACRO( ADD_EXECUTABLE_INTERNAL TRGTNAME )

MACRO( LINK_INTERNAL TRGTNAME )
    FOREACH(LINKLIB ${ARGN})
        TARGET_LINK_LIBRARIES(${TRGTNAME} optimized "${LINKLIB}" debug "${LINKLIB}${CMAKE_DEBUG_POSTFIX}")
    ENDFOREACH(LINKLIB)
ENDMACRO( LINK_INTERNAL TRGTNAME )
