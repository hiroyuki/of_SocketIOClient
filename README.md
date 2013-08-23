#ofxSokectIOClient

**openframeworks** addon of **socketIO** client worked at **OSX 10.8**

using [Sokect.IO Client++](https://github.com/ebshimizu/socket.io-clientpp),  [websocket++](https://github.com/zaphoyd/websocketpp) ,and [rapidjson](http://code.google.com/p/rapidjson/).

This library is able to connect to a [Socket.IO](https://github.com/LearnBoost/socket.io) server, and then send and receive messages.

##about installing into existing project

* drag "lib" and "src" folder into Project Navigator in Xcode
* header search path
../../../addons/ofxSocketIOClient/lib/websocketpp/src
../../../addons/ofxSocketIOClient/include

##how to run example
1. install [socketIO](http://socket.io/#how-to-use)
2. run test.js ( : example/test.js ) from terminal.
3. build and run example
