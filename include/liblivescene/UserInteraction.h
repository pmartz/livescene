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


/** \brief Customizable class for generating events from device-detected motion.

Read about the multitouch feature commit at this web page.
http://www.openscenegraph.org/projects/osg/changeset/11934
An explanation of the OSG interface is here:
http://forum.openscenegraph.org/viewtopic.php?t=7702
*/
class UserInteraction
{
public:
    LIVESCENE_EXPORT UserInteraction( osgViewer::GraphicsWindow& window );
    LIVESCENE_EXPORT ~UserInteraction();

    /** \brief Detects interactors and sends them as events.
    Apps call this once per frame to generate events from image data.
    Leaves the list of current Interactors in the \c _interactors member variable.
    */
    LIVESCENE_EXPORT void detectAndSendEvents( const livescene::Image& imageRGB, const livescene::Image& imageZ );



    struct LIVESCENE_EXPORT Interactor
    {
        Interactor();

        std::string _name; // Currently unused.
        unsigned int _id; // Unique identifier.
        osg::Vec2s _location; // Device coords.
        unsigned short _distance; // Device depth value.
        osg::Vec3 _worldCoord; // Currently unused.
        unsigned int _age; // Count of frames this Interactor has existed.
        bool _active; // Enabled or disables.
    };
    typedef std::vector< Interactor > InteractorContainer;


    /** \brief Scans the input rgb and z images and returns a list of detected interactors.
    If you don't specify a DetectionCallback, this function is used to detect interactors.
    */
    LIVESCENE_EXPORT void defaultDetection( InteractorContainer& interactors, const livescene::Image& imageRGB, const livescene::Image& imageZ );

    struct LIVESCENE_EXPORT DetectionCallback : public osg::Referenced
    {
    public:
        DetectionCallback() {}
        virtual ~DetectionCallback() {}

        /** Support for detecting interactors in application code. Add interactors to
        the \interactors parameter.
        */
        virtual void operator()( livescene::UserInteraction* ui,
            livescene::UserInteraction::InteractorContainer& interactors,
            const livescene::Image& imageRGB, const livescene::Image& imageZ )
        {
            ui->defaultDetection( interactors, imageRGB, imageZ );
        }
    };
    LIVESCENE_EXPORT void setDetectionCallback( DetectionCallback* callback );
    LIVESCENE_EXPORT DetectionCallback* getDetectionCallback();
    LIVESCENE_EXPORT const DetectionCallback* getDetectionCallback() const;

    /** \brief Generates user interface events.
    If you don't set a SendEventsCallback, this function is used to send events.

    Compares \c newInteractors to \c lastInteractors to generate events.
    Interactors that did not previously exist generate PUSH events.
    Interactors that no longer exist generate RELEASE events.
    */
    LIVESCENE_EXPORT void defaultSendEvents( InteractorContainer& lastInteractors, InteractorContainer& newInteractors );

    struct LIVESCENE_EXPORT SendEventsCallback : public osg::Referenced
    {
    public:
        SendEventsCallback() {}
        virtual ~SendEventsCallback() {}

        /** Support for sending events in application code. Compare the list
        of current interactors to the list of previous interactors, and generate
        your own events.
        */
        virtual void operator()( livescene::UserInteraction* ui,
            livescene::UserInteraction::InteractorContainer& lastInteractors,
            livescene::UserInteraction::InteractorContainer& newInteractors )
        {
            ui->defaultSendEvents( lastInteractors, newInteractors );
        }
    };
    LIVESCENE_EXPORT void setSendEventsCallback( SendEventsCallback* callback );
    LIVESCENE_EXPORT SendEventsCallback* getSendEventsCallback();
    LIVESCENE_EXPORT const SendEventsCallback* getSendEventsCallback() const;


    /** If using the defaultSendEvents, specify the mapping of interactions to events.
    */
    enum ContactType {
        CONTACT_START,
        MOTION,
        CONTACT_END,
        PUNCH
    };
    typedef std::map< ContactType, osgGA::GUIEventAdapter::EventType > ContactEventMap;

    LIVESCENE_EXPORT void setContactEventMap( const ContactEventMap& map );
    LIVESCENE_EXPORT void getContactEventMap( ContactEventMap& map );

    /** Specify the distance to detect a hand. This is the inactive distance,
    and should be a larger value than the setDefaultActiveThreshold.
    */
    LIVESCENE_EXPORT void setDefaultDetectionThreshold( unsigned short distance );
    LIVESCENE_EXPORT unsigned short getDefaultDetectionThreshold() const;

    /** Specify the distance to detect an active hand. This is the active distance,
    and should be a smaller value than the setDefaultDetectionThreshold.
    */
    LIVESCENE_EXPORT void setDefaultActiveThreshold( unsigned short distance );
    LIVESCENE_EXPORT unsigned short getDefaultActiveThreshold() const;

    // 1=left, 2=middle, 3=right
    LIVESCENE_EXPORT void setDefaultSendEventsButton( int button );
    LIVESCENE_EXPORT int getDefaultSendEventsButton() const;

    LIVESCENE_EXPORT void setDefaultSendEventsPunchMaxAge( unsigned int age );
    LIVESCENE_EXPORT unsigned int getDefaultSendEventsPunchMaxAge() const;


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


    LIVESCENE_EXPORT void transformMouse( float& x, float& y, unsigned short devX, unsigned short devY );

    LIVESCENE_EXPORT osgViewer::GraphicsWindow& getGraphicsWindow() { return( _window ); }

protected:
    InteractorContainer _interactors;
    osgViewer::GraphicsWindow& _window;

    osg::ref_ptr< DetectionCallback > _detectionCallback;
    osg::ref_ptr< SendEventsCallback > _sendEventsCallback;

    ContactEventMap _eventMap;

    unsigned short _defaultDetectionThreshold;
    unsigned short _defaultActiveThreshold;
    int _defaultSendEventsButton;
    unsigned int _defaultSendEventsPunchMaxAge;
};


// livescene
}


// __LIVESCENE_USER_INTERACTION_H__
#endif
