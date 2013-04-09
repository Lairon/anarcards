#include "Ogre.h"

const float scale = 0.00675f;
class Card{
	public:
		int id;
		std::string name;
		bool hasMap;

		/*Ogre variables*/
		Ogre::SceneNode* node;
		Ogre::Entity* surface;
		Ogre::Entity* map;
		Ogre::Entity* model;
		Ogre::Entity* textMap;
		Ogre::Entity* text3D;
		Ogre::Entity* textInfo;
        Ogre::Entity* textInfo2;
        Ogre::Entity* textMap2;

		/*SoundVariables*/
		unsigned soundInfoID;
		unsigned soundMapID;

		Card():id(-1){};
		Card(unsigned cardID, std::string name,bool hasMap);

		void attachObjects();
		
		bool operator==(const Card& b){
			return b.id==this->id;
		}
};
