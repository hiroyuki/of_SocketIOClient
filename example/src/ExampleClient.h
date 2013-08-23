//
//  ExampleClient.h
//  example
//
//  Created by 堀 宏行 on 2013/08/23.
//
//

#ifndef example_ExampleClient_h
#define example_ExampleClient_h
#include "ofxSocketIOClient.h"
#include "SocketIOEvents.h"

class ExampleClient : public ofxSocketIOClient<SocketIOEvents>
{
public:
    
    void connect( string url )
    {
        handler->bind_event("a message", &SocketIOEvents::example);
        handler->bind_event("item", &SocketIOEvents::example);
//        endpoint->elog().set_level(websocketpp::log::elevel::RERROR);
//        endpoint->elog().set_level(websocketpp::log::elevel::FATAL);
//        endpoint->elog().set_level(websocketpp::log::elevel::WARN);
        endpoint->alog().set_level(websocketpp::log::alevel::ALL);
        ofxSocketIOClient::connect( url );
    }
    
    void threadedFunction()
    {
        try
        {
            // Wait for a sec before sending stuff
            while (!handler->connected()) {
                ofSleepMillis(1);
            }
            // After connecting, send a connect message if using an endpoint
            handler->connect_endpoint("/chat");
            // Then to connect to another endpoint with the same socket, we call connect again.
            handler->connect_endpoint("/news");
            handler->emit("test", "hello!");

        }
        catch (std::exception& e)
        {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
    }
};


#endif
