//-------------------------------------------------------------------------------------
/// \file	MD2_Manager.cpp
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

#include "MD2_Manager.h"




//-----------------------------------------------------------------------------------------------	Md2Manager :: CurrentModelFps
//
float Md2Manager::CurrentModelFps() {
	float frame_time=0;
	for(unsigned int i=0;i!=m_UpdateStagger;++i)
		frame_time += m_LastUpdates[i];
	return 1.0f/frame_time;
}

//-----------------------------------------------------------------------------------------------	Md2Manager :: GetStagger
//
unsigned char Md2Manager::GetStagger() {
	return m_UpdateStagger;
}

//-----------------------------------------------------------------------------------------------	Md2Manager :: GetMemoryUsage
//
unsigned int Md2Manager::GetMemoryUsage(Md2MemoryType type) {
	return 1;
}

//-----------------------------------------------------------------------------------------------	Md2Manager :: IsValid
//
bool Md2Manager::IsValid(const Md2Instance* ptr) {
	std::vector<Md2Manager::ModelRef*>::const_iterator it = m_LoadedModels.begin();
	for( ; it != m_LoadedModels.end(); ++it )
	{
		unsigned int i=0;
		for( ; i != (*it)->pModel->m_Instances.size(); ++i )
		{
			if(ptr == (*it)->pModel->m_Instances[i])
				return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------------------------	Md2Manager :: Load
//
Md2Instance* Md2Manager::Load(foo *bar, unsigned short fps, GLuint texture) {
	
	std::vector<ModelRef*>::iterator it = m_LoadedModels.begin();
	for( ; it != m_LoadedModels.end(); ++it )
	{
		if( (*it)->Filename == bar->fp ) {
			return (*it)->pModel->CreateInstance(texture);
		}
	}
	ModelRef* pmref = new ModelRef;
	assert(pmref);
	pmref->pModel = new Md2Model;
	assert(pmref->pModel);
	pmref->Filename = bar->fp;
	
	if(!pmref->pModel->Load(bar)) {
		throw 123;
		delete pmref->pModel;
		delete pmref;
		return 0;
	}

	pmref->pModel->SetFps(fps);

	m_LoadedModels.push_back(pmref);

	return pmref->pModel->CreateInstance(texture);

}


//-----------------------------------------------------------------------------------------------	Md2Manager :: Release
//
void Md2Manager::Release() {
	std::vector<Md2Manager::ModelRef*>::iterator it = m_LoadedModels.begin();
	for( ; it != m_LoadedModels.end(); ++it ) {
		delete (*it)->pModel;
		delete *it;
	}
	m_LoadedModels.clear();
}

//-----------------------------------------------------------------------------------------------	Md2Manager :: Render
//
void Md2Manager::Render() {

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// render all model instances
	std::vector<ModelRef*>::iterator it = m_LoadedModels.begin();
	for( ; it != m_LoadedModels.end(); ++it )
	{
		(*it)->pModel->Render();
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}


//---------------------------------------------------------------------------------
/// this function allows you to stagger the updates for the models. ie, 2 would 
///	indicate that it should update every other frame, thus only half the models are
/// updated each frame.
///
void Md2Manager::SetStagger(unsigned char num) {
	if(num>8)
		num = 8;
	if(!num)
		num=1;
	m_UpdateStagger = num;
}


void Md2Manager::Update(float dt) {
	float ddt = 0.0f;
	m_LastUpdates[ m_UpdateCurrent ] = dt;
	for( unsigned char c = 0; c != m_UpdateStagger; ++c ) 
		ddt += m_LastUpdates[c];

	std::vector<ModelRef*>::iterator it = m_LoadedModels.begin();
	for( unsigned char i=m_UpdateCurrent; it != m_LoadedModels.end(); ++it, ++i )
	{
		(*it)->pModel->Update(ddt,i%m_UpdateStagger,m_UpdateStagger);
	}

	if( (++m_UpdateCurrent) >= m_UpdateStagger )
		m_UpdateCurrent = 0;
}