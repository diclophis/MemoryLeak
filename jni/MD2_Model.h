//-----------------------------------------------------------------------------------------------
/// \file	MD2_Model.h
/// \author	Rob Bateman	[robthebloke@hotmail.com]
/// \date	30-12-2004
/// \brief	This file defines the main model and instance classes for the MD2 loader.
///			Basically a model is split into 2 main classes, the Md2Model, and Md2Instance.
///			The model contains all data that does not change, ie tex coords, keyframes
///			etc. The Instance contains only the data that changes as the animation
///			updates, ie the vertex and normal positions. It also contains info about
///			what cycle is running and the current anim time. 
///
///			Spliiting the data in this way allows you to do one thing, create as many
///			independant instances of a specific model as you require. For example, using
///			the model from gametutorials.com, the actual model data is about 300k, each
///			instance requires about 17k of data. ie, for 10 instances, we'd only use
///			a total of 300 + 17*10 == 470k. If you compare that to the gametutorials demo,
///			that uses about a 1.1Mb per instance, thus ~11Mb for 10 independant instances. 
///
///			Unlike a number of other Md2 loaders i've seen about, this one uses vertex
///			arrays for final rendering (or VBO). This gives a significant speed increase
///			for rendering. The downside is that we need to store additional info to 
///			map the original points to their respective vertex array positions. ie, 
///			the keyframe data gets evaluated, normals get calculated, then data is 
///			expanded into the vertex arrays. This is not the quickest thing to do, 
///			so your load times may take a little while. generally though, in release
///			mode this loading time is not terrible, and the performance increases ARE
///			worth it. 
///
///			This loader has the option to generate normal vectors for you. This incurs
///			some calculation overhead and a little bit of extra storage space required
///			for each instance. (ie, the instance data basically doubles in size should
///			you use this).
///
///			The animation data can either be stored as unsigned chars or floats. 
///			Realistically there is very little point in using floats since it requires
///			almost times more memory..... 
///	\note	Sorry for the rather excessive use of #ifdefs  :p
///			
//-----------------------------------------------------------------------------------------------

#ifndef MD2__MODEL__H__
#define MD2__MODEL__H__

#ifdef WIN32
	#pragma once
#endif


//-----------------------------------------------------------------------------------------------

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <string>

#include "MD2_Config.h"
#include "MD2_File.h"
#include "MD2_Model.h"

#include "importgl.h"

#pragma pack(push,1)


//-----------------------------------------------------------------------------------------------	MD2 Memory Management

/// this defines a constant used to query the memory usage of a model and its instances
enum Md2MemoryType {

	/// size of internal global data buffers used to evaulate the points and normals
	/// for the instances. VBO will add additional overhead for a buffer that gets 
	/// filled before sending the data to the graphics card via glSubBufferDataARB
	MEM_GLOBAL,

	/// data type that is static. ie, data for the model
	MEM_MODEL,

	/// data that changes, basically evaluated vertices and other data for the instances
	MEM_INSTANCE,

	/// data stored within a vertex buffer object shared between all instances.
	/// This includes tex coords, face indices
	MEM_MODEL_VBO,

	/// the size of the data stored as VBO's for the instances. ie, vertices and normals
	MEM_INSTANCE_VBO,

	/// the amount of texture data stored on the graphics card
	MEM_TEXTURES,

	/// all memory used. 
	MEM_ALL
};


//-----------------------------------------------------------------------------------------------	MD2 Texture Stuff

//extern "C" {
//	/// function prototype for a texture loader
//	typedef int (*Md2TexLoadFunc)(const char*,unsigned char**,unsigned int*,unsigned int*,unsigned int*);
//}

//-----------------------------------------------------------------------------------------------	struct Md2BoundingBox
//
#if MD2_CALCULATE_BBOX
struct Md2BoundingBox {
	float minimum[3];
	float maximum[3];
};
#endif

