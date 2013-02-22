/**
 *  SoundManager.cpp
 *
 *  Original Code : Van Stokes, Jr. (http://www.EvilMasterMindsInc.com) - Aug 05
 *  Modified Code : Steven Gay - mec3@bluewin.ch - Septembre 2005
 *                  Deniz Sarikaya - daimler3@gmail.com - August 2010
 *					Johan Gonzalez - dymanic@gmail.com - February 2013
 */
#include "SoundManager.h"

SoundManager* SoundManager::mSoundManager = NULL;

/****************************************/
SoundManager::SoundManager(void){
	mSoundManager = this;

	isInitialised= false;
	isSoundOn = false;
	mSoundDevice = 0;
	mSoundContext = 0;

	mAudioPath = "";

	//EAX related
	isEAXPresent = false;

	//Initial position of the listener
	position[0] = 0.0;
	position[1] = 0.0;
	position[2] = 0.0;

	//Initial velocity of the listener
	velocity[0] = 0.0;
	velocity[1] = 0.0;
	velocity[2] = 0.0;

	//Initial orientation of the listener = direction + direction up
	orientation[0] = 0.0;
	orientation[1] = 0.0;
	orientation[2] = -1.0;
	orientation[3] = 0.0;
	orientation[4] = 1.0;
	orientation[5] = 0.0;

	//Needed because of hardware limitation
	mAudioBuffersInUseCount =0;
	mAudioSourcesInUseCount =0;

	for (int i=0; i<MAX_AUDIO_SOURCES; i++){
		mAudioSources[i] = 0;
		mAudioSourceInUse[i]=false;
	}

	for (int i=0; i<MAX_AUDIO_BUFFERS; i++){
		mAudioBuffers[i]=0;
		strcpy(mAudioBufferFileName[i],"--");
		mAudioBuffersInUse[i]=false;
	}

	printf("SoundManager Created.\n");
}

/*****************************/

SoundManager::~SoundManager(void){

	//Delete the sources and buffers
	alDeleteSources(MAX_AUDIO_SOURCES,mAudioSources);
	alDeleteBuffers(MAX_AUDIO_BUFFERS,mAudioBuffers);

	//Destroy the soudnd context and device
	mSoundContext = alcGetCurrentContext();
	alcMakeContextCurrent(NULL);
	alcDestroyContext(mSoundContext);
	if(mSoundDevice){
		alcCloseDevice(mSoundDevice);
	}

	printf("SoundManager Destroyed");
}

/***************************************/

void SoundManager::selfDestruct(){
	if(getSingletonPtr()) delete getSingletonPtr();
}

/*******************************************/

SoundManager* SoundManager::createManager(void){
	if(mSoundManager ==0){
		mSoundManager = new SoundManager();
	}
	return mSoundManager;
}

/********************************************/

bool SoundManager::init(void){
	//It's an error to initialize twice OpenAl
	if (isInitialised) return true;

	//Open an audio device
	mSoundDevice = alcOpenDevice(""); //TODO ((ALubyte*) "DirectSound3D");
	//mSoundDevice = alcOpenDevice("DirectSound3D");

	//Check for errors
	if( !mSoundDevice){
		printf("SoundManager::init error: No sound device.\n");
		return false;
	}

	mSoundContext = alcCreateContext(mSoundDevice,NULL);
	// if checkALError() || !mSoundContext) //TODO seems not to work
	if (!mSoundContext){
		printf("SoundManager::init error : No sound context.\n");
		return false;
	}

	//Make the context current and active
	alcMakeContextCurrent(mSoundContext);
	if (checkALError("Init()")){
		printf("SoundManager::init error : could not make sound context current and active.\n");
		return false;
	}

	// Check for EAX 2.0 support and
	// Retrieves function entry addresses to API ARB extensions, in this case,
	// for the EAX extension. See Appendix 1 (Extensions) of
	// http://www.openal.org/openal_webstf/specs/OpenAL1-1Spec_html/al11spec7.html
	//
	// TODO EAX fct not used anywhere in the code ... !!!

	isEAXPresent = alIsExtensionPresent( "EAX2.0");
	if(isEAXPresent){
		printf("EAX 2.0 Extension is available");

#ifdef _USEEAX
		eaxSet = (EAXSet) alGetProcAddress("EAXSet");
		if (eaxSet == NULL){
			isEAXPresent = false;
		}

		eaxGet = (EAXGet) alGetProcAddress("EAXGet");
		if (eaxGet == NULL){
			isEAXPresent = false;
		}

		if(!isEAXPresent){
			checkALError( "Failed to get the EAX extension functions addresses.\n");
		}
#else
		isEAXPresent = false;
		printf("... but not used.\n");
#endif //_USEEAX

	}


	//Create the Audio Buffers
	alGenBuffers(MAX_AUDIO_BUFFERS,mAudioBuffers);
	if(checkALError("init::alGenBuffers:")){
		return false;
	}

	//Genereate Sources
	alGenSources(MAX_AUDIO_SOURCES,mAudioSources);
	if(checkALError("init::alGenSources:")){
		return false;
	}

	//Setup the initial listener parameters
	// -> location
	alListenerfv(AL_POSITION,position);

	//-> velocity
	alListenerfv(AL_VELOCITY,velocity);

	//-> orientation
	alListenerfv(AL_ORIENTATION, orientation);

	//Gain
	alListenerf(AL_GAIN,1.0);

	// Initialize Doppler
	alDopplerFactor(1.0); //1.2 = exaggerate the pitch shift by 20%
	alDopplerVelocity(343.0f); //m/s this may need to be scaled at some point

	//OK
	isInitialised = true;
	isSoundOn = true;

	printf("SoundManager initialised.\n\n");

	return true;
}

