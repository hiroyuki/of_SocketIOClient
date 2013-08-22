//
//  Config.h
//  socketIO
//
//  Created by 堀 宏行 on 2013/06/09.
//
//

#ifndef socketIO_Config_h
#define socketIO_Config_h
#include "ofxXmlSettings.h"
#include "ofxOsc.h"

class Config
{
public:
    static Config& GetInstance()
    {
        static Config instance;
        return instance;
    }
    
    void setup()
    {
        xml.loadFile( "setting/setting.xml");
        serverURL = xml.getValue( "data:server", "");
        string maxIp = xml.getValue( "data:controlerIP" , "192.168.11.1" );
        int maxPort = xml.getValue( "data:controlerPort", 10002);
        photoGetUrl = xml.getValue( "data:imgGetUrl", "");
        imageSavePath = xml.getValue( "data:imgSavePath", "");
        sender.setup( maxIp, maxPort);
        recv.setup( xml.getValue( "data:myPort", 10003));
    }
    
    void sendIsConnect( int val )
    {
        ofxOscMessage m;
        m.setAddress( "/connect");
        m.addIntArg(val);
        sender.sendMessage( m );
    }
    
    ofxXmlSettings xml;
    string serverURL;
    string photoGetUrl;
    ofxOscSender sender;
    ofxOscReceiver recv;
    string imageSavePath;
    
protected:
    Config(){}
    ~Config(){}
};

#endif
