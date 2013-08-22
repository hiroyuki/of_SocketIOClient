#pragma once

#include "ofMain.h"
#include "MessageManage.h"
#include "ofxOsc.h"
#include "Config.h"
#include "PhotoGetAsync.h"
#include "ofxThreadedImageLoader.h"

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
        void connected( IO_ConnectionEventData& data);
        void modeChanged( IO_ModeChangeEventData& data);
        void sessionStart( IO_SessionStartEventData& data);
        void taped( IO_TapEventData& data);
        void swiped( IO_SwipeEventData& data);
        MessageManage messageThread;
        PhotoGetAsync photoAsync;
        deque< string > sessionStartStrings;
        deque< string > tapStrings;
        deque< string > swipeStrings;
        deque< string > modeChnagedStrings;
        
        ofImage img;
        ofxThreadedImageLoader asyncLoader;
        bool loading;
        int loadCounter = 0;
        string downloadTgtUserToken;
    
};