/****************************************/

bool SoundManager::checkALError(void){
	ALenum errCode;
	if ((errCode=alGetError()) != AL_NO_ERROR){
		std::string err = "ERROR SoundManager:: ";
		err += (char*) alGetString(errCode);

		printf("%s\n",err.c_str());
		return true;
	}
	return false;
}

/******************************************/

std::string SoundManager::listAvailableDevices(void){
	std::string str = "Sound Devices available : ";

	if (alcIsExtensionPresent(NULL,"ALC_ENUMERATION_EXT") == AL_TRUE){
		str = "List of Devices : ";
		str += (char*) alcGetString (NULL,ALC_DEVICE_SPECIFIER);
		str += "\n";
	}else{
		str += "... enumeration error.\n";
	}

	return str;
}

/**************************************************/

bool SoundManager::checkALError(std::string pMsg){
	ALenum error = 0;

	if((error = alGetError()) == AL_NO_ERROR){
		return false;
	}

	char mStr[256];
	switch(error){
		case AL_INVALID_NAME:
			sprintf(mStr,"ERROR SoundManager::%s Invalid Name", pMsg.c_str());
			break;
		case AL_INVALID_ENUM:
			sprintf(mStr,"ERROR SoundManager::%s Invalid Enum", pMsg.c_str());
			break;
		case AL_INVALID_VALUE:
			sprintf(mStr,"ERROR SoundManager::%s Invalid Value", pMsg.c_str());
			break;
		case AL_INVALID_OPERATION:
			sprintf(mStr,"ERROR SoundManager::%s Invalid Operation", pMsg.c_str());
			break;
		case AL_OUT_OF_MEMORY:
			sprintf(mStr,"ERROR SoundManager::%s Out Of Memory", pMsg.c_str());
			break;
		default:
			sprintf(mStr,"ERROR SoundManager::%s Unknown error (%i) case in testALError()", pMsg.c_str(), error);
			break;
	};

	printf( "%s\n", mStr );

	return true;
}

//Attempts to adquire an empty audio source and assign it back to the caller
//via AudioSourceID. This will lock the source

/******************************************************/

bool SoundManager::loadAudio(std::string filename, unsigned int *audioId, bool loop){
	if(filename.empty() || filename.length() > MAX_FILENAME_LENGTH){
		std::cout<<"1load\n";
		return false;
	}

	if(mAudioSourcesInUseCount == MAX_AUDIO_SOURCES){
		std::cout<<"2load\n";
		return false; //out of audio slots
	}

	int bufferID = -1;
	int sourceID = -1;

	//Check if the pSoundFile is already loaded into a buffer
	bufferID = locateAudioBuffer(filename);
	if(bufferID <0){
		//the file isn't loaded, attempt to load it
		bufferID = loadAudioInToSystem(filename);
		if(bufferID<0){
		std::cout<<"3load\n";
			return false; //failed
		}
	}

	sourceID = 0;

	while (mAudioBuffersInUse[sourceID] == true) {
		sourceID++;
	}

	// 'sourceID' now represents a free audio source slot

	*audioId = sourceID; //return the audio source to the caller
	mAudioSourceInUse[sourceID] = true; //mark this Source slot as in use
	mAudioSourcesInUseCount++; //bump the 'in use' counter

	//Now inform OpenAL of the sound assignment and attach th audo buffer to the audio source
	alSourcei(mAudioSources[sourceID],AL_BUFFER,mAudioBuffers[bufferID]);

	alSourcei(mAudioSources[sourceID],AL_LOOPING,loop);

	if( checkALError( "loadSource()::alSourcei" )){
		std::cout<<"4load\n";
		return false;
	}

	return true;
}

