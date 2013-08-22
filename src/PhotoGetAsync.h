//
//  PhotoGetAsync.h
//  socketIO
//
//  Created by 堀 宏行 on 2013/06/10.
//
//

#ifndef socketIO_PhotoGetAsync_h
#define socketIO_PhotoGetAsync_h
#include "Config.h"
#include "ofxJSONElement.h"
class UserData
{
public:
    string filePath;
    string userToken;
    string imageUrl;
    string miniUrl;
    bool downloadStarted = false;
    
    UserData()
    {
    }
    
    void setup()
    {
        filePath = Config::GetInstance().imageSavePath + userToken + ".png";
    }
    
    bool isDownloaded()
    {
        return ofDirectory(filePath).exists();
    }

};


class PhotoGetAsync : public ofThread
{
public:
    bool validThread = true;
    ofxJSONElement json;
    bool parsed = false;
    vector< UserData> userDatas;
    int startedMs;
    void start()
    {
        startThread( false, true );
    }
    
    void stop()
    {
//        stopThread();
    }
    
    void threadedFunction()
    {
        string url = Config::GetInstance().photoGetUrl;
        // Now parse the JSON
        json.open(url);
        {
            ofxJSONElement el = json["entries"];
            int n = 0;
            startedMs = ofGetElapsedTimeMillis();
            int counter = 0;
            for ( int i = 0; i < el.size(); i++ )
            {
                UserData userData;
                userData.userToken = el[i]["user_token"].asString();
                userData.imageUrl = el[i]["icon_url"].asString();
                userData.miniUrl = el[i]["s_icon_url"].asString();
                userData.setup();
                userDatas.push_back( userData );
            }
            cout << "parse finish!" << endl;
        }
        parsed = true;
    
    }
};
#endif
