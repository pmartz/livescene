// Copyright 2011 Skew Matrix Software and AlphaPixel

#include <liblivescene/UserInteraction.h>
#include <liblivescene/Image.h>

#include <osgViewer/GraphicsWindow>
#include <osg/Vec2s>
#include <osg/io_utils>

#include <iostream>
#include <math.h>


namespace livescene {


UserInteraction::UserInteraction( osgViewer::GraphicsWindow* window )
  : _window( window ),
    _defaultDetectionThreshold( 700 ),
    _defaultActiveThreshold( 500 ),
    _defaultSendEventsButton( 1 ),
    _defaultSendEventsPunchMaxAge( 9 )
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

    unsigned short undetected( 0xffff );
    unsigned short minVal( undetected );
    osg::Vec2s minLoc;
    int sdx, tdx;
    for( tdx=0; tdx<height; ++tdx )
    {
        for( sdx=0; sdx<width; ++sdx )
        {
            if( *ptr < minVal )
            {
                // Sometimes get spurious sudden changes in z, 300-400 units closer
                // to the Kinect. Tried sampling corners of a 4x4 square but saw _all_
                // those sample points jump closer. Now sampling corners of a 8x8
                // square instead, and that seems to filter out this problem.
                if( ( sdx < ( width - 8 ) ) &&
                    ( tdx < ( height - 8 ) ) )
                {
                    unsigned short sample1 = *( ptr + 7 );
                    unsigned short sample2 = *( ptr + (width * 7) );
                    unsigned short sample3 = *( ptr + (width * 7) + 7 );
                    unsigned short targetDistance = (*ptr) * 1.1; // 110%
                    if( ( sample1 < targetDistance ) &&
                        ( sample2 < targetDistance ) &&
                        ( sample3 < targetDistance ) )
                    {
                        minVal = *ptr;
                        ptr += ( (width * 7) + 7 ); // scan past the vertices we just sampled.
                        minLoc.set( sdx, tdx );
                        sdx += 7;
                        tdx += 7;
                    }
                }
            }
            ++ptr;
        }
    }

    if( minVal < getDefaultDetectionThreshold() )
    {
        Interactor newInteractor;
        newInteractor._name = "single hand";
        newInteractor._id = 121;
        newInteractor._location = minLoc;
        newInteractor._distance = minVal;
        newInteractor._active = ( minVal < getDefaultActiveThreshold() );
        //std::cout << "Active: " << newInteractor._active << " because: " << minVal << " and " << getDefaultActiveThreshold() << std::endl;
        interactors.push_back( newInteractor );
    }
    else
        std::cout << "Undetected. minVal: " << minVal << std::endl;
}

void UserInteraction::defaultSendEvents( InteractorContainer& lastInteractors, InteractorContainer& newInteractors )
{
    osgGA::EventQueue* eq = _window->getEventQueue();

    // Make a local copy of the const set of last Interactors.
    InteractorContainer old = lastInteractors;
    //std::cout << "initial " << old.size() << std::endl;

    InteractorContainer::iterator itr;
    for( itr=newInteractors.begin(); itr != newInteractors.end(); ++itr )
    {
        Interactor& current = *itr;
        int prevIdx = getIndexByID( current._id, lastInteractors );
        //std::cout << current._id << "  " << prevIdx << std::endl;
        if( prevIdx >= 0 )
        {
            // It's an existing Interactor. Generate a DRAG event.
            // Or generate a RELEASE event if current not active and previous was active.
            // Or generate a PUSH event if current active and previous not active.

            // We found this interactor. Take it off the 'old' list.
            // Interactors remaining on the 'old' list will generate RELEASE events.
            eraseByIndex( prevIdx, old );
            //std::cout << "post erase " << old.size() << std::endl;

            Interactor& previous = lastInteractors[ prevIdx ];
            current._age = previous._age+1;

            float x, y;
            transformMouse( x, y, current._location.x(), current._location.y() );
 
            if( previous._active && !( current._active ) )
            {
                if( current._age < _defaultSendEventsPunchMaxAge )
                {
                    // PUNCH event.
                    eq->keyPress( ' ' );
                    eq->keyRelease( ' ' );
                }
                else
                {
                    // In OSG, mouse seems to generate a DRAG just before RELEASE.
                    eq->mouseMotion( x, y );
                    //std::cout << " * RELEASE " << current._location;
                    //std::cout << " " << x << ", " << y << std::endl;
                    eq->mouseButtonRelease( x, y, _defaultSendEventsButton );
                }
            }
            else if( current._active && !( previous._active ) )
            {
                //std::cout << " * PUSH " << current._location;
                //std::cout << " " << x << ", " << y << std::endl;
                eq->mouseButtonPress( x, y, _defaultSendEventsButton );
            }
            else
            {
                // Regardless of active/inactive status, issue a mouse
                // motion event.
                /*
                if( current._active )
                    std::cout << "DRAG ";
                else
                    std::cout << "MOVE ";
                std::cout << current._location << "  " << x << ", " << y << std::endl;
                */
                eq->mouseMotion( x, y );
            }
            if( !current._active )
            {
                current._age = 0;
            }
        }
        else if( current._active )
        {
            // It's a new Interactor. Generate a PUSH event.
            float x, y;
            transformMouse( x, y, current._location.x(), current._location.y() );

            //std::cout << "PUSH " << current._location;
            //std::cout << " " << x << ", " << y << std::endl;
            eq->mouseButtonPress( x, y, _defaultSendEventsButton );
        }
    }

    //std::cout << "issuing release " << old.size() << std::endl;
    // Anything left on 'old' was not found on newInteractors, so generate RELEASE
    for( itr=old.begin(); itr != old.end(); ++itr )
    {
        const Interactor& current = *itr;

        float x, y;
        transformMouse( x, y, current._location.x(), current._location.y() );

        std::cout << "Mouse RELEASE event " << current._location;
        std::cout << " " << x << ", " << y << std::endl;
        eq->mouseButtonRelease( x, y, _defaultSendEventsButton );
    }
}


