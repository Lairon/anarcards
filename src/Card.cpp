#include "Card.h"
#include "SoundManager.h"

Card::Card(unsigned cardID, std::string name):id(cardID),name(name){
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
		tempNode->setScale(scale, scale, scale);
		Ogre::Real offsety = surface->getBoundingBox().getHalfSize().y;
		tempNode->translate(-0.25,+offsety*scale,0.9,Ogre::Node::TS_PARENT);
	}

	if(model!=NULL){ 
		tempNode = node->createChildSceneNode();
		tempNode->attachObject(model);
		tempNode->setScale(scale, scale, scale);
		Ogre::Real offsety = surface->getBoundingBox().getHalfSize().y;
		tempNode->yaw(Ogre::Degree(90));
		tempNode->translate(0,(+offsety*scale)+0.1,0,Ogre::Node::TS_LOCAL);
	}

	if(map!=NULL){
		tempNode = node->createChildSceneNode();
		tempNode->attachObject(map);
		tempNode->setScale(scale, scale, scale);
		Ogre::Real offsety = surface->getBoundingBox().getHalfSize().y;
		tempNode->translate(-0.25,+offsety*scale,0.9,Ogre::Node::TS_PARENT);
	}
	if(textMap!=NULL){
		tempNode = node->createChildSceneNode();
		tempNode->attachObject(textMap);
		tempNode->setScale(scale, scale, scale);
		//ogreNode[i][j]->scale(12,12,12);
		tempNode->yaw(Ogre::Degree(90));
		Ogre::Real offsety = surface->getBoundingBox().getHalfSize().y;
		tempNode->translate(-0.21,+offsety*scale,1.2,Ogre::Node::TS_LOCAL);
	}
	if(text3D!=NULL){ 
		tempNode = node->createChildSceneNode();
		tempNode->attachObject(text3D);
		tempNode->setScale(scale, scale, scale);
		tempNode->yaw(Ogre::Degree(90));
		Ogre::Real offsety = surface->getBoundingBox().getHalfSize().y;
		tempNode->translate(-0.24,+offsety*scale,1.2,Ogre::Node::TS_LOCAL);
	}
	if(textInfo!=NULL){
		tempNode = node->createChildSceneNode();
		tempNode->attachObject(textInfo);
		tempNode->setScale(scale, scale, scale);
		tempNode->yaw(Ogre::Degree(90));
		Ogre::Real offsety = surface->getBoundingBox().getHalfSize().y;
		tempNode->translate(-0.25,+offsety*scale,0.9,Ogre::Node::TS_PARENT);
	}

}
