#include "Card.h"
#include "SoundManager.h"

Card::Card(unsigned cardID, std::string name,bool hasMap):id(cardID),name(name),hasMap(hasMap){
	model=NULL;	
	surface=NULL;	
	map=NULL;	
	textMap=NULL;	
	textInfo=NULL;	
	text3D=NULL;	
};

void Card::attachObjects(){
	Ogre::SceneNode* tempNode;
	if(surface!=NULL){
		tempNode = node->createChildSceneNode();
		tempNode->attachObject(surface);
		tempNode->yaw(Ogre::Degree(90));
		tempNode->pitch(Ogre::Degree(-90));
		tempNode->setPosition(12,-2,0);
	}

	if(model!=NULL){ 
		tempNode = node->createChildSceneNode();
		tempNode->attachObject(model);
		tempNode->yaw(Ogre::Degree(45));
        tempNode->pitch(Ogre::Degree(-35));
	}

	if(map!=NULL){
		tempNode = node->createChildSceneNode();
		tempNode->attachObject(map);
		tempNode->yaw(Ogre::Degree(-45));
        tempNode->pitch(Ogre::Degree(-35));
		//tempNode->setScale(scale, scale, scale);
	}
	if(textMap!=NULL){
		tempNode = node->createChildSceneNode();
		tempNode->attachObject(textMap);
		tempNode->setPosition(-10,0,-5);
		tempNode->yaw(Ogre::Degree(270));
	}
	if(text3D!=NULL){ 
		tempNode = node->createChildSceneNode();
		tempNode->attachObject(text3D);
		tempNode->yaw(Ogre::Degree(90));
		tempNode->setPosition(+10,0,1);
	}
	if(textInfo!=NULL){
		tempNode = node->createChildSceneNode();
		tempNode->attachObject(textInfo);
		tempNode->setPosition(-2,0,10);
	}

}
