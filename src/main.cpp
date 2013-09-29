#include <Ogre.h>
#include <aruco/aruco.h>

#include "OgreApp.h"
#include "OgreARAppLogic.h"
#include "SoundManager.h"

#include <map>
#include "Card.h"

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
float TheMarkerSize=0.05; //Tamaño de la marca


// Ogre scene variables
std::map<int,Card*> cards;

// Sound variables

SoundManager* soundMgr;
unsigned bgm;
unsigned currentSound;

//Load card config file
bool loadCards(){
	FILE* cardFile = fopen("cards.cfg","r");
	if(cardFile==NULL){
		return false;

	}
	unsigned cardID;
	char cardName[50];
	char line[100];
	int hasMap;
	while(1){
		if(fgets(line,100,cardFile)==NULL){
			break;
		}
		std::cout<<line;
		sscanf(line, "%s %d %d",cardName,&cardID,&hasMap);
		if(cardName[0]!='#'){
			Card* card = new Card(cardID,string(cardName),bool(hasMap));
			cards[cardID]=card;
		}
	}
	fclose(cardFile);
	return true;
}

bool init(OgreARAppLogic* owner)
{

	// read input video  
	TheVideoCapturer.open(0);
	if (!TheVideoCapturer.isOpened()){
		cerr<<"Could not open video"<<endl;
		return false;
	}

	// read intrinsic file
	try {
		CameraParams.readFromXMLFile("intrinsics.yml");
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

	//SOUNDS
	soundMgr = SoundManager::createManager();
	std::cout << soundMgr->listAvailableDevices();
	std::cout<<soundMgr->init();
	soundMgr->setAudioPath("../sounds/");

	//BGM
	soundMgr->loadAudio("Julio D' Escrivan - bebe.ogg",&bgm,true);

	loadCards();
	// Init Ogre scene
	map<int,Card*>::iterator it;
	for(it = cards.begin();it!=cards.end();it++){
		Card* card = it->second;
		int id = card->id;
		std::string name = card->name;
		std::stringstream tmpstr;
		std::stringstream tmpstr2;

		tmpstr << "surface"<<id;
		card->surface = owner->mSceneMgr->createEntity(tmpstr.str(),"common/superficieretiro.mesh");

		tmpstr.str("");
		tmpstr << "text3d"<<id;
		card->text3D = owner->mSceneMgr->createEntity(tmpstr.str(),"common/letras3d.mesh");

		tmpstr.str("");
		tmpstr << "textInfo"<<id;
		card->textInfo = owner->mSceneMgr->createEntity(tmpstr.str(),"common/letrasinfo.mesh");

		if(card->hasMap){
		
            tmpstr.str("");
		    tmpstr << "textmap"<<id;
		    card->textMap = owner->mSceneMgr->createEntity(tmpstr.str(),"common/letrasmapa.mesh");

            tmpstr.str("");
            tmpstr << "textmap2"<<id;
            card->textMap2 = owner->mSceneMgr->createEntity(tmpstr.str(),"common/letrasmapa.mesh");

		    tmpstr.str("");
		    tmpstr << "textInfo2"<<id;
		    card->textInfo2 = owner->mSceneMgr->createEntity(tmpstr.str(),"common/letrasinfo.mesh");
        
			tmpstr.str("");
			tmpstr2.str("");
			tmpstr << "map"<<id;
			tmpstr2 << name<<"/"<<name<<"_mapa.mesh";
			card->map = owner->mSceneMgr->createEntity(tmpstr.str(),tmpstr2.str());
		}

		tmpstr.str("");
		tmpstr2.str("");
		tmpstr << "model"<<id;
	    tmpstr2 << name<<"/"<<name<<".mesh";
		card->model = owner->mSceneMgr->createEntity(tmpstr.str(),tmpstr2.str());

		//Assigning Ogrenode
		card->node = owner->mSceneMgr->getRootSceneNode()->createChildSceneNode();
		card->node->setScale(scale,scale,scale);
		
		card->attachObjects();
		card->node->setVisible(false);

		//Card especific sounds
		tmpstr.str("");
		tmpstr << name<<".ogg";
		if(!soundMgr->loadAudio(tmpstr.str(), &card->soundInfoID, false)){
			soundMgr->checkALError("load");
		}
		if(card->hasMap){
			tmpstr.str("");
			tmpstr << name<<"_mapa.ogg";
			cout <<tmpstr.str();
			if(!soundMgr->loadAudio(tmpstr.str(), &card->soundMapID, false)){
				soundMgr->checkALError("load");
			}
		}

	}

	//  correct initialization
	return true;
}

bool update(OgreARAppLogic* owner){

	soundMgr->playAudio(bgm,false);
	// capture a frame
	if (TheVideoCapturer.grab()){//Â¿Hay alguna imagen para procesar?
		// undistort image
		TheVideoCapturer.retrieve ( TheInputImage );
		cv::undistort(TheInputImage,TheInputImageUnd,CameraParams.CameraMatrix,CameraParams.Distorsion);

		// detect markers
		MDetector.detect(TheInputImageUnd,TheMarkers,CameraParamsUnd,TheMarkerSize);

		map<int,Card*>::iterator it;
		for(it=cards.begin();it!=cards.end();it++){
			Card* c = it->second;
			if(c!=NULL){
				c->node->setVisible(false);
			}
		}


		// set object poses
		for(unsigned int i=0; i<TheMarkers.size(); i++) {
			double position[3], orientation[4];
			TheMarkers[i].OgreGetPoseParameters(position, orientation);

			//Calcular el angulo de rotacion
			double yaw = atan2(2*(orientation[0]*orientation[3]+orientation[2]*orientation[1]),1-2*(pow(orientation[2],2)+pow(orientation[3],2))); 
			Card* card = cards[TheMarkers[i].id];
			if(card==NULL) continue;

			card->node->setVisible(true);
			card->node->setPosition(position[0],position[1],position[2]);
			card->node->setOrientation( orientation[0], orientation[1], orientation[2], orientation[3]); 

			if(-0.79>yaw && yaw>-2.36){
				//3d
				cout << "3D!!!!" << endl;
				if(card->hasMap){
					card->model->setVisible(true);
					card->map->setVisible(false);
				}
			}else if(2.36<yaw || yaw<-2.36){
				//3D info
				cout << "INFO!!!!" << endl;
				if(card->hasMap){
					card->model->setVisible(true);
					card->map->setVisible(false);
				}
				if(currentSound!=card->soundInfoID){
					soundMgr->stopAudio(currentSound);
					soundMgr->playAudio(card->soundInfoID,true);
					currentSound = card->soundInfoID;
				}
			}else if(0.79<yaw && yaw<2.36){
				//mapa
				if(card->hasMap){
					card->map->setVisible(true);
					card->model->setVisible(false);
				}
				cout << "MAPA!!!!" << endl;
			}else if(0.79>yaw && yaw>-0.79){
				//mapa infonada
				cout << "MAPAINFO!!!!" << endl;
                if(card->hasMap){
					card->model->setVisible(false);
					card->map->setVisible(true);
				}

				if(currentSound!=card->soundMapID){
					soundMgr->stopAudio(currentSound);
					soundMgr->playAudio(card->soundMapID,true);
					currentSound = card->soundMapID;
				}
			}
		}
		return true;
	}else{
		return false;
	}

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
				OgreApp app;
				OgreARAppLogic appLogic;
				appLogic.setUserFunctions(init, update);
				app.setAppLogic(&appLogic);
				app.run();
				delete soundMgr;
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


