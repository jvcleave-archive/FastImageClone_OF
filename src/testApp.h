#pragma once

#include "ofMain.h"
#include "gts.h"
#include "FastLaplaceComper.h"
#include "ofxOpenCv.h"
//#include "Pt.h"


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
	
		FastLaplaceComper *m_flc;
		ofxCvColorImage cvColorImage;
		ofxCvColorImage cvColorImageOutput;
		bool m_doInterp;
		bool m_drawmesh;
		//bool isSelecting = false;
		bool hasMadeSelection;
		vector<ofPoint> mousePoints;
		vector<pt_t> poly;
		IplImage *img;
};
