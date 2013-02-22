#include <Ogre.h>
#include <aruco/aruco.h>

#include "OgreApp.h"
#include "OgreARAppLogic.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif


bool init(OgreARAppLogic* owner);
bool update(OgreARAppLogic* owner);


// ArUco variables
string TheInputVideo, TheInputCameraFile;
cv::VideoCapture TheVideoCapturer;
cv::Mat TheInputImage, TheInputImageUnd;
aruco::CameraParameters CameraParams, CameraParamsUnd;
aruco::MarkerDetector MDetector;
vector<aruco::Marker> TheMarkers;
float TheMarkerSize=1;


// Ogre scene variables
#define MAX_OBJECTS 7
Ogre::Entity* ogreEntity[MAX_OBJECTS][5];
Ogre::SceneNode* ogreNode[MAX_OBJECTS][5];
const float scale = 0.00675f;
Ogre::AnimationState *baseAnim[MAX_OBJECTS], *topAnim[MAX_OBJECTS];



bool init(OgreARAppLogic* owner)
{

	// read input video  
	if (TheInputVideo=="live") TheVideoCapturer.open(0);
	else TheVideoCapturer.open(TheInputVideo);
	if (!TheVideoCapturer.isOpened())
	{
		cerr<<"Could not open video"<<endl;
		return false;
	}

	// read intrinsic file
	try {
		CameraParams.readFromXMLFile(TheInputCameraFile);
	} catch (std::exception &ex) {
		cout<<ex.what()<<endl;
		return false;
	}

	// capture first frame
	TheVideoCapturer.grab();
	TheVideoCapturer.retrieve ( TheInputImage );
	cv::undistort(TheInputImage,TheInputImageUnd,CameraParams.CameraMatrix,CameraParams.Distorsion);

	CameraParamsUnd=CameraParams;
	CameraParamsUnd.Distorsion=cv::Mat::zeros(4,1,CV_32F);


	// configure Ogre Camera
	owner->mCamera->setProjectionType(Ogre::PT_ORTHOGRAPHIC);
	owner->mCamera->setNearClipDistance(0.01f);
	owner->mCamera->setFarClipDistance(50.0f);
	double pMatrix[16];
	CameraParamsUnd.OgreGetProjectionMatrix(CameraParamsUnd.CamSize,CameraParamsUnd.CamSize, pMatrix, 0.05,10, false);
	Ogre::Matrix4 PM(pMatrix[0], pMatrix[1], pMatrix[2] , pMatrix[3],
			pMatrix[4], pMatrix[5], pMatrix[6] , pMatrix[7],
			pMatrix[8], pMatrix[9], pMatrix[10], pMatrix[11],
			pMatrix[12], pMatrix[13], pMatrix[14], pMatrix[15]);

	owner->mCamera->setCustomProjectionMatrix(true, PM);
	owner->mCamera->setCustomViewMatrix(true, Ogre::Matrix4::IDENTITY);


	// This two lines has to be defined in order to update background image
	owner->setImageSize(TheInputImageUnd.cols, TheInputImageUnd.rows);
	owner->setImageBuffer(TheInputImageUnd.ptr<unsigned char>(0)); // TheInputImageUnd will be shown as background


	// Init Ogre scene
	for(unsigned int i=0; i<MAX_OBJECTS; i++) {
		std::stringstream SU; SU << "su"<<i;
		std::stringstream MAP; MAP << "map"<<i;
		std::stringstream D3; D3 << "3d"<<i;
		std::stringstream INFO; INFO << "info"<<i;
		std::stringstream MESH; MESH << "mesh"<<i;

		ogreEntity[i][0] = owner->mSceneMgr->createEntity(SU.str(), "superficieretiro.mesh");
		ogreEntity[i][1] = owner->mSceneMgr->createEntity(MAP.str(), "letrasmapa.mesh");
		ogreEntity[i][2] = owner->mSceneMgr->createEntity(D3.str(), "letras3d.mesh");
		ogreEntity[i][3] = owner->mSceneMgr->createEntity(INFO.str(), "letrasinfo.mesh");
		ogreEntity[i][4] = owner->mSceneMgr->createEntity(MESH.str(), "cube.mesh");


		for(unsigned int j=0; j<5; j++){
			ogreNode[i][j] = owner->mSceneMgr->getRootSceneNode()->createChildSceneNode();
			ogreNode[i][j]->attachObject(ogreEntity[i][j]);
			ogreNode[i][j]->setScale(scale, scale, scale);
			if(j<4)
				ogreNode[i][j]->scale(12,12,12);
			ogreNode[i][j]->setVisible(false);
		}


		/*ogreEntity[i][1] = owner->mSceneMgr->createEntity(EN.str(), "letrasmapa.mesh");
		  ogreNode[i][1] = owner->mSceneMgr->getRootSceneNode()->createChildSceneNode();
		  ogreNode[i][1]->attachObject(ogreEntity[i]);
		  ogreNode[i][1]->setScale(scale, scale, scale);
		  ogreNode[i][1]->setVisible(false);

		  Init animation
		  ogreEntity[i][0]->getSkeleton()->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);
		  baseAnim[i] = ogreEntity[i][0]->getAnimationState("RunBase");
		  topAnim[i] = ogreEntity[i][0]->getAnimationState("Dance");
		  baseAnim[i]->setLoop(true);
		  topAnim[i]->setLoop(true);
		  baseAnim[i]->setEnabled(true);
		  topAnim[i]->setEnabled(true);*/
	}   

	//  correct initialization
	return true;


}

