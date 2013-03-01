#include "Ogre.h"

const float scale = 0.00675f;
class Card{
	public:
		int id;
		std::string name;

		/*Ogre variables*/
		Ogre::SceneNode* node;
		Ogre::Entity* surface;
		Ogre::Entity* map;
		Ogre::Entity* model;
		Ogre::Entity* textMap;
		Ogre::Entity* text3D;
		Ogre::Entity* textInfo;

		/*SoundVariables*/
		unsigned soundInfoID;
		unsigned soundMapID;

		Card(unsigned cardID, std::string name);

		void attachObjects();
		
		bool operator==(const Card& b){
			return b.id==this->id;
		}
};
