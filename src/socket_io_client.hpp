/* socket_io_client.hpp
 * Evan Shimizu, June 2012
 * websocket++ handler implementing (most of) the socket.io protocol.
 * https://github.com/LearnBoost/socket.io-spec
 *
 * This implementation uses the rapidjson library.
 * See examples at https://github.com/kennytm/rapidjson.
 */
/* socket_io_client.cpp
 * Evan Shimizu, June 2012
 * websocket++ handler implementing the socket.io protocol.
 * https://github.com/LearnBoost/socket.io-spec
 *
 * This implementation uses the rapidjson library.
 * See examples at https://github.com/kennytm/rapidjson.
 */

#ifndef SOCKET_IO_CLIENT_HPP
#define SOCKET_IO_CLIENT_HPP

#pragma warning(disable:4355) // C4355: this used in base member initializer list

#include <boost/shared_ptr.hpp>
#include <boost/regex.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <rapidjson/document.h>		// rapidjson's DOM-style API
#include <rapidjson/prettywriter.h>	// for stringify JSON
#include <rapidjson/filestream.h>
#include <rapidjson/stringwriter.h>

#include <roles/client.hpp>
#include <websocketpp.hpp>

#include "ofMain.h"


#include <map>
#include <string>
#include <queue>

#define JSON_BUFFER_SIZE 20000
// Comment this out to disable handshake logging to stdout
#define LOG(x) std::cout << x

using websocketpp::client;
using namespace rapidjson;

namespace socketio {
    
    // Class for event callbacks.
    // Class is automatically created  on the stack to handle calling the function when an event callback is triggered.
    // Class is broken out from the main handler class to allow for easier editing of functions
    // and modularity of code.
    // If you don't want to use this class, you can modify the callback type to whatever suits your purpose.

    template < class ofxSocket_IO_Events>
    class socketio_client_handler : public client::handler {
    public:
        socketio_client_handler() :
        m_heartbeatActive(false),
        m_connected(false)
        { }
        
        ~socketio_client_handler() {
            if (m_con)
                m_con->close(websocketpp::close::status::GOING_AWAY, "Client Initiated Disconnect");
        }

        
        // Callbacks
        // Client Functions - such as send, etc.
        
        // Function pointer to a event handler.
        // Args is an array, managed by rapidjson, and could be null
        // Can change to whatever signature you want, just make sure to change the call in on_socketio_event too.
#ifndef BOOST_NO_CXX11_HDR_FUNCTIONAL
        // If you're using C++11 and have the proper functional header in the standard lib, we'll use that
        typedef std::function<void (ofxSocket_IO_Events&, const Value&)> eventFunc;
#else
        // Otherwise we'll let boost fill in the gaps
        typedef boost::function<void (ofxSocket_IO_Events&, const Value&)> eventFunc;
#endif
        
        void on_fail(connection_ptr con)
        {
            stop_heartbeat();
            m_con = connection_ptr();
            m_connected = false;
            
            LOG("Connection failed." << std::endl);
        }
        
        void on_open(connection_ptr con)
        {
            m_con = con;
            // Create the heartbeat timer and use the same io_service as the main event loop.
#ifndef BOOST_NO_CXX11_SMART_PTR
            m_heartbeatTimer = std::unique_ptr<boost::asio::deadline_timer>(new boost::asio::deadline_timer(con->get_io_service(), boost::posix_time::seconds(0)));
#else
            m_heartbeatTimer = boost::shared_ptr<boost::asio::deadline_timer>(new boost::asio::deadline_timer(con->get_io_service(), boost::posix_time::seconds(0)));
#endif
            start_heartbeat();
            m_connected = true;
            
            LOG("Connected." << std::endl);
        }

        void on_close(connection_ptr con)
        {
            stop_heartbeat();
            m_connected = false;
            m_con = connection_ptr();
            
            LOG("Client Disconnected." << std::endl);
        }

