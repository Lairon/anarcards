#ifndef OGREAPPLOGIC_H
#define OGREAPPLOGIC_H

#include <OgrePrerequisites.h>
#include <OgreStringVector.h>
#include <OIS/OIS.h>
#include <OgreTextureManager.h>


class OgreApp;


class OgreARAppLogic
{
     bool pause;
public:
	OgreARAppLogic();
	~OgreARAppLogic();

public:
	void setParentApp(OgreApp *app) { mApplication = app; }

	/// Called before Ogre and everything in the framework is initialized
	/// Configure the framework here
	bool preInit(const Ogre::StringVector &commandArgs);
	/// Called when Ogre and the framework is initialized
	/// Init the logic here
	bool init(void);
	
	/// set user funcions to init AR Application and to update every frame (see example)
	void setUserFunctions(bool (*init)(OgreARAppLogic*), bool (*update)(OgreARAppLogic*));
	
	/// set image size and buffer. this funcions have to be called in the init user-defined funcion!!!
	void setImageSize(int width, int height);
	void setImageBuffer(unsigned char* buffer);
	
	
public:

	/// Called before everything in the framework is updated
	bool preUpdate(Ogre::Real deltaTime);
	/// Called when the framework is updated
	/// update the logic here
	bool update(Ogre::Real deltaTime);

	/// Called before Ogre and the framework are shut down
	/// shutdown the logic here
	void shutdown(void);
	/// Called when Ogre and the framework are shut down
	void postShutdown(void);

protected:
	void createSceneManager(void);
	void createViewport(void);
	void createCamera(void);
	void createScene(void);
	bool initAR();

	bool processInputs(Ogre::Real deltaTime);

	void createCameraPlane(int width, int height, Ogre::Real _distanceFromCamera);
	void createCameraBackground();

	// OGRE
	OgreApp *mApplication;
	
public:
	Ogre::SceneManager *mSceneMgr;
	Ogre::Viewport *mViewport;
	Ogre::Camera *mCamera;
	Ogre::SceneNode* mCameraNode;
	Ogre::SceneNode* mObjectNode;
	
	
private:
	
	/// For augmented Reality
  	int mWidth, mHeight;
	unsigned char* mBuffer;
	Ogre::TexturePtr mTexture;
	Ogre::PixelBox mPixelBox;

	/// user defined functions
	bool (* userInit) ( OgreARAppLogic* );
	bool (* userUpdate) (OgreARAppLogic* );


	// OIS
	class OISListener : public OIS::MouseListener, public OIS::KeyListener
	{
	public:
		virtual bool mouseMoved( const OIS::MouseEvent &arg );
		virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
		virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
		virtual bool keyPressed( const OIS::KeyEvent &arg );
		virtual bool keyReleased( const OIS::KeyEvent &arg );
		OgreARAppLogic *mParent;
	};
	friend class OISListener;
	OISListener mOISListener;
};

#endif // OGREAPPLOGIC_H