//-----------------------------------------------------------------------------------------------	struct Md2BoundingSphere
//
#if MD2_CALCULATE_BSPHERE
struct Md2BoundingSphere {
	float center[3];
	float radius;
};
#endif

//-----------------------------------------------------------------------------------------------	struct Md2TexCoord
//
struct Md2TexCoord {
	union {
		struct {
			float u;
			float v;
		};
		float data[2];
	};
	/// this converts the tex coord from an MD2 version.
	inline void FromMd2(const MD2::uv& tc, unsigned short iw, unsigned short ih) {
		u= static_cast<float>( tc.s ) / iw;
		v= static_cast<float>( tc.t ) / ih;
	}
	/// equality operator
	inline const Md2TexCoord& operator==(const Md2TexCoord& tex) {
		u= tex.u;
		v= tex.v;
		return *this;
	}
};	// 8 bytes

//-----------------------------------------------------------------------------------------------	struct Md2Vertex
//
struct Md2Vertex {
	union {
		struct {
			MD2_DATA_TYPE vx;
			MD2_DATA_TYPE vy;
			MD2_DATA_TYPE vz;
		};
		MD2_DATA_TYPE vertex[3];
	};
	/// this converts the MD2 from byte's to floats (scales & translates them). It also 
	/// changes the model from Z up to Y up.
	/// \param	v	-	the vertex in the file data
	/// \param	scale	-	a scale factor to apply to the model
	///	\param	translation	-	a vector offset to apply to all vertices
	///
	
	#if MD2_USE_FLOATS
	inline void FromMd2(const MD2::vertex& v, const float scale[],const float translation[]) {
	#else
	inline void FromMd2(const MD2::vertex& v) {
	#endif
		#if MD2_USE_FLOATS
			vx =    v.vert[0]*scale[0] + translation[0];
			vz =  -(v.vert[1]*scale[1] + translation[1]);
			vy =    v.vert[2]*scale[2] + translation[2];
		#else
			vx = v.vert[0];
			vz = v.vert[1];
			vy = v.vert[2];
		#endif
	}
};	// 3 or 12 bytes

//-----------------------------------------------------------------------------------------------	struct Md2VertexNormal
//
struct Md2VertexNormal {
	/// the normal data
	#if MD2_USE_NORMALS
		union {
			struct {
				float nx;
				float ny;
				float nz;
			};
			float normal[3];
		};
	#endif

	/// the vertex data
	union {
		struct {
			float vx;
			float vy;
			float vz;
		};
		float vertex[3];
	};
};

//-----------------------------------------------------------------------------------------------	struct Md2VertexArrayIndex
/// This structure holds a texture coord and vertex index for lookup when converting
/// the data to vertex arrays
/// 
struct Md2VertexArrayIndex {
	
	/// ctor
	inline Md2VertexArrayIndex() : v(), t() {}

	/// copy ctor
	inline Md2VertexArrayIndex(const Md2VertexArrayIndex& i) : v(i.v), t(i.t) {}
	
	bool operator == (const Md2VertexArrayIndex& i) const ;

	/// the vertex index used
	unsigned short v;

	/// the texture coord index
	unsigned short t;
};

//-----------------------------------------------------------------------------------------------	struct Md2VertexArrayIndexList
/// a list of tex coord and vertex indices. Used only as a lookup when converting the
/// data to vertex arrays. 
///
struct Md2VertexArrayIndexList : public std::vector<Md2VertexArrayIndex> {

	unsigned short Insert(Md2VertexArrayIndex& index);
};

#if !MD2_ALWAYS_TRIANGLES

//-----------------------------------------------------------------------------------------------	struct Md2VertexArrayIndex
/// This structure holds a texture coord and vertex index for lookup when converting
/// the data to vertex arrays
/// 
struct Md2VertexArrayIndex2 {
	
	/// ctor
	inline Md2VertexArrayIndex2(){}

