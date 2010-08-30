//------------------------------------------------------------------------------------
///	\file	MD2_File.h
///	\author	Rob Bateman [robthebloke@hotmail.com]
/// \date	30-12-2004
///	\brief	This file defines the structures within the MD2 file. This loader is not
///			something you want to try and use directly, DO NOT USE IT infact ;)
///			basically this is my attempt to provide a slightly more optimised loading
///			and rendering method for the MD2 files than you will often find in example
///			code on the internet.
///
///			This code does a single thing. It loads the MD2 file (in one go) into a 
///			global buffer. All access to the MD2's data is then done by the use of 
///			pointer offsets into this buffer. Basically this cuts down on file access
///			times to speed up the loading a bit. 
///
///			The data is handled in this way becuase the data needs to be transformed 
///			into something mildly more optimial for the final rendering with OpenGL 
///			vertex arrays. The original data is no longer something we care too much
///			about since the final representation will be completely different.
///			
///			The MD2Model class will call MD2::Load() to load the data, and MD2::Release
///			to kill it off. None of this stuff would be used by someone wanting to 
///			load an animated MD2 file.
/// 
//------------------------------------------------------------------------------------

#ifdef WIN32
	#pragma once
#endif

#ifndef MD2__FILE__H__
#define MD2__FILE__H__

#pragma pack(push,1)

#include <stdio.h>





#ifdef ANDROID_NDK
#include <android/log.h>
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "libnav", __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "libnav", __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "libnav", __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "libnav", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "libnav", __VA_ARGS__) 
#endif




#ifndef LOGV
#define LOGV printf
#endif

namespace MD2 {

	//-------------------------------------------------------------------------------
	// forward declarations of the MD2 types
	// 
	
	struct model;
	struct frame;
	struct triangle;
	struct uv;
	struct glCommandList;


	//-------------------------------------------------------------------------------
	// funcs that load and release the MD2 file (in a global buffer)
	// 

	/// this loads an MD2 into a single buffer
	bool Load(FILE* filename, unsigned int off, unsigned int len);

	/// the releases the data when we are done with it
	void Release();	

	//-------------------------------------------------------------------------------
	// funcs that use pointer offsets to return parts of the currently loaded MD2.
	// this really does nothing else!
	// 

	const model*		 GetModel();
	const frame*		 GetFrame(unsigned int num);
	const char*			 GetSkin(unsigned int num);
	const triangle*		 GetTriangles();
	const uv*			 GetTexCoords();
	unsigned char*		 GetCommandsStart();
	unsigned char*		 GetCommandsEnd();
	

	//-------------------------------------------------------------------------------
	// MD2 data types
	// 

	struct model
	{ 
		int magic; 
		int version; 
		int skinWidth; 
		int skinHeight; 
		int frameSize; 
		int numSkins; 
		int numVertices; 
		int numTexCoords; 
		int numTriangles; 
		int numGlCommands; 
		int numFrames; 
		int offsetSkins; 
		int offsetTexCoords; 
		int offsetTriangles; 
		int offsetFrames; 
		int offsetGlCommands; 
		int offsetEnd; 
	};
	
	struct triangle
	{
		short vertexIndices[3];
		short textureIndices[3];
	};

	struct uv
	{
		short s;
		short t;
	};

	struct vertex
	{
		unsigned char vert[3];
		unsigned char normalIndex;
	};

	struct frame
	{
		float scale[3];
		float translate[3];
		char  name[16];
		vertex vertices[1];
	};

	struct glCommandVertex
	{
		float s;
		float t;
		int vertexIndex;
	};

	struct glCommandList
	{
		int num;
		glCommandVertex verts[1];

		inline int GetNum() const {
			if(num<0) return -num;
			return num;
		}
		inline unsigned int size() const {
			return static_cast<unsigned int>(4 + GetNum()*sizeof(glCommandVertex));
		}
	};

}

#pragma pack(pop)

#endif