        void on_message(connection_ptr con, message_ptr msg)
        {
            // Parse the incoming message according to socket.IO rules
            parse_message(msg->get_payload());
        }

        
        // Performs a socket.IO handshake
        // https://github.com/LearnBoost/socket.io-spec
        // param - url takes a ws:// address with port number
        // param - socketIoResource is the resource where the server is listening. Defaults to "/socket.io".
        // Returns a socket.IO url for performing the actual connection.
        std::string perform_handshake(std::string url, std::string socketIoResource = "/socket.io")
        {
            // Log currently not accessible from this function, outputting to std::cout
            LOG("Parsing websocket uri..." << std::endl);
            websocketpp::uri uo(url);
            m_resource = uo.get_resource();
            
            // Declare boost io_service
            boost::asio::io_service io_service;
            
            LOG("Connecting to Server..." << std::endl);
            
            // Resolve query
            tcp::resolver r(io_service);
            tcp::resolver::query q(uo.get_host(), uo.get_port_str());
            tcp::socket socket(io_service);
            boost::asio::connect(socket, r.resolve(q));
            
            // Form initial post request.
            boost::asio::streambuf request;
            std::ostream reqstream(&request);
            
            reqstream << "POST " << socketIoResource << "/1/ HTTP/1.0\r\n";
            reqstream << "Host: " << uo.get_host() << "\r\n";
            reqstream << "Accept: */*\r\n";
            reqstream << "Connection: close\r\n\r\n";
            
            LOG("Sending Handshake Post Request..." << std::endl);
            
            // Write request.
            boost::asio::write(socket, request);
            
            // Receive response
            boost::asio::streambuf response;
            boost::asio::read_until(socket, response, "\r\n");
            
            // Parse response
            std::istream resp_stream(&response);
            
            // Extract HTTP version, status, and message.
            std::string httpver;
            unsigned int status;
            std::string status_msg;
            
            resp_stream >> httpver >> status;
            std::getline(resp_stream, status_msg);
            
            // Log response
            LOG("Received Response:" << std::endl);
            LOG(httpver << " " << status << std::endl);
            LOG(status_msg << std::endl);
            
            // Read response headers. Terminated with double newline.
            boost::asio::read_until(socket, response, "\r\n\r\n");
            
            // Log headers.
            std::string header;
            
            while (std::getline(resp_stream, header) && header[0] != '\r')
            {
                LOG(header << std::endl);
            }
            
            // Handle errors
            if (!resp_stream || httpver.substr(0, 5) != "HTTP/")
            {
                std::cerr << "Invalid HTTP protocol: " << httpver << std::endl;
                throw websocketpp::exception("Socket.IO Handshake: Invalid HTTP protocol: " + httpver, websocketpp::error::GENERIC);
            }
            switch (status)
            {
                case(200):
                    LOG("Server accepted connection." << std::endl);
                    break;
                case(401):
                case(503):
                    std::cerr << "Server rejected client connection" << std::endl;
                    throw websocketpp::exception("Socket.IO Handshake: Server rejected connection with code " + status, websocketpp::error::GENERIC);
                    break;
                default:
                    std::cerr << "Server returned unknown status code: " << status << std::endl;
                    throw websocketpp::exception("Socket.IO Handshake: Server responded with unknown code " + status, websocketpp::error::GENERIC);
                    break;
            }
            
            // Get the body components.
            std::string body;
            
            std::getline(resp_stream, body, '\0');
            boost::cmatch matches;
            const boost::regex expression("([0-9a-zA-Z_-]*):([0-9]*):([0-9]*):(.*)");
            
            if (boost::regex_match(body.c_str(), matches, expression))
            {
                m_sid = matches[1];
                
                m_heartbeatTimeout = atoi(matches[2].str().c_str());
                if (m_heartbeatTimeout <= 0) m_heartbeatTimeout = 0;
                
                m_disconnectTimeout = atoi(matches[3].str().c_str());
                
                m_transports = matches[4];
                if (m_transports.find("websocket") == std::string::npos)
                {
                    std::cerr << "Server does not support websocket transport: " << m_transports << std::endl;
                    websocketpp::exception("Socket.IO Handshake: Server does not support websocket transport", websocketpp::error::GENERIC);
                }
            }
            
            // Log socket.IO info
            LOG(std::endl << "Session ID: " << m_sid << std::endl);
            LOG("Heartbeat Timeout: " << m_heartbeatTimeout << std::endl);
            LOG("Disconnect Timeout: " << m_disconnectTimeout << std::endl);
            LOG("Allowed Transports: " << m_transports << std::endl);
            
            // Form the complete connection uri. Default transport method is websocket (since we are using websocketpp).
            // If secure websocket connection is desired, replace ws with wss.
            std::stringstream iouri;
            iouri << "ws://" << uo.get_host() << ":" << uo.get_port() << socketIoResource << "/1/websocket/" << m_sid;
            m_socketIoUri = iouri.str();
            return m_socketIoUri;
        }

            
        // Sends a plain string to the endpoint. No special formatting performed to the string.
        void send(const std::string &msg)
        {
            if (!m_con)
            {
                std::cerr << "Error: No active session" << std::endl;
                return;
            }
            
            m_con->alog()->at(websocketpp::log::alevel::DEVEL) << "Sent: " << msg << websocketpp::log::endl;
            m_con->send(msg);
        }

