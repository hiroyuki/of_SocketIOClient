//
//  ofxSocketIOClient.h
//  example
//
//  Created by 堀 宏行 on 2013/08/22.
//
//

#ifndef example_ofxSocketIOClient_h
#define example_ofxSocketIOClient_h
#include "ofMain.h" 
#include "socket_io_client.hpp"
#include "SocketIOEvents.h"

using namespace socketio;

template <class T>
class ofxSocketIOClient : public ofThread
{
public:
    typedef boost::shared_ptr<socketio_client_handler<T > > socketio_client_handler_ptr;
    socketio_client_handler_ptr handler;
    SocketIOEvents event;
    client* endpoint;
    virtual void setup()
    {
        socketio_client_handler_ptr handler(new socketio_client_handler<T >());
        this->handler = handler;
        endpoint = new client( handler );
    }
    
    virtual void connect(string url)
    {
        try
        {
            client::connection_ptr con;
            string socket_io_url = handler->perform_handshake(url);
            con = endpoint->get_connection( socket_io_url );
            endpoint->connect(con);
            boost::thread t( boost::bind(&client::run, endpoint, false));
            ofThread::startThread();
        }
        catch (std::exception& e)
        {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
    }
    
    virtual void threadedFunction()
    {
    }
    
    virtual void disconnect()
    {
        endpoint->stop(false);
    }
};


#endif
