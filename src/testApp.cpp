#include "testApp.h"

bool doRender = false;
//--------------------------------------------------------------
void testApp::setup(){
	//glDisable(GL_TEXTURE_2D);
	//ofDisableArbTex();
	m_flc = new FastLaplaceComper();
	ofImage imageSource;
	imageSource.loadImage("mona_ginerva.jpg");
	cvColorImage.allocate(imageSource.width, imageSource.height);
	cvColorImage.setFromPixels(imageSource.getPixelsRef());
	cvColorImage.resize(imageSource.width/3, imageSource.height/3);
	cvColorImage.flagImageChanged();
	IplImage *img_  = cvColorImage.getCvImage();
	img = cvCreateImage(cvSize(img_->width/3, img_->height/3), IPL_DEPTH_8U, 3);
    cvResize(img_, img);
    cvFlip(img);
    m_flc->SetSourceImage(img);
    m_flc->SetTargetImage(img);
	m_doInterp = true;
	m_drawmesh = true;
	m_flc->drawMesh(true);
	hasMadeSelection = false;
	cvColorImageOutput.allocate(img_->width/3, img_->height/3);
}

//--------------------------------------------------------------
void testApp::update(){
	cvColorImageOutput = img;
}

//--------------------------------------------------------------
void testApp::draw(){
	if(hasMadeSelection)
	{
		glClearColor(0,0,0,1);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		//glError();
		
		ofPushMatrix();
		
				m_flc->Render();
		
		ofPopMatrix();

	}else {
		/*glDisable(GL_TEXTURE_2D);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, ofGetWidth(), ofGetHeight(), 0, -1, 1);
        glViewport(0, 0, ofGetWidth(), ofGetHeight());
		
        glColor3f(1,1,1);
        glBegin(GL_LINES);
		
			glVertex3f(m_selectlet1.x(), m_selectlet1.y(), 0);
			glVertex3f(m_selectlet2.x(), m_selectlet2.y(), 0);
        glEnd();
		
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glEnable(GL_TEXTURE_2D);*/
		cvColorImage.draw(0, 0);
		ofPushStyle();
		ofSetColor(255, 0, 0);
		ofBeginShape();
		
		ofNoFill();
		for(int i = 0; i<mousePoints.size(); i++)
		{
			ofVertex(mousePoints[i].x, mousePoints[i].y);
		}
		ofEndShape(true);
		ofPopStyle();
	}

	

	//cvColorImageOutput.draw(0, 0);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key)
{
	//cout << "key: " << key << endl;
	
	//358 Left
	int incrementStep = 10;
	switch (key) {
		case OF_KEY_UP:
		{
			for(int i = 0; i<mousePoints.size(); i++)
			{
				mousePoints[i].y-=incrementStep;
			}
			break;
		}
		case OF_KEY_RIGHT:
		{
			for(int i = 0; i<mousePoints.size(); i++)
			{
				mousePoints[i].x+=incrementStep;
			}
			break;
		}
		case OF_KEY_DOWN:
		{
			for(int i = 0; i<mousePoints.size(); i++)
			{
				mousePoints[i].y+=incrementStep;
			}
			break;
		}
		case OF_KEY_LEFT:
		{
			for(int i = 0; i<mousePoints.size(); i++)
			{
				mousePoints[i].x-=incrementStep;
			}
			if (hasMadeSelection)
			{
				m_flc->SetTransform(0, pt_t(), 1, pt_t(20, 20), false);
				//m_flc->ComputeInterpolant(2);
				/*for(int i = 0; i < poly.size(); i++)
				{
					poly[i].x-=incrementStep;
					cout << "poly[i].x: " << poly[i].x << endl;
					//.push_back( pt_t(mousePoints[i].x, mousePoints[i].y) );
				}*/
			}
			break;
		}
		case OF_KEY_RETURN:
		{
			

			for(int i = 0; i < mousePoints.size(); i++)
			{
				poly.push_back( pt_t(mousePoints[i].x, mousePoints[i].y) );
			}
			m_flc->SetSourcePoly(&poly[0], (int)poly.size());
			hasMadeSelection = true;
			cout << "Area of boundary: " <<  m_flc->Mesh()->areaOfBoundaryPoly() << endl;
			doRender = true;
			break;
		}
		case ' ':
		{
			m_flc->Clear();
			poly.clear();
			hasMadeSelection = false;
			break;
		}
		case 'm':
		{
			m_flc->drawMesh(m_drawmesh = ! m_drawmesh);
			break;
		}	
	

		default:
		{
		    break;
		}
			
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{
	if (hasMadeSelection)
	{
		m_flc->SetTransform(0, pt_t(), 1, pt_t(x, y), false);
	}else {
		mousePoints.push_back(ofPoint(x, y));
	}

	
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	mousePoints.clear();
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