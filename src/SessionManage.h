//
//  SessionManage.h
//  socketIO
//
//  Created by 堀 宏行 on 2013/06/09.
//
//

#ifndef socketIO_SessionManage_h
#define socketIO_SessionManage_h
#include "ofxXmlSettings.h"
#include "ofxOsc.h"
#include "Config.h"
class Session
{
public:
    string name;
    int _index;
    int ms;
};

class SessionManage
{
public:
    ofxXmlSettings sessionXML;
    string connectURL;
    vector< Session > musicSessions;
    vector< Session > graphicSessions;
    void setup()
    {
        sessionXML.loadFile( "setting/session.xml");
        connectURL = sessionXML.getValue( "xml:url", "no host");
        {
            ofxOscMessage m;
            m.setAddress( "/musicSession/clear");
            Config::GetInstance().sender.sendMessage(m);
            ofSleepMillis(100);
        }
        {
            ofxOscMessage m;
            m.setAddress( "/graphicSession/clear");
            Config::GetInstance().sender.sendMessage(m);
            ofSleepMillis(100);
        }
        sessionXML.pushTag("xml");
        {
            sessionXML.pushTag("musicsessions");
            int numSession = sessionXML.getNumTags("session");
            for ( int i = 0; i < numSession; i++)
            {
                sessionXML.pushTag("session", i);
                Session sess;
                musicSessions.push_back( sess );
                musicSessions.back().name = sessionXML.getValue( "name", "def" );
                musicSessions.back()._index = sessionXML.getValue( "index", 0 );
                musicSessions.back().ms = sessionXML.getValue( "startedatsince", 1000 );
                ofxOscMessage m;
                m.setAddress( "/musicSession");
                m.addStringArg(musicSessions.back().name);
                m.addIntArg(musicSessions.back()._index);
                m.addIntArg(musicSessions.back().ms);
                Config::GetInstance().sender.sendMessage(m);
                sessionXML.popTag();
            }
            sessionXML.popTag();
        }
        {
            sessionXML.pushTag("graphicsessions");
            int numSession = sessionXML.getNumTags("session");
            for ( int i = 0; i < numSession; i++)
            {
                sessionXML.pushTag("session", i);
                Session sess;
                graphicSessions.push_back( sess );
                graphicSessions.back().name = sessionXML.getValue( "name", "def" );
                graphicSessions.back()._index = sessionXML.getValue( "index", 0 );
                graphicSessions.back().ms = sessionXML.getValue( "startedatsince", 1000 );
                ofxOscMessage m;
                m.setAddress( "/graphicSession");
                m.addStringArg(graphicSessions.back().name);
                m.addIntArg(graphicSessions.back()._index);
                m.addIntArg(graphicSessions.back().ms);
                Config::GetInstance().sender.sendMessage(m);
                sessionXML.popTag();
            }
            sessionXML.popTag();
        }
        sessionXML.popTag();
    }
};

#endif
