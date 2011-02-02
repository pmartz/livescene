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

    if( _detectionCallback.valid() )
        (*_detectionCallback)( this, newInteractors, imageRGB, imageZ );
    else
        defaultDetection( newInteractors, imageRGB, imageZ );

    if( _sendEventsCallback.valid() )
        (*_sendEventsCallback)( this, _interactors, newInteractors );
    else
        defaultSendEvents( _interactors, newInteractors );

    _interactors = newInteractors;
}

void UserInteraction::setDetectionCallback( DetectionCallback* callback )
{
    _detectionCallback = callback;
}
UserInteraction::DetectionCallback* UserInteraction::getDetectionCallback()
{
    return( _detectionCallback.get() );
}
const UserInteraction::DetectionCallback* UserInteraction::getDetectionCallback() const
{
    return( _detectionCallback.get() );
}

void UserInteraction::setSendEventsCallback( SendEventsCallback* callback )
{
    _sendEventsCallback = callback;
}
UserInteraction::SendEventsCallback* UserInteraction::getSendEventsCallback()
{
    return( _sendEventsCallback.get() );
}
const UserInteraction::SendEventsCallback* UserInteraction::getSendEventsCallback() const
{
    return( _sendEventsCallback.get() );
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

    const unsigned short threashold( 600 );

    Interactor newInteractor;
    newInteractor._name = "single hand";
    newInteractor._id = 121;
    newInteractor._location = minLoc;
    newInteractor._distance = minVal;
    newInteractor._active = ( minVal < threashold );
    interactors.push_back( newInteractor );
}

void UserInteraction::defaultSendEvents( InteractorContainer& lastInteractors, InteractorContainer& newInteractors )
{
    // TBD hardcoded value.
    const int mouseButton( 1 );

    // Make a local copy of the const set of last Interactors.
    InteractorContainer old = lastInteractors;

    InteractorContainer::iterator itr;
    for( itr=newInteractors.begin(); itr != newInteractors.end(); itr++ )
    {
        Interactor& current = *itr;
        int prevIdx = getIndexByID( current._id, lastInteractors );
        if( prevIdx >= 0 )
        {
            // It's an existing Interactor. Generate a DRAG event.
            // Or generate a RELEASE event if current not active and previous was active.
            // Or generate a PUSH event if current active and previous not active.

            // We found this interactor. Take it off the 'old' list.
            // Interactors remaining on the 'old' list will generate RELEASE events.
            eraseByIndex( prevIdx, old );

            Interactor& previous = lastInteractors[ prevIdx ];
            current._age = previous._age+1;

            float x, y;
            transformMouse( x, y, current._location.x(), current._location.y() );
 
            if( previous._active && !( current._active ) )
            {
                unsigned int punchMaxAge( 8 );
                if( ( current._age < punchMaxAge ) && ( current._age > 2 ) )
                {
                    _window.getEventQueue()->keyPress( ' ' );
                    _window.getEventQueue()->keyRelease( ' ' );
                }
                else
                {
                    // In OSG, mouse seems to generate a DRAG just before PUSH.
                    _window.getEventQueue()->mouseMotion( x, y );
                    //std::cout << " * RELEASE " << current._location;
                    //std::cout << " " << x << ", " << y << std::endl;
                    _window.getEventQueue()->mouseButtonRelease( x, y, mouseButton );
                }
            }
            else if( current._active && !( previous._active ) )
            {
                //std::cout << " * PUSH " << current._location;
                //std::cout << " " << x << ", " << y << std::endl;
                _window.getEventQueue()->mouseButtonPress( x, y, mouseButton );
            }
            else if( current._active && previous._active )
            {
                //std::cout << "DRAG " << current._location;
                //std::cout << " " << x << ", " << y << std::endl;
                _window.getEventQueue()->mouseMotion( x, y );
            }
            if( !current._active )
            {
                current._age = 0;
            }
        }
        else
        {
            // It's a new Interactor. Generate a PUSH event.
            float x, y;
            transformMouse( x, y, current._location.x(), current._location.y() );

            //std::cout << "PUSH " << current._location;
            //std::cout << " " << x << ", " << y << std::endl;
            _window.getEventQueue()->mouseButtonPress( x, y, mouseButton );
        }
    }

    // Anything left on 'old' was not found on nowInteractors, so generate RELEASE
    for( itr=old.begin(); itr != old.end(); itr++ )
    {
        const Interactor& current = *itr;

        float x, y;
        transformMouse( x, y, current._location.x(), current._location.y() );

        //std::cout << "RELEASE " << current._location;
        //std::cout << " " << x << ", " << y << std::endl;
        _window.getEventQueue()->mouseButtonRelease( x, y, mouseButton );
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
    int xOrigin, yOrigin, width, height;
    _window.getWindowRectangle( xOrigin, yOrigin, width, height );

    // TBD hardcoded values.
    float scaleX = (float)width / 640.f;
    float scaleY = (float)height / 480.f;

    // Arm to left appears in camera right, so flip x values.
    unsigned short flippedX = 640 - devX;
    x = (float)flippedX * scaleX;
    y = (float)devY * scaleY;
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
