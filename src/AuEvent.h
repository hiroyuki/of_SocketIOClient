//
//  AuEvent.h
//  socketIO
//
//  Created by 堀 宏行 on 2013/06/05.
//
//

#ifndef socketIO_AuEvent_h
#define socketIO_AuEvent_h

#include "ofMain.h"
#include <time.h>

class IO_ConnectionEventData
{
public:
//    IO_ConnectionEventData(){};
//    IO_ConnectionEventData( const IO_ConnectionEventData& dt)
//    {
//        establised = dt.establised;
//    }
    bool establised = false;
};


class IO_SwipeEventData
{
public:
    string usertoken;
    int count;
};

class IO_TapEventData
{
public:
    string usertoken;
    int count;
};

class IO_ModeChangeEventData
{
public:
    string mode;
};

class IO_SessionStartEventData
{
public:
    string sessionIndex;
    long startTime;
    int ms;
};

class AuEvents
{
public:
    ofEvent<IO_ConnectionEventData> connectionEvent;
    ofEvent<IO_SwipeEventData> swipeEvent;
    ofEvent<IO_TapEventData> tapEvent;
    ofEvent<IO_ModeChangeEventData> modeEvent;
    ofEvent<IO_SessionStartEventData> sessionEvent;
    
    
    static AuEvents& GetInstance()
    {
        static AuEvents instance;
        return instance;
    }
    
protected:
    AuEvents(){}
    ~AuEvents(){}
};


#endif
