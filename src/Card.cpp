#include "Card.h"
#include "SoundManager.h"

Card::Card(unsigned cardID, std::string name,bool hasMap):id(cardID),name(name),hasMap(hasMap){
	model=NULL;	
	surface=NULL;	
	map=NULL;	
	textMap=NULL;	
	textInfo=NULL;	
	text3D=NULL;	
	textInfo2=NULL;	
	textMap2=NULL;	
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
		tempNode->yaw(Ogre::Degree(225));
        tempNode->pitch(Ogre::Degree(-35));
		//tempNode->setScale(scale, scale, scale);
	}
	if(textMap!=NULL){
		tempNode = node->createChildSceneNode();
		tempNode->attachObject(textMap);
		tempNode->setPosition(-12,0,-5);
		tempNode->yaw(Ogre::Degree(270));
	}
	if(text3D!=NULL){ 
		tempNode = node->createChildSceneNode();
		tempNode->attachObject(text3D);
		tempNode->yaw(Ogre::Degree(90));
		tempNode->setPosition(14,0,1);
	}
	if(textInfo!=NULL){
		tempNode = node->createChildSceneNode();
		tempNode->attachObject(textInfo);
		tempNode->setPosition(-2,0,10);
	}
    if(map!=NULL){
	    Ogre::SceneNode* tempNode2;
	    Ogre::SceneNode* tempNode3;
        tempNode3 = node->createChildSceneNode();
        tempNode = tempNode3->createChildSceneNode();
        tempNode2 = tempNode3->createChildSceneNode();
        tempNode->attachObject(textInfo2);
        tempNode->setPosition(-3,0,0);
        tempNode2->attachObject(textMap2);
        tempNode2->setPosition(+3,0,0);
        tempNode3->setPosition(4,0,-10);
        tempNode3->yaw(Ogre::Degree(180));

    }

}