	/// copy ctor
	inline Md2VertexArrayIndex2(const Md2VertexArrayIndex2& i) : v(i.v) {
		uv[0] = i.uv[0];
		uv[1] = i.uv[1];
	}
	
	bool operator == (const Md2VertexArrayIndex2& i) const ;

	/// the vertex index used
	unsigned short v;

	/// the texture coord index
	float uv[2];
};

//-----------------------------------------------------------------------------------------------	struct Md2VertexArrayIndexList
/// a list of tex coord and vertex indices. Used only as a lookup when converting the
/// data to vertex arrays. 
///
struct Md2VertexArrayIndexList2 
	: public std::vector<Md2VertexArrayIndex2> {

	unsigned short Insert(Md2VertexArrayIndex2& index);
};

/// a structure to hold the info about the triangle strips and fans used
struct Md2StripInfo {
	// boolean flag. if 0, triangle strip, else triangle fan
	unsigned short isFan:1;

	// the number of indices in this fan
	unsigned short num:15;
};

#endif


//-----------------------------------------------------------------------------------------------	class Md2AnimCycle
//
class Md2AnimCycle {
	friend class Md2Model;
public:
	
	Md2AnimCycle(const Md2AnimCycle&);
	Md2AnimCycle& operator=(const Md2AnimCycle&);

	/// ctor
	Md2AnimCycle() ;

	/// dtor
	~Md2AnimCycle() ;

	/// \brief	evaluates the current animation cycle and writes the result
	///			into the global buffer, g_VertBuffer
	/// \param	fframe	-	the first keyframe
	/// \param	numV	-	the number of vertices in the model
	/// \param	t		-	interpolating t between fframe, and (fframe+1). 
	/// \param	scale	-	the output scale gets wanged here
	/// \param	tr		-	the output translation gets wanged here
	/// 
	#if MD2_USE_FLOATS
		void Update(unsigned char fframe,unsigned short numV,float t);
	#else
		void Update(unsigned char fframe,unsigned short numV,float t,float scale[3],float tr[3]);
	#endif

	/// \return	the name of the cycle
	///
	char* GetName(){
		return m_Name;
	}

	/// returns pointer for specified frame
	/// \param	frame ID
	/// \param	size	-	the number of verts in the model
	///
	Md2Vertex* GetFrame(unsigned short frame,unsigned long size);

	/// \brief	allocates the memory to store the keyframe data for this cycle
	/// \param	sz	-	the number of verts in the model
	/// \param	num	-	the number of keys
	///
	void SetSize(unsigned int sz,unsigned int num) { 
		m_NumFrames=num;
		m_KeyFrameData = new Md2Vertex[ sz*num ];
		#if !MD2_USE_FLOATS
			m_ScaleKeys = new float[3*num];
			m_TranslateKeys = new float[3*num];
		#endif
	}

	/// \brief	returns the size in bytes of the data used
	/// \return	the number of frames within this animation
	///
	unsigned char GetNumFrames() const {
		return m_NumFrames;
	}

	/// \brief	returns the size in bytes of the data used
	/// \param	size	-	number of vertices in the model
	///
	unsigned int GetDataSize(unsigned long size) const {
		#if MD2_USE_FLOATS
			return sizeof(Md2AnimCycle) + m_NumFrames * (size * sizeof(Md2Vertex));
		#else
			return sizeof(Md2AnimCycle) + m_NumFrames * (6*sizeof(float) + size * sizeof(Md2Vertex));
		#endif
	} 

//private:
	
	/// the name of the cycle
	char m_Name[15];

	/// the number of frames in the cycle
	unsigned char m_NumFrames;

	/// a quick lookup for the start of the keyframe data for this cycle
	Md2Vertex* m_KeyFrameData;

	#if !MD2_USE_FLOATS
		/// the key frame scale values
		float* m_ScaleKeys;