        // Allows user to send a custom socket.IO message
        void send(unsigned int type, std::string endpoint, std::string msg, unsigned int id = 0)
        {
            // Construct the message.
            // Format: [type]:[id]:[endpoint]:[msg]
            std::stringstream package;
            package << type << ":";
            if (id > 0) package << id;
            package << ":" << endpoint << ":" << msg;
            
            send(package.str());
        }
        
        // Signal connection to the desired endpoint. Allows the use of the endpoint once message is successfully sent.
        void connect_endpoint(std::string endpoint)
        {
            send("1::" + endpoint);
        }
            
        // Signal disconnect from specified endpoint.
        void disconnect_endpoint(std::string endpoint)
        {
            send("0::" + endpoint);
        }

        // Emulates the emit function from socketIO (type 5)
        void emit(std::string name, Document& args, std::string endpoint = "", unsigned int id = 0)
        {
            // Add the name to the data being sent.
            Value n;
            n.SetString(name.c_str(), name.length(), args.GetAllocator());
            args.AddMember("name", n, args.GetAllocator());
            
            // Stringify json
            std::ostringstream outStream;
            StreamWriter<std::ostringstream> writer(outStream);
            args.Accept(writer);
            
            // Extract the message from the stream and format it.
            std::string package(outStream.str());
            send(5, endpoint, package.substr(0, package.find('\0')), id);
        }
            
        void emit(std::string name, std::string arg0, std::string endpoint = "", unsigned int id = 0)
        {
            Document d;
            d.SetObject();
            Value args;
            args.SetArray();
            args.PushBack(arg0.c_str(), d.GetAllocator());
            d.AddMember("args", args, d.GetAllocator());
            
            emit(name, d, endpoint, id);
        }
            
        void emit(std::string name, std::vector< std::string> _args, std::string endpoint = "", unsigned int id = 0)
        {
            Document d;
            d.SetObject();
            Value args(kObjectType);
            //    args.SetObject();
            args.AddMember("session_index", _args[0].c_str(), d.GetAllocator());
            args.AddMember("started_at_since", ofToInt(_args[1]), d.GetAllocator());
            d.AddMember("args", args, d.GetAllocator());
            emit(name, d, endpoint, id);
        }

        
        // Sends a plain message (type 3)
        void message(std::string msg, std::string endpoint = "", unsigned int id = 0)
        {
            send(3, endpoint, msg, id);
        }
            
        // Sends a JSON message (type 4)
        void json_message(Document& json, std::string endpoint = "", unsigned int id = 0)
        {
            // Stringify json
            std::ostringstream outStream;
            StreamWriter<std::ostringstream> writer(outStream);
            json.Accept(writer);
            
            // Extract the message from the stream and format it.
            std::string package(outStream.str());
            send(4, endpoint, package.substr(0, package.find('\0')), id);
        }

        // Binds a function to a name. Function will be passed a a Value ref as the only argument.
        // If the function already exists, this function returns false. You must call unbind_event
        // on the name of the function first to re-bind a name.
        bool bind_event(std::string name, eventFunc func)
        {
            // If the name is already in use, don't replace it.
            if (m_events.count(name) == 0)
            {
                m_events[name] = func;
                return true;
            }
            
            return false;
        }
        
        // Removes the binding between event [name] and the function associated with it.
        bool unbind_event(std::string name)
        {
            // Delete from map if the key exists.
            if (m_events.count(name) > 0)
            {
                m_events.erase(m_events.find(name));
                return true;
            }
            return false;
        }
        
        // Closes the connection
        void close()
        {
            if (!m_con)
            {
                std::cerr << "Error: No active session" << std::endl;
                return;
            }
            
            send(3, "disconnect", "");
            m_con->close(websocketpp::close::status::GOING_AWAY, "");
        }
        