//Function to chck and s if the soundfils is already loaded into a buffer

/*********************************************/

int SoundManager::locateAudioBuffer(std::string filename){
	for(unsigned int b = 0; b <MAX_AUDIO_BUFFERS;b++){
		if(filename == mAudioBufferFileName[b]){
			return (int) b;
		}
	}
	return -1;
}

//Function to load a sound file into a buffer

/*********************************************/

int SoundManager::loadAudioInToSystem(std::string filename){
	if(filename.empty()){
		std::cout<<"-1sys";
		return -1;
	}

	//Make sure we have audio buffers available
	if (mAudioBuffersInUseCount == MAX_AUDIO_BUFFERS){
		std::cout<<"-2sys";
		return -1;
	}

	//Find a free buffer slot

	int bufferID = 0;

	while (mAudioBuffersInUse[bufferID] == true) {
		bufferID++;
	}

	//load .wav, .ogg, or .au

	if(filename.find(".ogg",0) != std::string::npos){
		printf("---> found ogg\n");
		if (!loadOGG(filename,mAudioBuffers[bufferID])){
			std::cout<<"-3sys";
			return -1;
		}
	}

	//succesful load of the file
	mAudioSourceInUse[bufferID] = true; //mark as in use

	strcpy(mAudioBufferFileName[bufferID],filename.c_str());

	mAudioBuffersInUseCount++;

	return bufferID;
}

/***************************************/

bool SoundManager::playAudio(unsigned int audioID, bool forceRestart){
	//Make sure the audio ID is valid and usable
	if(audioID >= MAX_AUDIO_SOURCES || ! mAudioSourceInUse[audioID]){
		return false;
	}

	int sourceAudioState =0;

	alGetError();

	//Are we playing he audio source?
	alGetSourcei(mAudioSources[audioID],AL_SOURCE_STATE, &sourceAudioState);

	if(sourceAudioState == AL_PLAYING){
		if(forceRestart){
			stopAudio(audioID);
		}else{
			return false;
		}
	}

	alSourcePlay(mAudioSources[audioID]);
	if(checkALError("playAudio::alSourcePlay: ")){
		return false;
	}

	return true;
}

/****************************************************************************/
bool SoundManager::pauseAudio( unsigned int audioID )
{
	// Make sure the audio source ident is valid and usable
	if ( audioID >= MAX_AUDIO_SOURCES || !mAudioSourceInUse[audioID] )
		return false;

	alGetError();

	alSourcePause( mAudioSources[audioID] );

	if ( checkALError( "pauseAudio::alSourceStop ") )
		return false;

	return true;
}

/************************************************/

bool SoundManager::pauseAllAudio(void){
	if(mAudioSourcesInUseCount>=MAX_AUDIO_SOURCES){
		return false;
	}

	alGetError();

	alSourcePausev(mAudioSourcesInUseCount,mAudioSources);

	if(checkALError("pauseALLAudio::alSourceStop ")){
		return false;
	}

	return true;
}

/**********************************************/

bool SoundManager::resumeAudio(unsigned int audioID){
	//Make sure the audio ID is valid and usable
	if(audioID >= MAX_AUDIO_SOURCES || ! mAudioSourceInUse[audioID]){
		return false;
	}

	alGetError();

	alSourcePlay(mAudioSources[audioID]);
	if(checkALError("playAudio::alSourcePlay: ")){
		return false;
	}

	return true;
}

/***************************************************/
bool SoundManager::resumeAllAudio(void){
	if(mAudioSourcesInUseCount>=MAX_AUDIO_SOURCES){
		return false;
	}

	alGetError();

	int sourceAudioState = 0;

	for (int i=0;i<mAudioSourcesInUseCount;++i){

		alGetSourcei(mAudioSources[i],AL_SOURCE_STATE,&sourceAudioState);
		if(sourceAudioState == AL_PAUSED){
			resumeAudio(i);
		}
	}

	if(checkALError("resumeAllAudio::alSourceStop ")){
		return false;
	}

	return true;
}

