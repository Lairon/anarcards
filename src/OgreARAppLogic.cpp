#include "OgreARAppLogic.h"
#include "OgreApp.h"
#include <Ogre.h>

using namespace Ogre;


// mWidth, mHeight, TheInputImage.ptr
OgreARAppLogic::OgreARAppLogic() : mApplication(0)
{
	// ogre
	mSceneMgr		= 0;
	mViewport		= 0;
	mCamera         = 0;
	mCameraNode     = 0;
	mObjectNode     = 0;

	mWidth = 0;
	mHeight = 0;
	mBuffer = 0;

	mOISListener.mParent = this;
}

OgreARAppLogic::~OgreARAppLogic()
{}

// preAppInit
bool OgreARAppLogic::preInit(const Ogre::StringVector &commandArgs)
{
	return true;
}

// postAppInit
bool OgreARAppLogic::init(void)
{	
	pause = false;
	
	//INIT OGRE
	createSceneManager();
	createViewport();
	createCamera();
	createScene();
	
	//INIT AR
	if( !initAR() ) return false;

	createCameraBackground();
	
	/*
	 *mApplication->getKeyboard()->setEventCallback(&mOISListener);
	 *mApplication->getMouse()->setEventCallback(&mOISListener);
	 */
	
	return true;
}

bool OgreARAppLogic::preUpdate(Ogre::Real deltaTime)
{
	return true;
}

bool OgreARAppLogic::update(Ogre::Real deltaTime)
{
	if (!pause) {
	  if( !userUpdate(this)) return false; // call user-defined funcions
	  
	  // update background image
	  if (!mTexture.isNull())
	  {
	      //Pedimos a ogre que actualice la imagen desde el PixelBox
	      Ogre::HardwarePixelBufferSharedPtr pixelBuffer = mTexture->getBuffer();
	      pixelBuffer->blitFromMemory(mPixelBox);
	  }	  	  
	}
	
	bool result = processInputs(deltaTime);
	return result;
}

void OgreARAppLogic::shutdown(void)
{
	if(mSceneMgr)
		mApplication->getOgreRoot()->destroySceneManager(mSceneMgr);
	mSceneMgr = 0;
}

void OgreARAppLogic::postShutdown(void)
{

}

//--------------------------------- Init --------------------------------

void OgreARAppLogic::createSceneManager(void)
{
	mSceneMgr = mApplication->getOgreRoot()->createSceneManager(ST_GENERIC, "SceneManager");
}

void OgreARAppLogic::createViewport(void)
{
	//mViewport = mApplication->getRenderWindow()->addViewport(0);
}

void OgreARAppLogic::createCamera(void)
{
	mCamera = mSceneMgr->createCamera("camera");
	mCamera->setNearClipDistance(0.5);
	mCamera->setFarClipDistance(50000);
	mCamera->setPosition(0, 0, 0);
	mCamera->lookAt(0, 0, 1);
	//mCamera->setFOVy(Degree(40)); //FOVy camera Ogre = 40°
	//mCamera->setAspectRatio((float) mViewport->getActualWidth() / (float) mViewport->getActualHeight());
	//mViewport->setCamera(mCamera);
	mViewport = mApplication->getRenderWindow()->addViewport(mCamera);

	mCameraNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("cameraNode");
	//mCameraNode->setPosition(0, 1700, 0);//
	//mCameraNode->lookAt(Vector3(0, 1700, -1), Node::TS_WORLD);//
	mCameraNode->attachObject(mCamera);
	//mCameraNode->setFixedYawAxis(true, Vector3::UNIT_Y);//
}

void OgreARAppLogic::createScene(void)
{
      // just create a light. The scene should be initialized in user-defined function
      Ogre::Light *L = mSceneMgr->createLight("L");
      L->setPosition(0, 1, -10);
      mSceneMgr->getRootSceneNode()->attachObject(L);

}