        // Heartbeat operations.
        void start_heartbeat()
        {
            // Heartbeat is already active so don't do anything.
            if (m_heartbeatActive) return;
            
            // Check valid heartbeat wait time.
            if (m_heartbeatTimeout > 0)
            {
                m_heartbeatTimer->expires_at(m_heartbeatTimer->expires_at() + boost::posix_time::seconds(m_heartbeatTimeout));
                m_heartbeatActive = true;
                m_heartbeatTimer->async_wait(boost::bind(&socketio_client_handler::heartbeat, this));
                
                m_con->alog()->at(websocketpp::log::alevel::DEVEL) << "Sending heartbeats. Timeout: " << m_heartbeatTimeout << websocketpp::log::endl;
            }
        }

        void stop_heartbeat()
        {
            // Timer is already stopped.
            if (!m_heartbeatActive) return;
            
            // Stop the heartbeats.
            m_heartbeatActive = false;
            m_heartbeatTimer->cancel();
            
            if (m_con)
                m_con->alog()->at(websocketpp::log::alevel::DEVEL) << "Stopped sending heartbeats." << websocketpp::log::endl;
        }
        
        std::string getSid() { return m_sid; }
        std::string getResource() { return m_resource; }
        bool connected() { return m_connected; }
    private:
        // Sends a heartbeat to the server.
        void send_heartbeat()
        {
            if (m_con)
            {
                m_con->send("2::");
                m_con->alog()->at(websocketpp::log::alevel::DEVEL) << "Sent Heartbeat" << websocketpp::log::endl;
            }
        }

        
        // Called when the heartbeat timer fires.
        void heartbeat()
        {
            send_heartbeat();
            
            m_heartbeatTimer->expires_at(m_heartbeatTimer->expires_at() + boost::posix_time::seconds(m_heartbeatTimeout));
            m_heartbeatTimer->async_wait(boost::bind(&socketio_client_handler::heartbeat, this));
        }

        
        // Parses a socket.IO message received
        void parse_message(const std::string &msg)
        {
            // Parse response according to socket.IO rules.
            // https://github.com/LearnBoost/socket.io-spec
            
            boost::cmatch matches;
            const boost::regex expression("([0-8]):([0-9]*):([^:]*)[:]?(.*)");
            
            if(boost::regex_match(msg.c_str(), matches, expression))
            {
                int type;
                int msgId;
                Document json;
                
                // Attempt to parse the first match as an int.
                std::stringstream convert(matches[1]);
                if (!(convert >> type)) type = -1;
                
                // Store second param for parsing as message id. Not every type has this, so if it's missing we just use 0 as the ID.
                std::stringstream convertId(matches[2]);
                if (!(convertId >> msgId)) msgId = 0;
                switch (type)
                {
                        // Disconnect
                    case (0):
                        m_con->alog()->at(websocketpp::log::alevel::DEVEL) << "Received message type 0 (Disconnect)" << websocketpp::log::endl;
                        close();
                        break;
                        // Connection Acknowledgement
                    case (1):
                        m_con->alog()->at(websocketpp::log::alevel::DEVEL) << "Received Message type 1 (Connect): " << msg << websocketpp::log::endl;
                        break;
                        // Heartbeat
                    case (2):
                        m_con->alog()->at(websocketpp::log::alevel::DEVEL) << "Received Message type 2 (Heartbeat)" << websocketpp::log::endl;
                        send_heartbeat();
                        break;
                        // Message
                    case (3):
                        m_con->alog()->at(websocketpp::log::alevel::DEVEL) << "Received Message type 3 (Message): " << msg << websocketpp::log::endl;
                        on_socketio_message(msgId, matches[3], matches[4]);
                        break;
                        // JSON Message
                    case (4):
                        m_con->alog()->at(websocketpp::log::alevel::DEVEL) << "Received Message type 4 (JSON Message): " << msg << websocketpp::log::endl;
                        // Parse JSON
                        if (json.Parse<0>(matches[4].str().c_str()).HasParseError())
                        {
                            m_con->elog()->at(websocketpp::log::elevel::WARN) << "JSON Parse Error: " << matches[4] << websocketpp::log::endl;
                            return;
                        }
                        on_socketio_json(msgId, matches[3], json);
                        break;
                        // Event
                    case (5):
                        m_con->alog()->at(websocketpp::log::alevel::DEVEL) << "Received Message type 5 (Event): " << msg << websocketpp::log::endl;
                        // Parse JSON
                        if (json.Parse<0>(matches[4].str().c_str()).HasParseError())
                        {
                            m_con->elog()->at(websocketpp::log::elevel::WARN) << "JSON Parse Error: " << matches[4] << websocketpp::log::endl;
                            return;
                        }
                        if (!json["name"].IsString())
                        {
                            m_con->elog()->at(websocketpp::log::elevel::WARN) << "Invalid socket.IO Event" << websocketpp::log::endl;
                            return;
                        }
                        on_socketio_event(msgId, matches[3], json["name"].GetString(), json["args"]);
                        break;
                        // Ack
                    case (6):
                        m_con->alog()->at(websocketpp::log::alevel::DEVEL) << "Received Message type 6 (ACK)" << websocketpp::log::endl;
                        on_socketio_ack(matches[4]);
                        break;
                        // Error
                    case (7):
                        m_con->alog()->at(websocketpp::log::alevel::DEVEL) << "Received Message type 7 (Error): " << msg << websocketpp::log::endl;
                        on_socketio_error(matches[3], matches[4].str().substr(0, matches[4].str().find("+")), matches[4].str().substr(matches[4].str().find("+")+1));
                        break;
                        // Noop
                    case (8):
                        m_con->alog()->at(websocketpp::log::alevel::DEVEL) << "Received Message type 8 (Noop)" << websocketpp::log::endl;
                        break;
                    default:
                        m_con->elog()->at(websocketpp::log::elevel::WARN) << "Invalid Socket.IO message type: " << type << websocketpp::log::endl;
                        break;
                }
            }
            else
            {
                m_con->elog()->at(websocketpp::log::elevel::WARN) << "Non-Socket.IO message: " << msg << websocketpp::log::endl;
            }
        }
        
