

#include "MD2_File.h"

#include <stdio.h>


namespace MD2 {

	static unsigned char* g_data = 0;
	
	const model* GetModel() {
		void *p=g_data;
		return reinterpret_cast<model*>( p );
	}

	const frame* GetFrame(unsigned int num) {
		const model* pm = GetModel();
		void* ptr = g_data + pm->offsetFrames + (num*pm->frameSize);
		return reinterpret_cast<frame*>(ptr);
	}

	const triangle* GetTriangles() {
		const model* pm = GetModel();
		void* ptr = g_data + pm->offsetTriangles;
		return reinterpret_cast<triangle*>(ptr);
	}
	
	const uv* GetTexCoords() {
		const model* pm = GetModel();
		void* ptr = g_data + pm->offsetTexCoords;
		return reinterpret_cast<uv*>(ptr);
	}
	
	unsigned char* GetCommandsStart() {
		const model* pm = GetModel();
		return g_data + pm->offsetGlCommands;
	}

	unsigned char* GetCommandsEnd() {
		const model* pm = GetModel();
		return g_data + pm->offsetGlCommands + 4*pm->numGlCommands;
	}
	
	const char*	GetSkin(unsigned int num) {
		void* p = g_data + GetModel()->offsetSkins + num*64;
		return reinterpret_cast<char*>(p);
	}

	
	// this loads an MD2 into a single buffer
	bool Load(foo *bar) {
		FILE *fp = bar->fp;
		unsigned int off = bar->off;
		unsigned int len = bar->len;
		fseek(fp, off, SEEK_SET);
		g_data = new unsigned char[len];
		fread(g_data, sizeof(unsigned char), len ,fp);
		if( GetModel()->magic != 0x32504449 || GetModel()->version != 8 ) {
			LOGV("MD2::Load fp is NOT a md2");
			Release();
			return false;
		}
		return true;	
	}


	// free allocated data
	void Release() {
		delete [] g_data;
		g_data=0;
	}
}