bool update(OgreARAppLogic* owner)
{

	// capture a frame
	if (TheVideoCapturer.grab())//¿Hay alguna imagen para procesar?
	{

		// undistort image
		TheVideoCapturer.retrieve ( TheInputImage );
		cv::undistort(TheInputImage,TheInputImageUnd,CameraParams.CameraMatrix,CameraParams.Distorsion);

		// detect markers
		MDetector.detect(TheInputImageUnd,TheMarkers,CameraParamsUnd,TheMarkerSize);

		// set object poses
		for(unsigned int i=0; i<MAX_OBJECTS; i++) {

			std::stringstream d3path; d3path << "../mesh/3d/";
			std::stringstream mapath; mapath << "../mesh/map/";
			std::stringstream infopath; infopath << "../mesh/info/";

			if(i<TheMarkers.size()) { 
				double position[3], orientation[4];
				TheMarkers[i].OgreGetPoseParameters(position, orientation);

				double yaw = atan2(2*(orientation[0]*orientation[3]+orientation[2]*orientation[1]),1-2*(pow(orientation[2],2)+pow(orientation[3],2))); 

				cout << "YAW " << yaw << endl;

				for (unsigned int j=0; j<5; j++){
					ogreNode[i][j]->setVisible(true);
					ogreNode[i][j]->setPosition( position[0], position[1], position[2]);
					ogreNode[i][j]->setOrientation( orientation[0], orientation[1], orientation[2], orientation[3]); 

					// Update animation and correct position
					//baseAnim[i]->addTime(0.08);
					//topAnim[i]->addTime(0.08);

					Ogre::Real offsety = ogreEntity[i][j]->getBoundingBox().getHalfSize().y;
					Ogre::Real offsetx = ogreEntity[i][j]->getBoundingBox().getHalfSize().x;


					switch(j){
						case 1:
							//ogreNode[i][j]->yaw(Ogre::Degree(90));
							ogreNode[i][j]->translate(-0.25,+offsety*scale,0.9,Ogre::Node::TS_LOCAL);
							break;
						case 2:
							ogreNode[i][j]->yaw(Ogre::Degree(90));
							ogreNode[i][j]->translate(-0.21,+offsety*scale,1.2,Ogre::Node::TS_LOCAL);
							break;
						case 3:
							ogreNode[i][j]->yaw(Ogre::Degree(-90));
							ogreNode[i][j]->translate(-0.24,+offsety*scale,1.2,Ogre::Node::TS_LOCAL);
							break;
						case 4:
							if(-0.79>yaw && yaw>-2.36){
								//3d
								cout << "3D!!!!" << endl;
							}else if(2.36<yaw || yaw<-2.36){
								//mapa
								cout << "MAPA!!!!" << endl;
							}else if(0.79<yaw && yaw<2.36){
								//info
								cout << "INFO!!!!" << endl;
							}else if(0.79>yaw && yaw>-0.79){
								//nada
								cout << "NADA!!!!" << endl;
							}

							ogreNode[i][j]->yaw(Ogre::Degree(90));
							ogreNode[i][j]->translate(0,(+offsety*scale)+0.1,0,Ogre::Node::TS_LOCAL);
							break;
						default:
							ogreNode[i][j]->yaw(Ogre::Degree(90));
							ogreNode[i][j]->translate(0,+offsety*scale,0,Ogre::Node::TS_LOCAL);
							break;
					}

				}

			}else{ 
				for (unsigned int j=0; j<5; j++){
					ogreNode[i][j]->setVisible(false);
				}
			}
		}

		return true;
	}
	else return false;

}


void usage()
{
	cout<<" This program test Ogre version of ArUco (single version) \n\n";
	cout<<" Usage <video.avi>|live <camera.yml> <markersize>"<<endl;
	cout<<" <video.avi>|live: specifies a input video file. Use 'live' to capture from camera"<<endl;
	cout<<" <camera.yml>: camera calibration file"<<endl;
	cout<<" <markersize>: in meters "<<endl;
}


bool readArguments ( int argc,char **argv )
{
	if (argc!=4) return false;
	TheInputVideo=argv[1];
	TheInputCameraFile=argv[2];
	TheMarkerSize=atof(argv[3]);
	return true;
}


#ifdef __cplusplus
extern "C" {
#endif


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
		int main(int argc, char *argv[])
#endif
		{
			try
			{
				if (!readArguments ( argc,argv )) {
					usage();
					return false;
				}
				OgreApp app;
				OgreARAppLogic appLogic;
				appLogic.setUserFunctions(init, update);
				app.setAppLogic(&appLogic);
				app.run();
			}
			catch (Ogre::Exception& e)
			{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
				MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
				std::cerr << "An exception has occured: " <<
					e.getFullDescription().c_str() << std::endl;
#endif
			}

			return 0;
		}

#ifdef __cplusplus
}
#endif


