//-------------------------------------------------------------------------------------
/// \file	MD2_Manager.h
/// \author	Rob Bateman	[robthebloke@hotmail.com]
/// \date	30-12-2004
/// \brief	This MD2 loader is setup in a fairly non-intuitive way. The reason is to 
///			create as many animated characters as possible whilst maintaining the smallest
///			amount of memory and processor usage. hehe, i want to be able to handle
///			about 500 visibile characters, and maybe another 2000 within the scene at
///			any one time!
///
///			As far as usage goes, the Md2Instance class is all you really want to be 
///			concerned with. This is used to set the characters position and orientation;
///			and also controls which anim cycle is used. The anim switching could be
///			improved somewhat, but i'll leave that for you to do.... ;)
///
///			The Md2Manager class allows you to create new instances of models, and provides
///			global functions to query memory usage, update all model instances, and 
///			render all instances. Control is somewhat taken away from direct manipulation
///			of Md2Models, however this is done to ensure that we use some fairly hacky
///			tricks to minimise the update calculation and minimise state changes when
///			rendering the models. 
/// 
///			By providing a global manager, we can also maintain usage of textures and
///			models loaded by the game engine. ie, we can use reference counting so that
///			texture and model data gets unloaded when we no longer need it. Since this
///			process is automated, simly use Md2Manager::Load and Md2Manager::Delete
///			to your little hearts content ;)
/// 
///			Since this code does not natively support the loading of textures, a mechanism
///			is provided for you to specify a callback function to load this data. The 
///			textures loaded are those specified as skins within the actual MD2 file. Since
///			this may be set up with some odd path, first the path is checked 
///			
//-------------------------------------------------------------------------------------

#ifndef MD2__MANAGER__H__
#define MD2__MANAGER__H__

#include "MD2_Model.h"
#include <map>

/// \brief	A global manager for the Md2 Files
///
class Md2Manager {

public:
	
	//static unsigned int Md2LoadTex(std::string);
	//static void Md2ReleaseTex(unsigned int);

	//---------------------------------------------------------------------------------
	/// \brief	releases all models, textures and kills itself...
	/// 
	void Release();

	//---------------------------------------------------------------------------------
	/// \brief	returns true if the specified instance is valid
	/// \param	instance	-	the instance to query if valid
	///	\return	true if OK
	/// 
	bool IsValid(const Md2Instance*);

	//---------------------------------------------------------------------------------
	/// \brief	used to return info about the amount of memory used by all the MD2 files.
	/// \param	type	-	the memory to request
	/// \return	the number of bytes for the memory type
	///
	unsigned int GetMemoryUsage(Md2MemoryType);

	//---------------------------------------------------------------------------------
	/// \brief	This function returns a new instance of the specified Md2 file. Internally
	///			this will first check to see if the model is loaded, if not it will load
	///			the model up. A new instance of the model is then created and returned. 
	///			This prevents you duplicating un-nessecary data.
	/// \param	filename	-	name of the Md2 file to load
	/// \param	fps			-	the frames per second for the model
	/// \return	a pointer to the model instance or NULL
	/// 
	Md2Instance* Load(foo *bar, unsigned short fps);

	//---------------------------------------------------------------------------------
	/// \brief	This function deletes the specified model instance. If no instances remain
	///			for the model, then it will be deleted
	/// \param	ptr	-	the instance to delete
	/// \return	true if OK
	/// 
	bool Delete(Md2Instance* ptr);
	
	//---------------------------------------------------------------------------------
	/// \brief	This function updates all current model instances
	/// \param	dt	-	the time increment
	///
	void Update(float dt);
	
	//---------------------------------------------------------------------------------
	/// \brief	This function updates all current model instances
	///
	void Render();
	
	//---------------------------------------------------------------------------------
	/// this function allows you to stagger the updates for the models. ie, 2 would 
	///	indicate that it should update every other frame, thus only half the models are
	/// updated each frame.
	///
	void SetStagger(unsigned char num) ;

	//---------------------------------------------------------------------------------
	/// \brief	this function allows you to return the update stagger amount 
	/// \return	the stagger amount, ie, how many frames between updates. 1 indicates
	///			update every frame, 2 indicates update every other frame
	/// 
	unsigned char GetStagger() ;

	//---------------------------------------------------------------------------------
	/// \brief	This function returns the current frame rate at which the model instances
	///			are updating. This can be used to query if the update rate has dropped too
	///			low. If this happens, then you can simply descrease the stagger amount. ie,
	/// \code	
	///		if( Md2Manager::CurrentModelFps() < 25 ) 
	///			Md2Manager::SetStagger( Md2Manager::GetStagger() - 1 );
	/// \endcode
	/// \return	the current frame per second at which the model is updating.
	/// 
	float CurrentModelFps() ;

	/// internal reference for a loadede model
	struct ModelRef {
		
		/// the name of the model loaded	
		FILE* Filename;

		/// pointer to the model data
		Md2Model* pModel;
	};

	std::vector<ModelRef*> m_LoadedModels;
	
	unsigned char m_UpdateStagger;
	
	/// the current instance to update
	unsigned char m_UpdateCurrent;
	
	/// we can stagger updates to the array of instances so we store the last few updates.
	/// we can thereofre accumulate the time from the last n frames.
	float m_LastUpdates[8];
	
	
	
	
	
	
	//std::vector<Md2Manager::ModelRef*> m_LoadedModels;

	Md2Manager() : m_LoadedModels(), m_UpdateStagger(), m_UpdateCurrent() {
		m_UpdateStagger=1;
		m_UpdateCurrent=0;
		for (int i=0; i<=8; i++) {
			m_LastUpdates[i] = 0.0f;
		}
	}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
};

#endif