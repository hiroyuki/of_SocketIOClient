//
//  SocketIOEvents.h
//  example
//
//  Created by 堀 宏行 on 2013/08/22.
//
//

#ifndef example_SocketIOEvents_h
#define example_SocketIOEvents_h
#include "ofxSocketIOClient.h"
class SocketIOEvents
{
public:
    
    virtual void example( const Value& args)
    {
        ofLog() << ">>>> [example]:";
        for (SizeType i = 0; i < args.Size(); i++)
        {
            for (Value::ConstMemberIterator itr = args[i].MemberBegin(); itr != args[i].MemberEnd(); ++itr)
            {
                cout << "name:" << (string)itr->name.GetString();
                cout << " -> value:" << itr->value.GetString() << endl;
            }
        }
    }
};


#endif
