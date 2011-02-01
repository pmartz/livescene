// Copyright 2011 Skew Matrix Software and AlphaPixel

#include <liblivescene/UserInteraction.h>
#include <liblivescene/Image.h>

#include <osgViewer/GraphicsWindow>
#include <osg/Vec2s>
#include <osg/io_utils>

#include <iostream>
#include <math.h>


namespace livescene {


UserInteraction::UserInteraction( osgViewer::GraphicsWindow& window )
  : _window( window )
{
}
UserInteraction::~UserInteraction()
{
}

void UserInteraction::detectAndSendEvents( const livescene::Image& imageRGB, const livescene::Image& imageZ )
{
    InteractorContainer newInteractors;

    // TBD callback
    defaultDetection( newInteractors, imageRGB, imageZ );

    // TBD callback
    defaultSendEvents( _interactors, newInteractors );

    _interactors = newInteractors;
}

void UserInteraction::defaultDetection( InteractorContainer& interactors, const livescene::Image& imageRGB, const livescene::Image& imageZ )
{
    unsigned short* ptr = ( unsigned short* )( imageZ.getData() );
    int width = imageZ.getWidth();
    int height = imageZ.getHeight();

    unsigned short minVal( 0xffff );
    osg::Vec2s minLoc;
    int sdx, tdx;
    for( tdx=0; tdx<height; tdx++ )
    {
        for( sdx=0; sdx<width; sdx++ )
        {
            if( *ptr < minVal )
            {
                minVal = *ptr;
                minLoc.set( sdx, tdx );
            }
            ptr++;
        }
    }

    Interactor newInteractor;
    newInteractor._name = "single hand";
    newInteractor._id = 121;
    newInteractor._location = minLoc;
    newInteractor._active = true;
    interactors.push_back( newInteractor );
}

void UserInteraction::defaultSendEvents( InteractorContainer& lastInteractors, const InteractorContainer& newInteractors )
{
    // Make a local copy of the const set of last Interactors.
    InteractorContainer old = lastInteractors;

    InteractorContainer::const_iterator itr;
    for( itr=newInteractors.begin(); itr != newInteractors.end(); itr++ )
    {
        const Interactor& current = *itr;
        int prevIdx = getIndexByID( current._id, lastInteractors );
        if( prevIdx >= 0 )
        {
            // It's an existing Interactor. Generate a DRAG event.

            // We found this interactor. Take it off the 'old' list.
            // Interactors remaining on the 'old' list will generate RELEASE events.
            eraseByIndex( prevIdx, old );

            Interactor& previous = lastInteractors[ prevIdx ];
            previous._location = current._location;

            float x, y;
            transformMouse( x, y, current._location.x(), current._location.y() );

            std::cout << "DRAG " << current._location;
            std::cout << " " << x << ", " << y << std::endl;
            _window.getEventQueue()->mouseMotion( x, y );
        }
        else
        {
            // It's a new Interactor. Generate a PUSH event.
            float x, y;
            transformMouse( x, y, current._location.x(), current._location.y() );

            std::cout << "PUSH " << current._location;
            std::cout << " " << x << ", " << y << std::endl;
            // TBD hardcoded left button.
            _window.getEventQueue()->mouseButtonPress( x, y, 1 );
        }
    }

    // Anything left on 'old' was not found on nowInteractors, so generate RELEASE
    for( itr=old.begin(); itr != old.end(); itr++ )
    {
        const Interactor& current = *itr;

        float x, y;
        transformMouse( x, y, current._location.x(), current._location.y() );

        std::cout << "RELEASE " << current._location;
        std::cout << " " << x << ", " << y << std::endl;
        // TBD hardcoded left button.
        _window.getEventQueue()->mouseButtonRelease( x, y, 1 );
    }

    osgGA::EventQueue* eq = _window.getEventQueue();
}


int UserInteraction::getIndexOfClosest( const osg::Vec2s& loc, const InteractorContainer& interactors, float& distance ) const
{
    int returnValue( -1 );
    if( interactors.empty() )
        return( returnValue );

    double minDistance( FLT_MAX );
    unsigned int idx;
    for( idx=0; idx < interactors.size(); idx++ )
    {
        const Interactor& candidate( interactors[ idx ] );
        double dsq = candidate._location.x() * loc.x() + candidate._location.y() * loc.y();
        if( dsq < minDistance )
        {
            minDistance = dsq;
            returnValue = (int) idx;
        }
    }
    distance = (float)( sqrt( minDistance ) );
    return( returnValue );
}

int UserInteraction::getIndexByName( const std::string& name, const InteractorContainer& interactors ) const
{
    int returnValue( -1 );
    if( interactors.empty() )
        return( returnValue );

    unsigned int idx;
    for( idx=0; idx < interactors.size(); idx++ )
    {
        const Interactor& candidate( interactors[ idx ] );
        if( candidate._name == name )
            return( (int) idx );
    }
    return( returnValue );
}

int UserInteraction::getIndexByID( const unsigned int id, const InteractorContainer& interactors ) const
{
    int returnValue( -1 );
    if( interactors.empty() )
        return( returnValue );

    unsigned int idx;
    for( idx=0; idx < interactors.size(); idx++ )
    {
        const Interactor& candidate( interactors[ idx ] );
        if( candidate._id == id )
            return( (int) idx );
    }
    return( returnValue );
}

bool UserInteraction::eraseByIndex( const unsigned int index, InteractorContainer& interactors ) const
{
    InteractorContainer::iterator itr = interactors.begin();
    unsigned int idx( 0 );
    while( idx != index )
        itr++;
    if( itr == interactors.end() )
        return( false );

    interactors.erase( itr );
    return( true );
}

void UserInteraction::transformMouse( float& x, float& y, unsigned short devX, unsigned short devY )
{
    // TBD hardcoded values.
    x = (float)devX;
    y = (float)devY;
}



UserInteraction::Interactor::Interactor()
  : _id( 0 ),
    _distance( 0 ),
    _age( 0 ),
    _active( false )
{
}



// livescene
}