/**************************************************/
bool SoundManager::stopAudio(unsigned int audioID){
	// Make sure the audio source ident is valid and usable
	if ( audioID >= MAX_AUDIO_SOURCES || !mAudioSourceInUse[audioID] ){
		return false;
	}

	alGetError();

	alSourceStop( mAudioSources[audioID] );

	if ( checkALError( "stopAudio::alSourceStop ") ){
		return false;
	}
	return true;
}

/****************************************************************************/
bool SoundManager::stopAllAudio( void ){
	if ( mAudioSourcesInUseCount >= MAX_AUDIO_SOURCES ){
		return false;
	}

	alGetError();

	for ( int i=0; i<mAudioSourcesInUseCount; i++ )	{
		stopAudio( i );
	}

	if ( checkALError( "stopAllAudio::alSourceStop ") ){
		return false;
	}

	return true;
}

/****************************************************************************/
bool SoundManager::releaseAudio( unsigned int audioID ){
	if ( audioID >= MAX_AUDIO_SOURCES )
		return false;
	alSourceStop( mAudioSources[audioID] );
	mAudioSourceInUse[ audioID ] = false;
	mAudioSourcesInUseCount--;
	return true;
}

/****************************************************************************/
bool SoundManager::setSound( unsigned int audioID, Vector3 position,
		Vector3 velocity, Vector3 direction, float maxDistance,
		bool playNow, bool forceRestart, float minGain ) {
	if ( audioID >= MAX_AUDIO_SOURCES || !mAudioSourceInUse[ audioID ] )
		return false;

	// Set the position
	ALfloat pos[] = { position.x, position.y, position.z };

	alSourcefv( mAudioSources[ audioID ], AL_POSITION, pos );

	if ( checkALError( "setSound::alSourcefv:AL_POSITION" ) )
		return false;

	// Set the veclocity
	ALfloat vel[] = { velocity.x, velocity.y, velocity.z };

	alSourcefv( mAudioSources[ audioID ], AL_VELOCITY, vel );

	if ( checkALError( "setSound::alSourcefv:AL_VELOCITY" ) )
		return false;

	// Set the direction
	ALfloat dir[] = { velocity.x, velocity.y, velocity.z };

	alSourcefv( mAudioSources[ audioID ], AL_DIRECTION, dir );

	if ( checkALError( "setSound::alSourcefv:AL_DIRECTION" ) )
		return false;

	// Set the max audible distance
	alSourcef( mAudioSources[ audioID ], AL_MAX_DISTANCE, maxDistance );

	// Set the MIN_GAIN ( IMPORTANT - if not set, nothing audible! )
	alSourcef( mAudioSources[ audioID ], AL_MIN_GAIN, minGain );

	// Set the max gain
	alSourcef( mAudioSources[ audioID ], AL_MAX_GAIN, 1.0f ); // TODO as parameter ? global ?

	// Set the rollof factor
	alSourcef( mAudioSources[ audioID ], AL_ROLLOFF_FACTOR, 1.0f ); // TODO as parameter ?

	// Do we play the sound now ?
	if ( playNow ) return playAudio( audioID, forceRestart ); // TODO bof... not in this fct

	return true;
}

/****************************************************************************/
bool SoundManager::setSoundPosition( unsigned int audioID, Vector3 position ){
	if ( audioID >= MAX_AUDIO_SOURCES || !mAudioSourceInUse[ audioID ] )
		return false;

	// Set the position
	ALfloat pos[] = { position.x, position.y, position.z };

	alSourcefv( mAudioSources[ audioID ], AL_POSITION, pos );

	if ( checkALError( "setSound::alSourcefv:AL_POSITION" ) )
		return false;

	return true;
}