		/// the key frame translation values
		float* m_TranslateKeys;
	#endif
};	//	20-28 bytes


	
class Md2Model;
	
	
	
//-----------------------------------------------------------------------------------------------	class Md2Instance
/// data that changes when a model is animating. This is seperated from the rest of 
/// the model data so that we can create lots of instances whilst sharing the main
///	models data - ie, not duplicating it
///
class Md2Instance {
	
	friend class Md2Model;
	//friend void Md2ReleaseTex(unsigned int);
public:

	Md2Instance(const Md2Instance&);
	Md2Instance& operator=(const Md2Instance&);
	
	//-------------------------------------------------------------------------------------------	Visibility

	
	/// \brief	This function allows you to perform you own visibility tests on the model.
	///			If you set the visibility to false, then the model is not rendered, and it 
	///			is not updated until it is visible again.
	/// \param	visible	-	if true, the model will be rendered and updated. 
	/// \note	the animation time is still updated even though the VBO's aren't. This means
	///			that the anims will still look like they've updated when they are off the screen
	/// 
	void SetVisible(bool visible);


	//-------------------------------------------------------------------------------------------	Anim Controls

	/// \brief	returns the number of anim cycles in the model
	/// \return	the number of cycles in the loaded model
	/// 
	unsigned int GetNumCycles() const ;

	/// \brief	returns the name of the requested cycle
	/// \param	id	-	the index of the cycle to query
	/// \return	the name of the animation cycle
	/// 
	const char* GetCycleName(unsigned int idx) const;

	/// \brief	this func sets the anim cycle to use for the model
	/// \param	cnum	-	the cycle number for the animation
	/// \return	true if OK
	/// 
	bool SetCycle(unsigned int cnum);

	/// \brief	this func sets the anim cycle to use for the model
	/// \param	cnum		-	the cycle number for the animation
	/// \param	over		-	the time in seconds over which the transition need to occur
	/// \param	FromStart	-	if true, the second anim will start at time 0. If false it will use 
	///							the current anims time as it's starting point. ie, for walk to a run
	///							you would probably want false since the anims can then be easily synched.
	///							for a walk to a jump, you'd probably want true since you want to blend from
	///							the start of the jump, not half way through !!
	/// \return	true if OK
	/// 
	bool SwitchCycle(unsigned int cnum, float over = 0.5f, bool FromStart = true, int loops = -1, int afterCycle = 0);


	//-------------------------------------------------------------------------------------------	Texture Stuff


	/// returns the number of skins in the model
	unsigned int GetNumSkins() const ;
	
	/// \brief	returns the name of the requested cycle
	/// \param	id	-	the index of the cycle to query
	/// \return	the name of the animation cycle
	/// 
	std::string GetSkinName(unsigned int idx) const;

	/// \brief	This function sets the requested skin to that specified within the
	///			md2 file. 
	bool SetSkin(unsigned int snum);
	
	/// \brief	This function loads the specified texture and applies it as the skin 
	///			for this texture. Use this if you want to overide the skin names contained
	///			within the Md2File
	/// 
	//bool SetTexture(std::string);


	//-------------------------------------------------------------------------------------------	Movement Controls


	/// \brief	This function offsets the position of the instance releative to the
	///			direction it is facing. You probably want to use this for in game
	///			movement of the characters.
	/// \param	x	-	the x offset (forward/back)
	/// \param	y	-	the y offste (up/down)
	/// \param	z	-	the z offset (left/right)
	/// \param	relative	-	if true moves in the direction the character is facing,
	///							otherwise adds the offset as world space offset
	///
	void Move(float x,float y,float z,bool Relative=true);
	
	///	\brief	This function offsets the current Y rotation by the specified amount
	/// \param	rotate	-	the current rotation amount
	/// 
	void Rotate(float rotate) {
		m_RotateY += rotate;
	}

