//-------------------------------------------------------------------------------------
/// \file	MD2_Config.h
/// \author	Rob Bateman	[robthebloke@hotmail.com]
/// \date	30-12-2004
/// \brief	A set of compile time flags that determine how the MD2 loader works internally.
///			If you are using VBO, you should make sure you have glew available (and
///			have called glewInit() before loading a model )
///
//-------------------------------------------------------------------------------------

#ifndef MD2__CONFIG__H__
#define MD2__CONFIG__H__

	#ifdef WIN32
		#pragma once
	#endif

	//---------------------------------------------------------------------------------	Md2 Compile Flags

	/// set this flag to zero if you dont want to use normals. The downsides to normals
	/// are that the update time is slightly increased and the memory requirements
	/// grow a little bit. Given modern hardware however, this is not a big problem.
	/// (usually an instance with normals is about ~20kb, without that requirement
	/// halves).
	/// You only really want this as 0 if you are not using lighting for your models
	///

	//#define MD2_USE_NORMALS 0

	/// if 0, the code uses vertex arrays. If 1, vertex buffer objects are used. Using 
	/// VBO's may increase the total memory usage since some temporary data buffers are
	/// needed to update the VBO's. There is however a 10 to 20% performance increase.
	/// Unless you need to handle ancient graphics cards, you probably want to leave this as 1
	///
	/// I've had loads of problems with VBO's under linux using nvidia cards. I can only
	/// seem to create a single VBO under linux (which is a bit cack). You might therefore
	/// want to make it use normal vertex arrays if the code crashes for any random reasons.
	///
	#define MD2_USE_VBO 0

	/// set this flag to 1 for compressed textures, 0 for normal RGB,RGBA texture data
	///
	#define MD2_USE_COMPRESSED_TEXTURES 1

	/// Some Md2 files contain optimised lists of triangle strips / triangle fans.
	/// The process for conversion from this data to the vertex array data is slightly
	/// different. It's debatable whether it will actually make any performance difference,
	/// my tests indicate that the MD2 file is usually quicker when rendering it purely
	/// as triangles. (ie, this set to 1). Strips and fans seem to slow it down a little
	/// bit, and give it a more erratic frame rate. I suppose this depends on how the model
	/// was stripped in the first place. You *may* find this performs better if you use a
	/// single triangle strip joined with degenerate triangles. Toggle the flag about and
	/// see how it performs i guess....
 	///
	#define MD2_ALWAYS_TRIANGLES 1

	/// set this flag to 1 if you want the animation data to be stored as floats. 
	/// if set to 0, the keyframes are stored as unsigned char's, and the keyframes scale
	/// and translation keys are also stored and interpolated. The plus point is that
	/// it saves on data storage (up to 2 thirds). The cost is a little bit of a speed overhead
	/// since the scale & translation keys also need to be LERPED and a number of unsigned
	/// chars need to be converted to floats.
	/// 
	/// THERE IS LITTLE POINT IN USING FLOATS. T0ere is a *small* speed increase, however the
	/// data requirements of the md2 file increases by a factor of 4 (or there abouts)
	///
	#define MD2_USE_FLOATS 0

	/// set this flag to 1 to calculate a bounding box for the model instances. This is
	/// pretty much free if the MD2_USE_FLOATS flag is set to zero. It takes a little
	/// bit of overhead otherwise....
	///
	#define MD2_CALCULATE_BBOX 0

	/// set this flag to 1 to calculate a bounding sphere for the model instances. If
	/// this is set to 1, the bounding box will also be calculated whether you like it
	/// or not !!  :)
	/// This is slightly more computationally expensive when MD2_USE_FLOATS is zero.
	///
	#define MD2_CALCULATE_BSPHERE 0

	/// unless you need it for debugging purposes, leave this on zero!!
	///
	#define MD2_RENDER_BOUNDING_PRIMITIVES 0


	//---------------------------------------------------------------------------------	

	// my hacky method of calculating bounding spheres involves the use of bounding boxes.
	#if (MD2_CALCULATE_BSPHERE && !MD2_CALCULATE_BBOX)
		#undef MD2_CALCULATE_BBOX
		#define MD2_CALCULATE_BBOX 1
	#endif

	// define an internal data buffer size - used to store data prior to uploading to
	// VBO's on graphics card and to blend 2 cycles as they are switching...
	#if MD2_USE_VBO
		#define MD2_BUFFER_SIZE 3072
	#else 
		#define MD2_BUFFER_SIZE 2048
	#endif

	// define the data type used for keyframe vertex data
	//
	#if MD2_USE_FLOATS
		#define MD2_DATA_TYPE float
	#else
		#define MD2_DATA_TYPE unsigned char
	#endif

#endif