        // Message Parsing callbacks.
        void on_socketio_message(int msgId, std::string msgEndpoint, std::string data)
        {
            m_con->alog()->at(websocketpp::log::alevel::DEVEL) << "Received message (" << msgId << ") " << data << websocketpp::log::endl;
        }

        void on_socketio_json(int msgId, std::string msgEndpoint, Document& json)
        {
            m_con->alog()->at(websocketpp::log::alevel::DEVEL) << "Received JSON Data (" << msgId << ")" << websocketpp::log::endl;
        }

        void on_socketio_event(int msgId, std::string msgEndpoint, std::string name, const Value& args)
        {
            m_con->alog()->at(websocketpp::log::alevel::DEVEL) << "Received event (" << msgId << ") " << websocketpp::log::endl;
            
            if (m_events.count(name) > 0)
            {
                
                ofxSocket_IO_Events events;
                m_events[name](events, args);
            }
            else m_con->elog()->at(websocketpp::log::elevel::WARN) << "No bound event with name: " << name << websocketpp::log::endl;
        }

        void on_socketio_ack(std::string data)
        {
            m_con->alog()->at(websocketpp::log::alevel::DEVEL) << "Received ACK: " << data << websocketpp::log::endl;
        }

        void on_socketio_error(std::string endppoint, std::string reason, std::string advice)
        {
            m_con->alog()->at(websocketpp::log::elevel::RERROR) << "Received Error: " << reason << " Advice: " << advice << websocketpp::log::endl;
        }
            
        // Connection pointer for client functions.
        connection_ptr m_con;
        
        // Socket.IO server settings
        std::string m_sid;
        unsigned int m_heartbeatTimeout;
        unsigned int m_disconnectTimeout;
        std::string m_socketIoUri;
        std::string m_resource;
        bool m_connected;
        
        // Currently we assume websocket as the transport, though you can find others in this string
        std::string m_transports;
        
        // Heartbeat variabes.
#ifndef BOOST_NO_CXX11_SMART_PTR
        // If you're using C++11 use the standar library smart pointer
        std::unique_ptr<boost::asio::deadline_timer> m_heartbeatTimer;
#else
        // Otherwise let boost provideo the smart pointer
        boost::shared_ptr<boost::asio::deadline_timer> m_heartbeatTimer;
#endif
        
        bool m_heartbeatActive;
        
        // Event bindings
        std::map<std::string, eventFunc> m_events;
    };
    
}

#endif // SOCKET_IO_CLIENT_HPP