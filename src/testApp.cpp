#include "testApp.h"

void testApp::connected( IO_ConnectionEventData& data)
{
    cout << "_connected" << endl;
}

void testApp::modeChanged( IO_ModeChangeEventData& data)
{
    cout << data.mode << endl;
    modeChnagedStrings.push_back( ofGetTimestampString() + " " + data.mode );
    if ( modeChnagedStrings.size() > 50) modeChnagedStrings.pop_front();
}

void testApp::sessionStart( IO_SessionStartEventData& data)
{
    tm* time = localtime(&data.startTime);
    printf("session(%s) start at %d/%d/%d %d:%d:%d.%d\n", data.sessionIndex.c_str(), time->tm_year + 1900, time->tm_mon + 1, time->tm_mday + 1, time->tm_hour, time->tm_min, time->tm_sec, data.ms);
    char str[100];
    sprintf(str, "session(%s) start at %d/%d/%d %d:%d:%d.%d\n", data.sessionIndex.c_str(), time->tm_year + 1900, time->tm_mon + 1, time->tm_mday + 1, time->tm_hour, time->tm_min, time->tm_sec, data.ms);
    sessionStartStrings.push_back( ofGetTimestampString() + " " +  str);
    if ( sessionStartStrings.size() > 50) sessionStartStrings.pop_front();
}

void testApp::taped( IO_TapEventData& data)
{
    cout << "tapped [" << data.usertoken << "] count: " << data.count << endl;
    tapStrings.push_back("tapped [" + ofToString(data.usertoken) + "] count: " + ofToString(data.count));
    if ( tapStrings.size() > 50) tapStrings.pop_front();
}

void testApp::swiped( IO_SwipeEventData& data)
{
    cout << "swiped [" << data.usertoken << "] count: " << data.count << endl;
    swipeStrings.push_back("swiped [" + ofToString(data.usertoken) + "] count: " + ofToString(data.count));
    if ( swipeStrings.size() > 50) swipeStrings.pop_front();
}

//--------------------------------------------------------------
void testApp::setup(){
    Config::GetInstance().setup();
    messageThread.setup();
	loading=false;
    ofSetLogLevel(OF_LOG_VERBOSE);
    AuEvents& event =AuEvents::GetInstance();
    ofAddListener(event.connectionEvent, this, &testApp::connected);
    ofAddListener(event.modeEvent, this, &testApp::modeChanged);
    ofAddListener(event.sessionEvent, this, &testApp::sessionStart);
    ofAddListener(event.tapEvent, this, &testApp::taped);
    ofAddListener(event.swipeEvent, this, &testApp::swiped);
}

//--------------------------------------------------------------
void testApp::update(){
    while( Config::GetInstance().recv.hasWaitingMessages())
    {
        ofxOscMessage m;
        Config::GetInstance().recv.getNextMessage( &m );
        if ( m.getAddress() == "/connect")
        {
            if ( !messageThread.isThreadRunning() )
            {
                messageThread.start();
            }
        }
        if ( m.getAddress() == "/close" )
        {
            if ( messageThread.isThreadRunning())
            {
                messageThread.keepThread = false;
                messageThread.stop();
            }
        }
        if ( m.getAddress() == "/music/send" )
        {
            
            int no = m.getArgAsInt32(0);
            int ms = m.getArgAsInt32(1);
            if ( messageThread.isThreadRunning())
            {
                messageThread.addSendSession(no, ms);
            }
        }
        if ( m.getAddress() == "/graphic/send" )
        {
            int no = m.getArgAsInt32(0);
            int ms = m.getArgAsInt32(1);
            if ( messageThread.isThreadRunning())
            {
                messageThread.addSendSession(no, ms);
            }
        }
        if ( m.getAddress() == "/download/photo" )
        {
            photoAsync.start();
            img.clear();
//            loading =true;
        }
    }
    
    if ( photoAsync.parsed && loadCounter < photoAsync.userDatas.size())
    {
        if ( !photoAsync.userDatas[loadCounter].downloadStarted )
        {
            photoAsync.userDatas[loadCounter].downloadStarted = true;
            cout << loadCounter << " " << photoAsync.userDatas[loadCounter].imageUrl << endl;
            asyncLoader.loadFromDisk(img, photoAsync.userDatas[loadCounter].imageUrl);
            //ofLoadURLAsync(photoAsync.userDatas[loadCounter].imageUrl,photoAsync.userDatas[loadCounter].userToken);
            downloadTgtUserToken = photoAsync.userDatas[loadCounter].userToken;
        }
    }
    if ( img.isAllocated())
    {
        img.saveImage(photoAsync.userDatas[loadCounter].filePath);
        img.clear();
        loadCounter++;
    }
}

//--------------------------------------------------------------
void testApp::draw(){
    ofBackground(0, 0, 0);
    if ( messageThread.isThreadRunning())
    {
        ofSetColor(ofColor::red);
        ofRect(10, 10, 20, 20);
        ofSetColor(ofColor::white);
    }
    for ( int i = 0; i < modeChnagedStrings.size(); i++ )
    {
        ofDrawBitmapString(modeChnagedStrings[i], 10, i*13+15);
    }
    for ( int i = 0; i < sessionStartStrings.size(); i++ )
    {
        ofDrawBitmapString(sessionStartStrings[i], 300, i*13+15);
    }
    for ( int i = 0; i < swipeStrings.size(); i++ )
    {
        ofDrawBitmapString(swipeStrings[i], 850, i*13+15);
    }
    for ( int i = 0; i < tapStrings.size(); i++ )
    {
        ofDrawBitmapString(tapStrings[i], 1100, i*13+15);
    }
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}