void UserInteraction::setContactEventMap( const ContactEventMap& map )
{
    _eventMap = map;
}
void UserInteraction::getContactEventMap( ContactEventMap& map )
{
    map = _eventMap;
}

void UserInteraction::setDefaultDetectionThreshold( unsigned short distance )
{
    _defaultDetectionThreshold = distance;
}
unsigned short UserInteraction::getDefaultDetectionThreshold() const
{
    return( _defaultDetectionThreshold );
}

void UserInteraction::setDefaultActiveThreshold( unsigned short distance )
{
    _defaultActiveThreshold = distance;
}
unsigned short UserInteraction::getDefaultActiveThreshold() const
{
    return( _defaultActiveThreshold );
}

void UserInteraction::setDefaultSendEventsButton( int button )
{
    _defaultSendEventsButton = button;
}
int UserInteraction::getDefaultSendEventsButton() const
{
    return( _defaultSendEventsButton );
}

void UserInteraction::setDefaultSendEventsPunchMaxAge( unsigned int age )
{
    _defaultSendEventsPunchMaxAge = age;
}
unsigned int UserInteraction::getDefaultSendEventsPunchMaxAge() const
{
    return( _defaultSendEventsPunchMaxAge );
}


int UserInteraction::getIndexOfClosest( const osg::Vec2s& loc, const InteractorContainer& interactors, float& distance ) const
{
    int returnValue( -1 );
    if( interactors.empty() )
        return( returnValue );

    double minDistance( FLT_MAX );
    unsigned int idx;
    for( idx=0; idx < interactors.size(); ++idx )
    {
        const Interactor& candidate( interactors[ idx ] );
        float a = candidate._location.x() - loc.x();
        float b = candidate._location.y() - loc.y();
        double dsq = a * a + b * b;
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
    for( idx=0; idx < interactors.size(); ++idx )
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
    for( idx=0; idx < interactors.size(); ++idx )
    {
        const Interactor& candidate( interactors[ idx ] );
        if( candidate._id == id )
            return( (int) idx );
    }
    return( returnValue );
}

bool UserInteraction::eraseByIndex( const unsigned int index, InteractorContainer& interactors ) const
{
    //std::cout << "eraseByIndex for index " << index << " size " << interactors.size() << std::endl;

    InteractorContainer::iterator itr = interactors.begin();
    unsigned int idx( 0 );
    while( ( idx != index ) && ( itr != interactors.end() ) )
    {
        ++idx;
        ++itr;
    }
    if( itr == interactors.end() )
    {
        std::cout << "  eraseByIndex failed to find index " << index << std::endl;
        return( false );
    }

    //std::cout << "  eraseByIndex erasing index " << index << std::endl;
    interactors.erase( itr );
    //for( itr = interactors.begin(); itr != interactors.end(); ++itr )
    //    std::cout << "    " << itr->_location.x() << "," << itr->_location.y() << std::endl;
    return( true );
}

void UserInteraction::transformMouse( float& x, float& y, unsigned short devX, unsigned short devY )
{
    int xOrigin, yOrigin, width, height;
    _window->getWindowRectangle( xOrigin, yOrigin, width, height );

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
