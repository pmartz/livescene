// Copyright 2011 Skew Matrix Software and AlphaPixel

#ifndef __LIVESCENE_USER_INTERACTION_H__
#define __LIVESCENE_USER_INTERACTION_H__ 1

#include <liblivescene/Export.h>
#include <liblivescene/Image.h>

#include <osgViewer/GraphicsWindow>
#include <osg/Vec2s>
#include <osg/Vec3>

#include <string>
#include <vector>


namespace livescene {


/**
*/
class UserInteraction
{
public:
    LIVESCENE_EXPORT UserInteraction( osgViewer::GraphicsWindow& window );
    LIVESCENE_EXPORT ~UserInteraction();

    /** \brief Detects interactors and sends them as events.
    */
    LIVESCENE_EXPORT void detectAndSendEvents( const livescene::Image& imageRGB, const livescene::Image& imageZ );

    LIVESCENE_EXPORT struct Interactor
    {
        Interactor();

        std::string _name;
        unsigned int _id;
        osg::Vec2s _location;
        unsigned short _distance;
        osg::Vec3 _worldCoord;
        unsigned int _age;
        bool _active;
    };
    typedef std::vector< Interactor > InteractorContainer;

    /** Search the list of \c interactors and return the one closest to
    the given distance.
    \param loc Input location.
    \param interactors List of \c Interactors to search.
    \param distance Distance from input \c loc to the location of \closest.
    \return -1 if \c interactors is empty. Otherwise, sets \c distance and returns the index of the \c Interactor closest to \c loc.
    \return closest
    */
    LIVESCENE_EXPORT int getIndexOfClosest( const osg::Vec2s& loc, const InteractorContainer& interactors, float& distance ) const;

    /**  Search by name.
    */
    LIVESCENE_EXPORT int getIndexByName( const std::string& name, const InteractorContainer& interactors ) const;

    /**  Search by ID.
    */
    LIVESCENE_EXPORT int getIndexByID( const unsigned int id, const InteractorContainer& interactors ) const;

    /**  Search by ID.
    */
    LIVESCENE_EXPORT bool eraseByIndex( const unsigned int index, InteractorContainer& interactors ) const;


    void transformMouse( float& x, float& y, unsigned short devX, unsigned short devY );

protected:
    /** \brief Scans the input rgb and z images and returns a list of detected interactors.
    */
    LIVESCENE_EXPORT void defaultDetection( InteractorContainer& interactors, const livescene::Image& imageRGB, const livescene::Image& imageZ );

    /** \brief Generates user interface events.
    Compares \c newInteractors to \c lastInteractors to generate events.
    Interactors that did not previously exist generate PUSH events.
    Interactors that no longer exist generate RELEASE events.
    More TBD.
    */
    LIVESCENE_EXPORT void defaultSendEvents( InteractorContainer& lastInteractors, InteractorContainer& newInteractors );

    InteractorContainer _interactors;
    osgViewer::GraphicsWindow& _window;
};


// livescene
}


// __LIVESCENE_USER_INTERACTION_H__
#endif