bool OgreARAppLogic::initAR()
{
	//INIT Realidad aumentada
	if ( !userInit(this) ) return false; // user defined function to init AR
	if(mWidth==0 || mHeight==0 || mBuffer==0) {
	  std::cout << "Image size or buffer not defined in user-defined init function" << std::endl;
	  return false;
	}
	
	// create background texture
	mPixelBox = Ogre::PixelBox(mWidth, mHeight, 1, Ogre::PF_R8G8B8, mBuffer);
	// Create Texture
	mTexture = Ogre::TextureManager::getSingleton().createManual(
		      "CameraTexture",
		      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		      Ogre::TEX_TYPE_2D,
		      mWidth,
		      mHeight,
		      0,
		      Ogre::PF_R8G8B8,
		      Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);	

	//Create Camera Material
	MaterialPtr material = MaterialManager::getSingleton().create("CameraMaterial", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	Ogre::Technique *technique = material->createTechnique();
	technique->createPass();
	material->getTechnique(0)->getPass(0)->setLightingEnabled(false);
	material->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
	material->getTechnique(0)->getPass(0)->createTextureUnitState("CameraTexture");
	return true;
}

void OgreARAppLogic::createCameraPlane(int width, int height, Ogre::Real _distanceFromCamera)
{
	// Create a prefab plane dedicated to display video
	float videoAspectRatio = width / (float) height;

	float planeHeight = 2 * _distanceFromCamera * Ogre::Math::Tan(Degree(26)*0.5); //FOVy webcam = 26° (intrinsic param)
	float planeWidth = planeHeight * videoAspectRatio;

	Plane p(Vector3::UNIT_Z, 0.0);
	MeshManager::getSingleton().createPlane("VerticalPlane", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, p , planeWidth, planeHeight, 1, 1, true, 1, 1, 1, Vector3::UNIT_Y);
	Entity* planeEntity = mSceneMgr->createEntity("VideoPlane", "VerticalPlane"); 
	planeEntity->setMaterialName("CameraMaterial");
	planeEntity->setRenderQueueGroup(RENDER_QUEUE_WORLD_GEOMETRY_1);

	// Create a node for the plane, inserts it in the scene
	Ogre::SceneNode* node = mCameraNode->createChildSceneNode("planeNode");
	node->attachObject(planeEntity);

	// Update position    
	Vector3 planePos = mCamera->getPosition() + mCamera->getDirection() * _distanceFromCamera;
	node->setPosition(planePos);

	// Update orientation
	node->setOrientation(mCamera->getOrientation());
}


void OgreARAppLogic::createCameraBackground()
{
	// Create background rectangle covering the whole screen
	Rectangle2D* rect = new Rectangle2D(true);
	rect->setCorners(-1.0, 1.0, 1.0, -1.0);
	rect->setMaterial("CameraMaterial");
 
	// Render the background before everything else
	rect->setRenderQueueGroup(RENDER_QUEUE_BACKGROUND);
 
	// Hacky, but we need to set the bounding box to something big
	// Use infinite AAB to always stay visible
	AxisAlignedBox aabInf;
	aabInf.setInfinite();
	rect->setBoundingBox(aabInf);
 
	// Attach background to the scene
	SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode("Background");
	node->attachObject(rect);
}



void OgreARAppLogic::setUserFunctions(bool (*init)(OgreARAppLogic*), bool (*update)(OgreARAppLogic*)) {
  userInit = init;
  userUpdate = update;
}

void OgreARAppLogic::setImageSize(int width, int height)
{
  mWidth = width;
  mHeight = height;
}


void OgreARAppLogic::setImageBuffer(unsigned char* buffer)
{
  mBuffer = buffer;
}




//--------------------------------- update --------------------------------

bool OgreARAppLogic::processInputs(Ogre::Real deltaTime)
{
	OIS::Keyboard *keyboard = mApplication->getKeyboard();
	if(keyboard->isKeyDown(OIS::KC_ESCAPE))
	{
		return false;
	}
	if(keyboard->isKeyDown(OIS::KC_SPACE))
	{
	  pause = !pause;
	  
	}
	return true;
}

bool OgreARAppLogic::OISListener::mouseMoved( const OIS::MouseEvent &arg )
{
	return true;
}

bool OgreARAppLogic::OISListener::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	return true;
}

bool OgreARAppLogic::OISListener::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	return true;
}

bool OgreARAppLogic::OISListener::keyPressed( const OIS::KeyEvent &arg )
{
	return true;
}

bool OgreARAppLogic::OISListener::keyReleased( const OIS::KeyEvent &arg )
{
	return true;
}