/****************************************************************************/
bool SoundManager::setSoundPosition( unsigned int audioID, Vector3 position,
		Vector3 velocity, Vector3 direction ) {
	if ( audioID >= MAX_AUDIO_SOURCES || !mAudioSourceInUse[ audioID ] )
		return false;

	// Set the position
	ALfloat pos[] = { position.x, position.y, position.z };

	alSourcefv( mAudioSources[ audioID ], AL_POSITION, pos );

	if ( checkALError( "setSound::alSourcefv:AL_POSITION" ) )
		return false;

	// Set the veclocity
	ALfloat vel[] = { velocity.x, velocity.y, velocity.z };

	alSourcefv( mAudioSources[ audioID ], AL_VELOCITY, vel );

	if ( checkALError( "setSound::alSourcefv:AL_VELOCITY" ) )
		return false;

	// Set the direction
	ALfloat dir[] = { velocity.x, velocity.y, velocity.z };

	alSourcefv( mAudioSources[ audioID ], AL_DIRECTION, dir );

	if ( checkALError( "setSound::alSourcefv:AL_DIRECTION" ) )
		return false;

	return true;
}

/****************************************************************************/
bool SoundManager::setListenerPosition( Vector3 position, Vector3 velocity,
		Quaternion orientation ){
	Vector3 axis;

	// Set the position
	ALfloat pos[] = { position.x, position.y, position.z };

	alListenerfv( AL_POSITION, pos );

	if ( checkALError( "setListenerPosition::alListenerfv:AL_POSITION" ) )
		return false;

	// Set the veclocity
	ALfloat vel[] = { velocity.x, velocity.y, velocity.z };

	alListenerfv( AL_VELOCITY, vel );

	if ( checkALError( "setListenerPosition::alListenerfv:AL_VELOCITY" ) )
		return false;

	// Orientation of the listener : look at then look up
	axis = Vector3::ZERO;
	axis.x = orientation.getYaw().valueRadians();
	axis.y = orientation.getPitch().valueRadians();
	axis.z = orientation.getRoll().valueRadians();

	// Set the direction
	ALfloat dir[] = { axis.x, axis.y, axis.z };

	alListenerfv( AL_ORIENTATION, dir );

	if ( checkALError( "setListenerPosition::alListenerfv:AL_DIRECTION" ) )
		return false;

	// TODO as parameters ?
	alListenerf( AL_MAX_DISTANCE, 10000.0f );
	alListenerf( AL_MIN_GAIN, 0.0f );
	alListenerf( AL_MAX_GAIN, 1.0f );
	alListenerf( AL_GAIN, 1.0f );

	return true;
}

bool SoundManager::loadOGG( std::string filename, ALuint pDestAudioBuffer ){    
	OggVorbis_File oggfile;

	if (alIsBuffer(pDestAudioBuffer) == AL_FALSE){
		printf("SoundManager::loadOGG : bad AL buffer.\n");
		return false;
	}

	if(ov_fopen(const_cast<char*>(filename.c_str()), &oggfile)){
		printf("SoundManager::loadOGG() : ov_fopen failed.\n");
		return false;
	}

	vorbis_info* info = ov_info(&oggfile, -1);

	ALenum format;
	printf("channels = %d",info->channels);
	switch(info->channels)
	{
		case 1:
			format = AL_FORMAT_MONO16; break;
		case 2:
			format = AL_FORMAT_STEREO16; break;
		case 4:
			format = alGetEnumValue("AL_FORMAT_QUAD16"); break;
		case 6:
			format = alGetEnumValue("AL_FORMAT_51CHN16"); break;
		case 7:
			format = alGetEnumValue("AL_FORMAT_61CHN16"); break;
		case 8:
			format = alGetEnumValue("AL_FORMAT_71CHN16"); break;
		default:
			format = 0; break;
	}

	std::vector<int16> samples;
	unsigned size = ov_pcm_total(&oggfile,-1)*info->channels*2;
	char * data = (char*) malloc(size);
	char* dataptr = data;
	int section = 0;
	bool firstrun = true;
	while(1){
		int result = ov_read(&oggfile, dataptr, 4096, 0, 2, 1, &section);
		if(result > 0){
			firstrun = false;
			dataptr += result;
		}else{
			if(result < 0){
				printf("SoundManager::loadOGG() : Loading ogg sound data failed!");
				ov_clear(&oggfile);
				return false;
			}
			else
			{
				if(firstrun){
					return false;
				}
				break;
			}
		}
	}

	std::cout<<pDestAudioBuffer<<","<<format<<','<<&samples[0]<<','<<ov_pcm_total(&oggfile,-1)<<','<<info->rate;;
	alBufferData(pDestAudioBuffer, format, data, size, info->rate);
	checkALError("asda");
	free(data);
	return true;
}