	/// \param	This function sets the current position of the model. You probably
	///			only want to use this when setting the starting position for an insatnce
	///	\param	x	-	the x coord
	/// \param	y	-	the y coord
	/// \param	z	-	the z coord
	/// 
	void SetPosition(float x,float y,float z) {
		m_Position[0] = x;
		m_Position[1] = y;
		m_Position[2] = z;
	}

	/// \brief	This function sets the current rotation value of the model. You 
	///			probably only want to use this when setting the initial position 
	///			for the instance
	/// \param	rot	-	the current rotation amount
	/// 
	void SetRotation(float rot) {
		m_RotateY = rot;
	}

	/// \brief	returns the current position of the model at the current moment
	/// \return	the position array (3 floats; x,y,z)
	///
	const float* GetPosition() const {
		return m_Position;
	}

	/// \brief	returns the current Y rotation of the model at the current moment
	/// \return	the Y rotation
	///
	const float GetRotation() const {
		return m_RotateY;
	}

	//-------------------------------------------------------------------------------------------	Bounding Primitives

	
	#if MD2_CALCULATE_BBOX
		/// the instances bounding box
		const Md2BoundingBox& GetBoundingBox() const {
			return m_BBox;
		}
	#endif
			
	#if MD2_CALCULATE_BSPHERE
		/// the instances bounding box
		const Md2BoundingSphere& GetBoundingSphere() const {
			return m_BSphere;
		}
	#endif


// internal methods
//private:

	#if MD2_CALCULATE_BBOX
		/// the instances bounding box
		Md2BoundingBox m_BBox;
	#endif
	
	#if MD2_CALCULATE_BSPHERE
		/// the instances bounding sphere
		Md2BoundingSphere m_BSphere;
	#endif

	/// \brief	this function renders the model
	///
	void Render();
	
	/// \brief	returns the size in bytes of the data used
	/// \param	type	-	what memory type to query
	///
	unsigned int GetDataSize(Md2MemoryType type) const ;

	/// only a model can create an instance
	Md2Instance(Md2Model* mod, GLuint texture);

	/// dtor
	~Md2Instance() ;

	/// 
	void Update(float dt);

// data
//private:

	/// the current animation time
	float m_AnimTime;

	/// the previous animation time
	float m_PreviousTime;

	/// the total time for the transition to occur over
	float m_TransitionTime;
	
	/// the time for remaining transition
	float m_TimeRemaining;

	#if MD2_USE_VBO

		/// the VBO for the vertex (and normal data)
		unsigned int  m_VertexBuffer;

	#else

		/// the models evaluated vertices (as vertex array)
		Md2VertexNormal* m_VertexData;

	#endif

	/// pointer to the model that this instance references
	Md2Model* m_pModel;
	
	// if unsigned chars are used for data storage, we need to LERP the scale and translation
	// keys between the keyframes and apply them with glScale & glTranslate
	#if !MD2_USE_FLOATS

		/// the evaluated scale for the model
		float m_Scale[3];

		/// the evaluated translation for the model
		float m_Translate[3];
	#endif

	/// the position of the model (x,y,z)
	float m_Position[3];

	/// the instances y rotation
	float m_RotateY;
		
	/// the current skin IDX used by this model
	unsigned int m_CurrentSkin;
	
	/// the current anim cycle
	unsigned char m_CurrentAnim;
	
	int m_LoopAnim;
	int m_AfterCycle;
	
	/// the previous anim cycle (for transitions)
	unsigned char m_PreviousAnim;

	/// is the instance visible
	bool m_Visible;
	
	GLuint m_Texture;

}; // 55 bytes









//-----------------------------------------------------------------------------------------------	class Md2Model
// data thats const - shared by all model instances. Only the Md2Manager and Md2Instance class
// have the rights to access this classes internal gubbins
//
class Md2Model {
	friend class Md2Instance;
	friend class Md2Manager;
	friend void Md2ReleaseTex(unsigned int);
//private:

	Md2Model(const Md2Model&);
	Md2Model& operator=(const Md2Model&);
	
