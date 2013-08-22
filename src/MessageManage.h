//
//  MessageManage.h
//  socketIO
//
//  Created by 堀 宏行 on 2013/06/05.
//
//

#ifndef socketIO_MessageManage_h
#define socketIO_MessageManage_h
#include "ofMain.h"
#include "AuEvent.h"
#include "Config.h"
#include "SessionManage.h"

#include "socket_io_client.hpp"
using namespace socketio;

class MessageManage : public ofThread
{
public:
    bool keepThread;
    SessionManage session;
    deque < Session > sendSessions;
    MessageManage() : ofThread()
    {
        
    }
    
    void setup()
    {
        session.setup();
    }
    
    void start()
    {
        startThread(false, true);
        keepThread = true;
    }
    
    void stop()
    {
        stopThread();
    }
    
    void threadedFunction()
    {
        string uri = session.connectURL;
        cout << "connect to... " << uri << endl;
        socketio_client_handler_ptr tHandler( new socketio_client_handler());
        handler = tHandler;
        client::connection_ptr con;
        client endpoint( handler );
        // Set log level. Leave these unset for no logging, or only set a few for selective logging.
        endpoint.elog().set_level(websocketpp::log::elevel::RERROR);
        endpoint.elog().set_level(websocketpp::log::elevel::FATAL);
        endpoint.elog().set_level(websocketpp::log::elevel::WARN);
        endpoint.alog().set_level(websocketpp::log::alevel::DEVEL);
        
        std::string socket_io_uri = handler->perform_handshake(uri);
        con = endpoint.get_connection(socket_io_uri);
        
        // The previous two lines can be combined:
        // con = endpoint.get_connection(handler->perform_handshake(uri));
        
        //event handler
        handler->bind_event("connected", &socketio_events::connected);
        handler->bind_event("mode_changed", &socketio_events::modeChanged);
        handler->bind_event("pa_registered", &socketio_events::paRegistered);
        handler->bind_event("tapped", &socketio_events::tapped);
        handler->bind_event("swiped", &socketio_events::swiped);
        handler->bind_event("session_started", &socketio_events::sessionStarted);
        
        endpoint.connect(con);
        
        boost::thread t(boost::bind(&client::run, &endpoint, false));
        
        // Wait for a sec before sending stuff
        while (!handler->connected()) {
            ofSleepMillis(100);
        }
        handler->emit("register_pa", "");
        
        while (keepThread) {
            if ( sendSessions.size() > 0 )
            {
                cout << "EMIT " << ofToString(sendSessions.front()._index) << endl;
                vector < string > params;
                params.push_back( ofToString(sendSessions.front()._index));
                params.push_back( ofToString(sendSessions.front().ms));
                handler->emit("start_session", params);
                sendSessions.pop_front();
            }
            ofSleepMillis(10);
        }
        
        //    // After connecting, send a connect message if using an endpoint
        //    handler->connect_endpoint("/chat");
        //
        //    std::getchar();
        //
        //    // Then to connect to another endpoint with the same socket, we call connect again.
        //    handler->connect_endpoint("/news");
        //    
        //    std::getchar();
        //    
        //    handler->emit("test", "hello!");
        //    
        //    std::getchar();
        
        
        endpoint.stop(false);
    }
    
    void addSendSession( int sessionid, int ms )
    {
        Session sess;
        sess._index = sessionid;
        sess.ms = ms;
        sendSessions.push_back( sess );
    }
    socketio_client_handler_ptr handler;
};


#endif