	/// ctor
	Md2Model();

	/// dtor
	~Md2Model();
	
	

	#if MD2_USE_NORMALS
		void CalcNormals();
	#endif

	/// \brief	used to create a new instance of the model
	/// \return a pointer to the new instance
	///
	Md2Instance* CreateInstance(GLuint);

	/// \brief	used to delete an instance of the model
	/// \param	instance	-	the instance to delete
	/// \return	true if deleted
	///
	bool DeleteInstance(Md2Instance*) ;

	/// \brief	expands the tex coord data from the original data to vertex array format
	/// \param	il	-	the temporary index list used to create the vertex array
	///
	void ExpandUvs(Md2VertexArrayIndexList& il);

	#if !MD2_ALWAYS_TRIANGLES
		void ExpandUvs(Md2VertexArrayIndexList2& il);
	#endif
	
	/// \brief	returns the size in bytes of the data used
	/// \param	type	-	what memory type to query
	///
	unsigned int GetDataSize(Md2MemoryType type) const ;

	/// \brief	returns the number of anim cycles
	///
	unsigned int GetNumCycles() const;

	/// \brief	returns the number of skins listed in the model
	///
	unsigned int GetNumSkins() const;


	/// \brief	This function loads the MD2 file and performs the vertex array conversion
	///			for the mesh data. 
	/// \param	filename	-	the file to load.
	/// \return	true if OK
	/// \note	This calls MD2::Load and MD2::Release
	///
	bool Load(foo *bar);

	#if !MD2_ALWAYS_TRIANGLES
	void MakeStrippedArray();
	#endif
	
	/// \brief	makes a vertex array from the data in the md2 file
	/// 
	void MakeVertexArray();

	/// \brief	draws all instances of this model
	/// 
	void Render(); 

	/// \brief	this func sets the number of frames per second that this cycle should use
	/// \param	the frames per second for the anim
	///
public: 
	void SetFps(unsigned int);
	void SetTexture(GLuint);

	
	/// \brief	this function updates the animation cycle
	/// \param	dt		-	time increment
	/// \param	off		-	the stagger offset for the updates
	/// \param	stagger	-	the stagger size for the updates
	/// 
	void Update(float dt,unsigned char off,unsigned char stagger);
	
//private:

	/// the number of cycles
	unsigned char m_NumCycles;

	/// the frames per second
	unsigned char m_FPS;

	/// the number of triangles
	unsigned short m_NumTris;

	/// the number of vertices
	unsigned short m_NumVerts;
	
	/// the numbert of vertices in vertex array
	unsigned short m_NumElements;

	//union {
		/// VBO for the tex coords
		unsigned int m_TexCoordBuffer;

		/// the tex coords, expanded to vertex array format. All instances will share 
		/// this data.
		Md2TexCoord* m_TexCoords;
	//};

	/// this allows us to expand the evaluated keyframe data into vertex arrays
	unsigned short* m_VertexMap;
	
	union {

		/// VBO for the index data
		unsigned int m_IndexBuffer;
	
		/// the vertex array indices
		unsigned short* m_Indices;
	};
	
	#if !MD2_ALWAYS_TRIANGLES
	
		/// the original point indices - only needed to calculate the vertex normals
		Md2StripInfo* m_StripCounts;

		/// 
		unsigned short m_NumStrips;

		///
		unsigned short m_NumIndices;
	
	#endif

	#if MD2_USE_NORMALS
		/// the original point indices - only needed to calculate the vertex normals
		unsigned short* m_OriginalIndices;
	#endif

	/// the anim cycles
	Md2AnimCycle* m_AnimCycles;

	/// an array of all instances of this model
	std::vector<Md2Instance*> m_Instances;

	/// an array of skin names used in the model
	std::vector<std::string> m_SkinNames;


};	// 68bytes + sizeof(std::vector<Md2Instance*>)

#pragma pack(pop)

#endif
