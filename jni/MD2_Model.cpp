//-------------------------------------------------------------------------------------
/// \file	MD2_Model.cpp
/// \author	Rob Bateman	[robthebloke@hotmail.com]
/// \date	02-Jan-2005
/// \brief	Sorry for the rather excessive use of #ifdefs, doesn't make the code too
///			easy to follow. Should have used templates :|
///			
//-------------------------------------------------------------------------------------

#include <iostream>
#include <algorithm>

#include "MD2_Manager.h"
#include "MD2_Model.h"
#include "MD2_File.h"

//
// texture loading funcs defined in MD2_Manager.cpp
//
//extern unsigned int Md2LoadTex(NSString *);
//extern void Md2ReleaseTex(unsigned int);

//-----------------------------------------------------------------------------------------------	Global Data

// a global buffer used as a temp buffer when evaluating the anim cycles and calculating the 
// new normals
// 
Md2VertexNormal g_VertBuffer[2048];

// we need a temporary buffer for 2 reasons. 
//
// 1. We need to expand the vertex and normal data before uploading to a VBO
// 2. When blending the cycles, we effectively have to evaluate two cycles, then merge
//
// The size of the buffer changes depending on which flags are set in MD2_Config.h
//
Md2VertexNormal g_TempBuffer[ MD2_BUFFER_SIZE ];


/// a global to see if we need to change the texture currently bound
unsigned int g_LastBound=0;

/*
#if MD2_USE_NORMALS
inline void CrossProduct(float out[],float a[],float b[]) {
	out[0] = ( a[1] * b[2]  -  a[2] * b[1] );
	out[1] = ( a[2] * b[0]  -  a[0] * b[2] );
	out[2] = ( a[0] * b[1]  -  a[1] * b[0] );
}
#endif
 */


//-----------------------------------------------------------------------------------------------	class Md2AnimCycle
//###############################################################################################
//-----------------------------------------------------------------------------------------------




//-----------------------------------------------------------------------------------------------	Md2AnimCycle :: Md2AnimCycle
//
Md2AnimCycle::Md2AnimCycle() 
	: m_KeyFrameData(0),m_NumFrames(0)
{
	//NSLog("new anim cycle");
}

//-----------------------------------------------------------------------------------------------	Md2AnimCycle :: ~Md2AnimCycle
//
Md2AnimCycle::~Md2AnimCycle() {
	//NSLog(" free anim cycle ");
	delete m_ScaleKeys;
	delete m_TranslateKeys;
	delete [] m_KeyFrameData;
}

//-----------------------------------------------------------------------------------------------	Md2AnimCycle :: Update
//
#if MD2_USE_FLOATS
void Md2AnimCycle::Update(unsigned char firstFrameIdx,unsigned short numV,float t) { 
#else
void Md2AnimCycle::Update(unsigned char firstFrameIdx,unsigned short numV,float t,float scale[3],float translate[3]) {
#endif

	#if !MD2_USE_FLOATS
		float* firstScale;
		float* secondScale;
		float* firstTranslate;
		float* secondTranslate;
	#endif

	Md2Vertex* secondFrame = 0;
	if(firstFrameIdx>=(m_NumFrames-1)) {
		secondFrame = m_KeyFrameData;
		#if !MD2_USE_FLOATS
			secondScale     = m_ScaleKeys;
			secondTranslate = m_TranslateKeys;
		#endif
	}

	// iterators through first vertex to last vertex
	Md2Vertex* firstFrame = m_KeyFrameData + (firstFrameIdx*numV);
	Md2Vertex* frameEnd   = m_KeyFrameData + ((firstFrameIdx+1)*numV);
	
	#if !MD2_USE_FLOATS
		firstScale     = m_ScaleKeys + (firstFrameIdx)*3;
		firstTranslate = m_TranslateKeys + (firstFrameIdx)*3;
	#endif

	// get pointer to second frame
	if(!secondFrame) {
		secondFrame = frameEnd;
		#if !MD2_USE_FLOATS
			secondScale     = firstScale + 3;
			secondTranslate = firstTranslate + 3;
		#endif
	}

	#if !MD2_USE_FLOATS
		scale[0] = firstScale[0] + t * ( secondScale[0] - firstScale[0] );
		scale[1] = firstScale[1] + t * ( secondScale[1] - firstScale[1] );
		scale[2] = firstScale[2] + t * ( secondScale[2] - firstScale[2] );

		translate[0] = firstTranslate[0] + t * ( secondTranslate[0] - firstTranslate[0] );
		translate[1] = firstTranslate[1] + t * ( secondTranslate[1] - firstTranslate[1] );
		translate[2] = firstTranslate[2] + t * ( secondTranslate[2] - firstTranslate[2] );
	#endif


	// update vertices into the global buffer
	Md2VertexNormal* outputVertex = g_VertBuffer;

	// loop through all points
	for( ; firstFrame != frameEnd; ++firstFrame,++secondFrame,++outputVertex)
	{
		// do interpolation 
		outputVertex->vx =  firstFrame->vx + t * static_cast<float>( secondFrame->vx - firstFrame->vx );
		outputVertex->vy =  firstFrame->vy + t * static_cast<float>( secondFrame->vy - firstFrame->vy );
		outputVertex->vz =  firstFrame->vz + t * static_cast<float>( secondFrame->vz - firstFrame->vz );

/*
		// zero normals so we can calc normals 
		#if MD2_USE_NORMALS
			outputVertex->nx = outputVertex->ny = outputVertex->nz = 0.0f;
		#endif
*/
		
	}
}





//-----------------------------------------------------------------------------------------------	class Md2Model
//###############################################################################################
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------	Md2Model :: Md2Model
//
Md2Model::Md2Model()
	: m_FPS(5),
	m_NumTris(0),
	m_NumVerts(0),
	m_NumElements(0),
	m_NumCycles(0),
	m_SkinNames(),
	m_TexCoords(0),
	m_VertexMap(0),
	m_Indices(0),
	//#if MD2_USE_NORMALS
	//m_OriginalIndices(0),
	//#endif
	#if !MD2_ALWAYS_TRIANGLES
	m_StripCounts(0),
	m_NumStrips(0),
	m_NumIndices(0),
	#endif
	m_AnimCycles(0),
	m_Instances(0) 

{
}

//-----------------------------------------------------------------------------------------------	Md2Model :: ~Md2Model
// dtor
Md2Model::~Md2Model() {
	delete [] m_AnimCycles;
	#if !MD2_ALWAYS_TRIANGLES
	delete [] m_StripCounts;
	#endif

/*
#if MD2_USE_NORMALS
	delete [] m_OriginalIndices;
#endif
*/
	
	delete [] m_VertexMap;

	#if MD2_USE_VBO
	glDeleteBuffers(1, &m_TexCoordBuffer);
	glDeleteBuffers(1,&m_IndexBuffer);
		//glDeleteBuffersARB(1,&m_TexCoordBuffer);
		//glDeleteBuffersARB(1,&m_IndexBuffer);
	#else
		delete [] m_TexCoords;
		delete [] m_Indices;
	#endif

	std::vector<Md2Instance*>::iterator it = m_Instances.begin();
	for( ; it != m_Instances.end(); ++it ) {
		delete *it;
	}
}


/*
#if MD2_USE_NORMALS
void Md2Model::CalcNormals() {

	unsigned short* ptr = m_OriginalIndices;
	unsigned short* end = ptr + 3*m_NumTris;

	for( ; ptr != end; ptr+=3 )
	{
		float ab[3];
		float ac[3];

		Md2VertexNormal* a = g_VertBuffer + ptr[0];
		Md2VertexNormal* b = g_VertBuffer + ptr[1];
		Md2VertexNormal* c = g_VertBuffer + ptr[2];

		ab[0] = b->vx - a->vx; 
		ab[1] = b->vy - a->vy;
		ab[2] = b->vz - a->vz;

		ac[0] = c->vx - a->vx; 
		ac[1] = c->vy - a->vy;
		ac[2] = c->vz - a->vz;

		float cross[3];
		CrossProduct(cross,ab,ac);

		float l = 1.0f/sqrt( cross[0]*cross[0] + cross[1]*cross[1] + cross[2]*cross[2] );
		cross[0]*=l;
		cross[1]*=l;
		cross[2]*=l;

		a->nx += cross[0];
		a->ny += cross[1];
		a->nz += cross[2];

		b->nx += cross[0];
		b->ny += cross[1];
		b->nz += cross[2];

		c->nx += cross[0];
		c->ny += cross[1];
		c->nz += cross[2];
	}

}
#endif
*/
	

//-----------------------------------------------------------------------------------------------	Md2Model :: CreateInstance
//
Md2Instance* Md2Model::CreateInstance() {
	Md2Instance* newInstance = new Md2Instance(this);
	m_Instances.push_back(newInstance);
	return newInstance;
}

//-----------------------------------------------------------------------------------------------	Md2Model :: DeleteInstance
//
bool Md2Model::DeleteInstance(Md2Instance* instance) {
	// find instance to delete
	std::vector<Md2Instance*>::iterator it = m_Instances.begin();
	for(; it != m_Instances.end(); ++it ) {

		if( *it == instance )
		{
			// delete instance data
			delete *it;

			// remove instance
			m_Instances.erase(it);
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------------------------	Md2Model :: ExpandUvs
//
void Md2Model::ExpandUvs(Md2VertexArrayIndexList& tempIndices)
{
	//
	Md2TexCoord* oldData = m_TexCoords;

	// allocate new texture coord array
	m_TexCoords = new Md2TexCoord[ tempIndices.size() ];
	
	Md2TexCoord* newDataIterator = m_TexCoords;

	// use temp index list to expand the data 
	Md2VertexArrayIndexList::iterator indexIterator = tempIndices.begin();
	for( ; indexIterator != tempIndices.end(); ++indexIterator,++newDataIterator ) {
		newDataIterator->u = oldData[ indexIterator->t ].u;
		newDataIterator->v = oldData[ indexIterator->t ].v;
	}

	// delete original uv coords - dont need it anymore
	delete [] oldData;
}

//-----------------------------------------------------------------------------------------------	Md2Model :: ExpandUvs
//
#if !MD2_ALWAYS_TRIANGLES
void Md2Model::ExpandUvs(Md2VertexArrayIndexList2& tempIndices)
{
	// allocate new texture coord array
	m_TexCoords = new Md2TexCoord[ tempIndices.size() ];
	
	Md2TexCoord* newDataIterator = m_TexCoords;

	// use temp index list to expand the data 
	Md2VertexArrayIndexList2::iterator indexIterator = tempIndices.begin();
	for( ; indexIterator != tempIndices.end(); ++indexIterator,++newDataIterator ) {
		newDataIterator->u = indexIterator->uv[0];
		newDataIterator->v = indexIterator->uv[1];
	}
}
#endif

//-----------------------------------------------------------------------------------------------	Md2Model :: GetNumCycles
//
unsigned int Md2Model::GetNumCycles() const{
	return m_NumCycles;
}

//-----------------------------------------------------------------------------------------------	Md2Model :: GetNumSkins
//
unsigned int Md2Model::GetNumSkins() const{
	return static_cast<unsigned int>( m_SkinNames.size() );
}


//-----------------------------------------------------------------------------------------------	Md2Model :: GetDataSize
//
unsigned int Md2Model::GetDataSize(Md2MemoryType type) const {
	unsigned int sz=0;

	if(type == MEM_MODEL || type == MEM_ALL )
	{
		#if !MD2_USE_VBO
			sz += sizeof(Md2TexCoord)*m_NumElements;

			// if using stripped data, we will have a different size index array
			#if !MD2_ALWAYS_TRIANGLES
			if(m_StripCounts) {
				sz += sizeof(unsigned short)*m_NumIndices;
			}
			else
			#endif
			{
				sz += sizeof(unsigned short)*3*m_NumTris; // index array
			}
		#endif
			
		#if !MD2_ALWAYS_TRIANGLES
			if(m_StripCounts) {
				sz += sizeof(Md2StripInfo)*m_NumStrips;
			}
		#endif

		sz += sizeof(unsigned short)*m_NumElements; // vertex map
		
/*
		#if MD2_USE_NORMALS
			// original indices array size
			sz += sizeof(unsigned short)*3*m_NumTris;
		#endif
*/
		
		unsigned int i=0;
		for( ; i != m_NumCycles; ++i )
		{
			sz += m_AnimCycles[i].GetDataSize(m_NumVerts);
		}

		sz += sizeof(Md2Model);
	}

	if(type == MEM_INSTANCE || type == MEM_ALL ) {
		unsigned int i=0;
		for( ; i != m_Instances.size(); ++i )
		{
			sz += m_Instances[i]->GetDataSize(type);
		}
	}

	if( type == MEM_MODEL_VBO || type == MEM_ALL ) {
		#if MD2_USE_VBO
			sz += sizeof(Md2TexCoord)*m_NumElements;
			
			// include index data size
			#if !MD2_ALWAYS_TRIANGLES
				if(m_StripCounts) {
					sz += sizeof(unsigned short)*m_NumIndices;
				}
				else
			#endif
				sz += sizeof(unsigned short)*3*m_NumTris;

		#endif
	}

	if( type == MEM_INSTANCE_VBO ) {
		unsigned int i=0;
		for( ; i != m_Instances.size(); ++i )
		{
			sz += m_Instances[i]->GetDataSize(type);
		}
	}
	return sz;
}

//-----------------------------------------------------------------------------------------------	Md2Model :: Load
//
bool Md2Model::Load(foo *bar) {
	if(!MD2::Load(bar)) {
		LOGV("failed to MD2::Load file");
		return false;
	}

	// copy over skin names
	{
		unsigned int nSkins = MD2::GetModel()->numSkins;
		for( unsigned int i=0 ; i != nSkins; ++i ) {
			m_SkinNames.push_back(MD2::GetSkin(i));
		}
	}

	// copy uv coord data
	#if !MD2_ALWAYS_TRIANGLES
	if(MD2::GetModel()->numGlCommands<2)
	#endif
	{
		m_TexCoords = new Md2TexCoord[ MD2::GetModel()->numTexCoords ];
		assert(m_TexCoords);

		Md2TexCoord* ptr = m_TexCoords;
		Md2TexCoord* end = m_TexCoords + MD2::GetModel()->numTexCoords;
		unsigned int i=0;
		for( ; ptr != end; ++i, ++ptr )
		{
			ptr->FromMd2( (MD2::GetTexCoords())[i] , MD2::GetModel()->skinWidth, MD2::GetModel()->skinHeight );
		}
	}
	
	#if !MD2_ALWAYS_TRIANGLES
	if(MD2::GetModel()->numGlCommands >1)
	{
		MakeStrippedArray();
	}
	else
	#endif
	{
		MakeVertexArray();
	}

	// create a static VBO for the texture data if needed
	#if MD2_USE_VBO
		unsigned int VBO=0;
	//JAB
		//glGenBuffersARB( 1, &VBO );
	glGenBuffers(1, &VBO);
	
		//glBindBufferARB( GL_ARRAY_BUFFER_ARB, VBO );
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

		// set VBO size and upload data
		//glBufferDataARB( GL_ARRAY_BUFFER_ARB, m_NumElements*2*sizeof(float), m_TexCoords, GL_STATIC_DRAW_ARB );
	glBufferData(GL_ARRAY_BUFFER, m_NumElements*2*sizeof(float), m_TexCoords, GL_STATIC_DRAW);

		// delete local texture data
		delete [] m_TexCoords;

		// assign VBO
		m_TexCoordBuffer = VBO;

		// now do the same for the index data

		unsigned int indexDataSize = 3*m_NumTris;
		
		#if !MD2_ALWAYS_TRIANGLES
		if(MD2::GetModel()->numGlCommands>1)
		{
			indexDataSize = m_NumIndices;
		}
		#endif

		//JAB
	
	
		//glGenBuffersARB( 1, &VBO );
	glGenBuffers(1, &VBO);
	
		//glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, VBO );
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO);

		// set VBO size and upload data
		//glBufferDataARB( GL_ELEMENT_ARRAY_BUFFER_ARB, indexDataSize*sizeof(unsigned short), m_Indices, GL_STATIC_DRAW_ARB );
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSize*sizeof(unsigned short), m_Indices, GL_STATIC_DRAW);

		// delete local index data
		delete [] m_Indices;

		// assign VBO
		m_IndexBuffer = VBO;

	#endif
		
	// get number of frames
	unsigned int nf = MD2::GetModel()->numFrames;


	// determine number of frames per anim
	std::vector<unsigned int> counts;
	{
		// quick name lookup for frame - num
		std::string firstName="";

		// loop through all frames to find counts
		for( unsigned int i=0 ; i != nf; ++i )
		{
			const MD2::frame* thisFrame = MD2::GetFrame(i);
			if(firstName=="" || strncmp(firstName.c_str(),thisFrame->name,firstName.size()) != 0 )
			{
				firstName = thisFrame->name;
				if(firstName[firstName.size()-2]=='0')
					firstName.resize(firstName.size()-2);
				else 
					firstName.resize(firstName.size()-1);
				counts.push_back(1);
			}
			else {
				counts[counts.size()-1]++;
			}
		}
	}
	m_NumCycles = static_cast<unsigned short>(counts.size());
	m_NumVerts = MD2::GetModel()->numVertices;

	// allocate anim cycle data
	m_AnimCycles = new Md2AnimCycle[m_NumCycles];
	assert(m_AnimCycles);
	unsigned int frameIndex=0;

	Md2AnimCycle* animIterator = m_AnimCycles;
	
	std::vector<unsigned int>::iterator it = counts.begin();
	for( ; it != counts.end(); ++animIterator, ++it )
	{
		// copy name from first frame of animation
		{
 			strcpy(animIterator->GetName(),MD2::GetFrame(frameIndex)->name);

			// get name length
			unsigned int len = static_cast<unsigned int>(strlen(animIterator->GetName()));

			// either name01 or name1. strip number bit
			if( animIterator->GetName()[ len - 2 ] == '0' ) 
				animIterator->GetName()[ len - 2 ] = '\0';
			else
				animIterator->GetName()[ len - 1 ] = '\0';
		}

		//print std::cout << animIterator->GetName() << " " << *it << std::endl;

		// allocate storage for this frame
		animIterator->SetSize(MD2::GetModel()->numVertices , (*it));
		Md2Vertex* vertexIterator = animIterator->m_KeyFrameData;
		#if !MD2_USE_FLOATS
			float* ps = animIterator->m_ScaleKeys;
			float* pt = animIterator->m_TranslateKeys;
		#endif

		// loop through each frame in cycle
		for(unsigned int j=0;j!=(*it);++j,++frameIndex)
		{

			const MD2::frame* pf = MD2::GetFrame(frameIndex);
			#if !MD2_USE_FLOATS
				ps[0] = pf->scale[0];
				ps[2] = pf->scale[1];
				ps[1] = pf->scale[2];
				pt[0] = pf->translate[0];
				pt[2] = pf->translate[1];
				pt[1] = pf->translate[2];
				pt += 3;
				ps += 3;
			#endif
			// copy over each vertex position (and scale correctly)
			for(unsigned int k=0;k!=MD2::GetModel()->numVertices; ++k, ++vertexIterator ) {
				#if MD2_USE_FLOATS
					vertexIterator->FromMd2( pf->vertices[k], pf->scale, pf->translate );
				#else
					vertexIterator->FromMd2( pf->vertices[k] );
				#endif
			}
		}		

		//Md2AnimCycle* ppp = animIterator;
	}

	// free statically allocated data
	MD2::Release();

	return true;
}


#if !MD2_ALWAYS_TRIANGLES
void Md2Model::MakeStrippedArray() {
	Md2VertexArrayIndexList2 tempIndexLookup;

	unsigned char* pGL  = MD2::GetCommandsStart();
	unsigned char* pGLe = MD2::GetCommandsEnd();

	//
	std::vector<Md2StripInfo> Counts;
	std::vector<unsigned short> Indices;

	// for each triangle...
	for( ; pGL != pGLe;  ) {
		MD2::glCommandList* pcommandlist = (MD2::glCommandList*)((void*)pGL);
		pGL += pcommandlist->size();

		Md2StripInfo info;
		info.isFan = pcommandlist->num < 0;
		info.num   = pcommandlist->GetNum();

		if(info.num==0)
			continue;

		Counts.push_back(info);

		#if MD2_USE_FLOATS
		if(info.isFan) 
		{
			MD2::glCommandVertex* cv = pcommandlist->verts + pcommandlist->GetNum();
			MD2::glCommandVertex* cve = pcommandlist->verts;

			// loop over each index
			while( cv != cve ) {
				--cv;
				
				Md2VertexArrayIndex2 ind;

				// create index
				ind.uv[0] = cv->s;
				ind.uv[1] = cv->t;
				ind.v = cv->vertexIndex;

				// store vertex array index
				Indices.push_back( tempIndexLookup.Insert(ind) );
			}
		}
		else
		{
			MD2::glCommandVertex* cv = pcommandlist->verts;
			MD2::glCommandVertex* cve = cv + pcommandlist->GetNum();

			Md2VertexArrayIndex2 ind;
			
			// create index
			ind.uv[0] = cv->s;
			ind.uv[1] = cv->t;
			ind.v = cv->vertexIndex;

			// store vertex array index
			Indices.push_back( tempIndexLookup.Insert(ind) );

			++cv;

			// loop over each index
			for( ; cv < cve; cv+=2 ) {


				if( cv + 1 < cve ) {
					// create index
					ind.uv[0] = cv[1].s;
					ind.uv[1] = cv[1].t;
					ind.v = cv[1].vertexIndex;

					// store vertex array index
					Indices.push_back( tempIndexLookup.Insert(ind) );
				}

				// create index
				ind.uv[0] = cv->s;
				ind.uv[1] = cv->t;
				ind.v = cv->vertexIndex;

				// store vertex array index
				Indices.push_back( tempIndexLookup.Insert(ind) );
			}
		}
		#else
		{
			MD2::glCommandVertex* cv = pcommandlist->verts;
			MD2::glCommandVertex* cve = cv + pcommandlist->GetNum();

			// loop over each index
			for( ; cv != cve; ++cv ) {
				
				Md2VertexArrayIndex2 ind;

				// create index
				ind.uv[0] = cv->s;
				ind.uv[1] = cv->t;
				ind.v = cv->vertexIndex;

				// store vertex array index
				Indices.push_back( tempIndexLookup.Insert(ind) );
			}
		}
		#endif
	}



	// create a mapping from original points to vertex array points
	m_VertexMap = new unsigned short[ tempIndexLookup.size() ];
	unsigned short* pmap = m_VertexMap;
	Md2VertexArrayIndexList2::iterator it =  tempIndexLookup.begin();
	for( ; it != tempIndexLookup.end(); ++it,++pmap )
	{
		*pmap = it->v;
	}

	m_NumElements = static_cast<unsigned short>(tempIndexLookup.size());
	m_Indices	  = new unsigned short [ Indices.size() ];

	m_NumIndices = static_cast<unsigned short>(Indices.size());

	// copy over indices
	for(unsigned int i=0;i<Indices.size();++i) {
		m_Indices[i] = Indices[i];
	}

	m_NumStrips = static_cast<unsigned short>(Counts.size());
	
	// allocate strip info
	m_StripCounts = new Md2StripInfo[ m_NumStrips ];
	for(unsigned int i=0;i<Counts.size();++i) {
		m_StripCounts[i] = Counts[i];
	}
		
	m_NumTris = MD2::GetModel()->numTriangles;

	// allocate memory for original indices and vertex array indices

/*
	#if MD2_USE_NORMALS
		m_OriginalIndices = new unsigned short [ m_NumTris*3 ];
		unsigned short* poi = m_OriginalIndices;

		for( unsigned int i=0 ; i != m_NumTris; ++i, poi+=3 ) {

			// get the triangle
			const MD2::triangle* tri = MD2::GetTriangles()+i;

			#if MD2_USE_FLOATS				
				poi[0] = tri->vertexIndices[2];
				poi[1] = tri->vertexIndices[1];
				poi[2] = tri->vertexIndices[0];
			#else
				poi[0] = tri->vertexIndices[0];
				poi[1] = tri->vertexIndices[1];
				poi[2] = tri->vertexIndices[2];
			#endif
		}

	#endif
*/

	// convert original uv data into vertex array format
	ExpandUvs(tempIndexLookup);

}
#endif

//-----------------------------------------------------------------------------------------------	Md2Model :: MakeVertexArray
//
void Md2Model::MakeVertexArray() {
	
	Md2VertexArrayIndexList tempIndexLookup;

	m_NumTris = MD2::GetModel()->numTriangles;

	// allocate memory for original indices and vertex array indices
	
	#if MD2_USE_NORMALS
		m_OriginalIndices = new unsigned short [ m_NumTris*3 ];
	#endif
	
	m_Indices		  = new unsigned short [ m_NumTris*3 ];

	// iterators for index lists

	#if MD2_USE_NORMALS
		unsigned short* poi = m_OriginalIndices;
	#endif
	
	unsigned short* pi = m_Indices;

	// for each triangle...
	unsigned int i=0;
	for( ; i != m_NumTris; ++i ) {

		// get the triangle
		const MD2::triangle* tri = MD2::GetTriangles()+i;

		#if MD2_USE_FLOATS
			unsigned short texInds[3];
			unsigned short vrtInds[3];
			texInds[0] = tri->textureIndices[2];
			texInds[1] = tri->textureIndices[1];
			texInds[2] = tri->textureIndices[0];
			
			vrtInds[0] = tri->vertexIndices[2];
			vrtInds[1] = tri->vertexIndices[1];
			vrtInds[2] = tri->vertexIndices[0];
		#else
			const short* texInds = tri->textureIndices;
			const short* vrtInds = tri->vertexIndices;
		#endif

		// loop over each index
		for(unsigned int j=0;j!=3;++j,++pi) {
			
			Md2VertexArrayIndex ind;

			// create index
			ind.t = texInds[j];
			ind.v = vrtInds[j];

			#if MD2_USE_NORMALS
				// copy original point index
				*poi = ind.v;
				++poi;
			#endif	

			// store vertex array index
			*pi = tempIndexLookup.Insert(ind);
		}
	}



	// create a mapping from original points to vertex array points
	m_VertexMap = new unsigned short[ tempIndexLookup.size() ];
	unsigned short* pmap = m_VertexMap;
	Md2VertexArrayIndexList::iterator it =  tempIndexLookup.begin();
	for( ; it != tempIndexLookup.end(); ++it,++pmap )
	{
		*pmap = it->v;
	}

	m_NumElements = static_cast<unsigned short>(tempIndexLookup.size());

	// convert original uv data into vertex array format
	ExpandUvs(tempIndexLookup);
}


//-----------------------------------------------------------------------------------------------	Md2Model :: Render
//
void Md2Model::Render() {

	// uv coord data from the model

	glTexCoordPointer(2,GL_FLOAT,0, m_TexCoords);

	for( unsigned int i=0 ; i != m_Instances.size(); ++i )
	{
		m_Instances[i]->Render();
	}

}

//-----------------------------------------------------------------------------------------------	Md2Model :: SetFps
//
void Md2Model::SetFps(unsigned int fps){
	m_FPS = fps;
}

//-----------------------------------------------------------------------------------------------	Md2Model :: Update
//
void Md2Model::Update(float dt,unsigned char offset,unsigned char stagger) {

	std::vector<Md2Instance*>::iterator it = m_Instances.begin() + offset;
	for( ; it < m_Instances.end(); it+=stagger ) {
		(*it)->Update(dt);
	}
}



//-----------------------------------------------------------------------------------------------	class Md2IndexList
//###############################################################################################
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------	Md2IndexList :: operator == 
//
bool Md2VertexArrayIndex::operator == (const Md2VertexArrayIndex& i) const {
	return v==i.v && t==i.t;
}

//-----------------------------------------------------------------------------------------------	Md2IndexList :: Insert
//
unsigned short Md2VertexArrayIndexList::Insert(Md2VertexArrayIndex& index) {
	unsigned short idx=0;
	iterator it = begin();
	for( ; it != end(); ++it, ++idx ) {
		if( *it == index ) 
			return idx;
	}
	push_back(index);
	return idx;
}

//-----------------------------------------------------------------------------------------------	class Md2IndexList
//###############################################################################################
//-----------------------------------------------------------------------------------------------

#if !MD2_ALWAYS_TRIANGLES
//-----------------------------------------------------------------------------------------------	Md2IndexList :: operator == 
//
bool Md2VertexArrayIndex2::operator == (const Md2VertexArrayIndex2& i) const {
	return v==i.v 
		&& (uv[0]>(i.uv[0]-0.0001f)) && (uv[0]<(i.uv[0]+0.0001f))
		&& (uv[1]>(i.uv[1]-0.0001f)) && (uv[1]<(i.uv[1]+0.0001f));
}

//-----------------------------------------------------------------------------------------------	Md2IndexList :: Insert
//
unsigned short Md2VertexArrayIndexList2::Insert(Md2VertexArrayIndex2& index) {
	unsigned short idx=0;
	iterator it = begin();
	for( ; it != end(); ++it, ++idx ) {
		if( *it == index ) 
			return idx;
	}
	push_back(index);
	return idx;
}
#endif



//-----------------------------------------------------------------------------------------------	class Md2Instance
//###############################################################################################
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------	Md2Instance :: Md2Instance
//
Md2Instance::Md2Instance(Md2Model* mod) {
	//NSLog("new instance");
	
	m_pModel = mod;
	m_Visible = true;

	#if !MD2_USE_VBO
		m_VertexData = new Md2VertexNormal[ m_pModel->m_NumElements ];
	#else
		// create the vbo for the instance and assign
	//JAB
		//glGenBuffersARB( 1, &m_VertexBuffer );
	glGenBuffers(1, &m_VertexBuffer);
		std::cout << "buffer> " << m_VertexBuffer << std::endl;
		//glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_VertexBuffer );
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);

		// set VBO size
		//glBufferDataARB( GL_ARRAY_BUFFER_ARB, m_pModel->m_NumElements*sizeof(Md2VertexNormal), 0, GL_STATIC_DRAW_ARB );
	glBufferData(GL_ARRAY_BUFFER, m_pModel->m_NumElements*sizeof(Md2VertexNormal), 0, GL_STATIC_DRAW);
	#endif

	m_Position[0] = m_Position[1] = m_Position[2] = m_RotateY = 0.0f;

	m_CurrentSkin = 0;
	m_CurrentAnim = 0;
	m_PreviousAnim = 0;
	m_AnimTime = 0;
	m_PreviousTime = 0;
	m_TransitionTime = 0.1f;
	m_TimeRemaining = -1.0f;

	// if unsigned chars are used for data storage, we need to LERP the scale and translation
	// keys between the keyframes and apply them with glScale & glTranslate
	#if !MD2_USE_FLOATS

		m_Scale[0] = m_Scale[1] = m_Scale[2] = 1.0f;
		m_Translate[0] = m_Translate[1] = m_Translate[2] = 0.0f;

	#endif
}

//-----------------------------------------------------------------------------------------------	Md2Instance :: ~Md2Instance
//
Md2Instance::~Md2Instance() {
	//NSLog("wtf instance");
	#if MD2_USE_VBO
	//JAB
		//glDeleteBuffersARB(1,&m_VertexBuffer);
	glDeleteBuffers(1, &m_VertexBuffer);
	#else
		delete [] m_VertexData;
	#endif
	m_pModel = 0;	
}


//-----------------------------------------------------------------------------------------------	Md2Instance :: GetCycleName
//
const char* Md2Instance::GetCycleName(unsigned int idx) const {
	if(idx<GetNumCycles())
		return m_pModel->m_AnimCycles[idx].GetName();
	return 0;
}

//-----------------------------------------------------------------------------------------------	Md2Instance :: GetDataSize
//
unsigned int Md2Instance::GetDataSize(Md2MemoryType type) const {
	unsigned int sz=0;
	if(type == MEM_INSTANCE || type == MEM_ALL) {
		#if !MD2_USE_VBO
		sz += sizeof(Md2Instance) +  m_pModel->m_NumElements *  sizeof(Md2VertexNormal);
		#else
		sz += sizeof(Md2Instance);
		#endif
	}
	if(type == MEM_INSTANCE_VBO || type == MEM_ALL) {
		#if MD2_USE_VBO
		sz += m_pModel->m_NumElements *  sizeof(Md2VertexNormal);
		#endif
	}
	return sz;
}

//-----------------------------------------------------------------------------------------------	Md2Instance :: GetNumCycles
//
unsigned int Md2Instance::GetNumCycles() const  {
	return m_pModel->GetNumCycles();
}


//-----------------------------------------------------------------------------------------------	Md2Instance :: GetNumSkins
//
unsigned int Md2Instance::GetNumSkins() const {
	return m_pModel->GetNumSkins();
}

//-----------------------------------------------------------------------------------------------	Md2Instance :: GetSkinName
//
std::string Md2Instance::GetSkinName(unsigned int idx) const {

	if( idx < GetNumSkins() ) {
		return m_pModel->m_SkinNames[idx];
	}
	return 0;
}

//-----------------------------------------------------------------------------------------------	Md2Instance :: Move
//
void Md2Instance::Move(float x,float y,float z,bool Relative) {
	m_Position[1] += y;
	if( Relative ) {
		float angle = m_RotateY*3.14159f/180.0f;
		float sa = sin(angle);
		float ca = cos(angle);
		m_Position[0] += z*sa + x*ca;
		m_Position[2] -= x*sa + z*ca;
	}
	else {
		m_Position[0] += x;
		m_Position[2] += z;
	}
}

//-----------------------------------------------------------------------------------------------	Md2Instance :: Render
//
void Md2Instance::Render() {

	if(!m_Visible)
		return;
	
	//if(g_LastBound != m_CurrentSkin) {
	//	//glBindTexture(GL_TEXTURE_2D,m_CurrentSkin);
	//	g_LastBound = m_CurrentSkin;
	//}

	glPushMatrix();

		glTranslatef(m_Position[0],m_Position[1],m_Position[2]);
		glRotatef(m_RotateY,0,1,0);


		#if !MD2_USE_FLOATS
			glPushMatrix();
				glTranslatef(m_Translate[0],m_Translate[1],m_Translate[2]);
				glScalef(m_Scale[0],m_Scale[1],m_Scale[2]);
		#endif
	



				// vertex data
				glVertexPointer(3,GL_FLOAT,sizeof(Md2VertexNormal), m_VertexData[0].vertex);
				

			#if !MD2_ALWAYS_TRIANGLES
				if(m_pModel->m_StripCounts) {

					/// the original point indices - only needed to calculate the vertex normals
					Md2StripInfo* strip_start = m_pModel->m_StripCounts;
					Md2StripInfo* strip_end = strip_start + m_pModel->m_NumStrips;

					unsigned short* data;

					#if MD2_USE_VBO
						data = 0;
						//glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, m_pModel->m_IndexBuffer );
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_pModel->m_IndexBuffer);
					#else
						data = m_pModel->m_Indices;
					#endif

					for( ; strip_start != strip_end; ++strip_start )
					{
						GLenum Type = GL_TRIANGLE_STRIP;
						if(strip_start->isFan)
							Type = GL_TRIANGLE_FAN;

						glDrawElements(Type,strip_start->num,GL_UNSIGNED_SHORT,data);

						data += strip_start->num;
					}
				}
				else
			#endif
				{
					//LOGV("%d\n", *m_pModel->m_Indices);
						glDrawElements(GL_TRIANGLES,m_pModel->m_NumTris*3,GL_UNSIGNED_SHORT,m_pModel->m_Indices);
				}

		#if !MD2_USE_FLOATS
			glPopMatrix();
		#endif

	glPopMatrix();
}


//-----------------------------------------------------------------------------------------------	Md2Instance :: SetCycle
//
bool Md2Instance::SetCycle(unsigned int cnum){

	// set new cycle if valid
	if(m_pModel->GetNumCycles()>cnum) {
		m_CurrentAnim = cnum;
		m_AnimTime = 0;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------------------	Md2Instance :: SetSkin
//
bool Md2Instance::SetSkin(unsigned int snum) {
	//if(snum<GetNumSkins()) {
		//Md2ReleaseTex(m_CurrentSkin);
		m_CurrentSkin = snum;
		return m_CurrentSkin != 0;
	//}
	//return false;
}


bool Md2Instance::SwitchCycle(unsigned int cnum,float over,bool FromStart, int loops, int afterCycle) {
	if (cnum < GetNumCycles()) {
		if (m_TimeRemaining <= 0.0f) {			
			m_PreviousTime   = m_AnimTime;
			if(FromStart)
				m_AnimTime = 0.0f;
			m_TransitionTime = over;
			m_TimeRemaining = over;
			m_PreviousAnim = m_CurrentAnim;
			m_CurrentAnim = cnum;
			m_LoopAnim = loops;
			m_AfterCycle = afterCycle;
			return true;
		}
	}
	return false;
}


void Md2Instance::Update(float dt){

	Md2AnimCycle* thisAnim = m_pModel->m_AnimCycles + m_CurrentAnim;
	Md2AnimCycle* nextAnim = m_pModel->m_AnimCycles + m_PreviousAnim;

	m_AnimTime += dt;
	float rate = 1.0f/m_pModel->m_FPS;
	while( m_AnimTime > rate * thisAnim->GetNumFrames()) {
		m_AnimTime -= rate * thisAnim->GetNumFrames();
	}
	
	if(m_TimeRemaining>0.0f) 
	{
		m_PreviousTime  += dt;
		m_TimeRemaining -= dt;
		while( m_PreviousTime > rate * nextAnim->GetNumFrames()) {
			m_PreviousTime -= rate * nextAnim->GetNumFrames();
		}
	}

	if (!m_Visible) {
		return;
	}

	if (m_LoopAnim != 0) {
		
		float ddt = m_AnimTime;
		unsigned int firstFrame = 0;
		while( ddt > rate ) {
			ddt -= rate;
			++firstFrame;
		}
		
		ddt /= rate;




#if MD2_USE_FLOATS
			thisAnim->Update(firstFrame,m_pModel->m_NumVerts,ddt);
#else
			thisAnim->Update(firstFrame,m_pModel->m_NumVerts,ddt,m_Scale,m_Translate);
#endif


		if(m_TimeRemaining > 0.0f)  {
			float invInterpT = m_TimeRemaining/m_TransitionTime;
			float interpT = 1.0f - invInterpT;

			// blend vertex of new anim and store in temp
			Md2VertexNormal* pp = g_VertBuffer;
			Md2VertexNormal* pt = g_TempBuffer;
			Md2VertexNormal* pe = pp + m_pModel->m_NumVerts;
			for( ; pp != pe; ++pt, ++pp ) 
			{
				pt->vx = pp->vx * interpT;
				pt->vy = pp->vy * interpT;
				pt->vz = pp->vz * interpT;
			}
				
			ddt = m_PreviousTime;
			firstFrame = 0;
			while( ddt > rate ) {
				ddt -= rate;
				++firstFrame;
			}
			ddt /= rate;

#if MD2_USE_FLOATS
				nextAnim->Update(firstFrame,m_pModel->m_NumVerts,ddt);
#else
				float tempScale[3];
				float tempTranslation[3];
				nextAnim->Update(firstFrame,m_pModel->m_NumVerts,ddt,tempScale,tempTranslation);
			
				m_Scale[0] = m_Scale[0]*interpT + tempScale[0]*invInterpT;
				m_Scale[1] = m_Scale[1]*interpT + tempScale[1]*invInterpT;
				m_Scale[2] = m_Scale[2]*interpT + tempScale[2]*invInterpT;

				m_Translate[0] = m_Translate[0]*interpT + tempTranslation[0]*invInterpT;
				m_Translate[1] = m_Translate[1]*interpT + tempTranslation[1]*invInterpT;
				m_Translate[2] = m_Translate[2]*interpT + tempTranslation[2]*invInterpT;

#endif

			pp = g_VertBuffer;
			pt = g_TempBuffer;
			for( ; pp != pe; ++pt,++pp ) 
			{
				pp->vx *= invInterpT;
				pp->vy *= invInterpT;
				pp->vz *= invInterpT;
				pp->vx += pt->vx;
				pp->vy += pt->vy;
				pp->vz += pt->vz;
			}
		} else {
			if (firstFrame == (thisAnim->GetNumFrames() - 1)) {
				if (m_AfterCycle) {
					this->SwitchCycle(m_AfterCycle, 0.0, false);
				} else {
					m_LoopAnim--;
				}
			}
		}
	
	} else {
		

	}

	
	/*
	#if MD2_CALCULATE_BBOX 
		#if MD2_USE_FLOATS
			m_BBox.maximum[0] = m_BBox.maximum[1] = m_BBox.maximum[2] = -999999.0f;
			m_BBox.minimum[0] = m_BBox.minimum[1] = m_BBox.minimum[2] = 999999.0f;
			
			Md2VertexNormal* pp = g_VertBuffer;
			Md2VertexNormal* pve = pp + m_pModel->m_NumVerts;
			for( ; pp != pve; ++pp ) 
			{
				if(pp->vx<m_BBox.minimum[0]) m_BBox.minimum[0] = pp->vx;
				if(pp->vy<m_BBox.minimum[1]) m_BBox.minimum[1] = pp->vy;
				if(pp->vz<m_BBox.minimum[2]) m_BBox.minimum[2] = pp->vz;
				if(pp->vx>m_BBox.maximum[0]) m_BBox.maximum[0] = pp->vx;
				if(pp->vy>m_BBox.maximum[1]) m_BBox.maximum[1] = pp->vy;
				if(pp->vz>m_BBox.maximum[2]) m_BBox.maximum[2] = pp->vz;
			}

			#if MD2_CALCULATE_BSPHERE
				m_BSphere.center[0] = (m_BBox.maximum[0]-m_BBox.minimum[0])*0.5f + m_BBox.minimum[0];
				m_BSphere.center[1] = (m_BBox.maximum[1]-m_BBox.minimum[1])*0.5f + m_BBox.minimum[1];
				m_BSphere.center[2] = (m_BBox.maximum[2]-m_BBox.minimum[2])*0.5f + m_BBox.minimum[2];
				m_BSphere.radius = 0;
				pp = g_VertBuffer;
				for( ; pp != pve; ++pp ) 
				{
					float midToVert[3];
					midToVert[0] = m_BSphere.center[0] - pp->vx;
					midToVert[1] = m_BSphere.center[1] - pp->vy;
					midToVert[2] = m_BSphere.center[2] - pp->vz;

					float radius = sqrt(midToVert[0] * midToVert[0] +
								  		midToVert[1] * midToVert[1] +
										midToVert[2] * midToVert[2] );
					if( radius > m_BSphere.radius ) {
						m_BSphere.radius = radius;
					}
				}
			#endif	
		#else
			// the bounding box is very easy when the scale and translation is calculated for us ;)
			m_BBox.minimum[0] = m_Translate[0];
			m_BBox.minimum[1] = m_Translate[1];
			m_BBox.minimum[2] = m_Translate[2];
			
			m_BBox.maximum[0] = m_Translate[0] + m_Scale[0]*255.0f;
			m_BBox.maximum[1] = m_Translate[1] + m_Scale[1]*255.0f;
			m_BBox.maximum[2] = m_Translate[2] + m_Scale[2]*255.0f;
			
			#if MD2_CALCULATE_BSPHERE
				m_BSphere.center[0] = m_Scale[0] * 255.0f * 0.5f;
				m_BSphere.center[1] = m_Scale[1] * 255.0f * 0.5f;
				m_BSphere.center[2] = m_Scale[2] * 255.0f * 0.5f;
				m_BSphere.radius = 0;
				Md2VertexNormal* pp = g_VertBuffer;
				Md2VertexNormal* pve = pp + m_pModel->m_NumVerts;
				for( ; pp != pve; ++pp ) 
				{
					float midToVert[3];
					midToVert[0] = m_BSphere.center[0] - m_Scale[0] * pp->vx;
					midToVert[1] = m_BSphere.center[1] - m_Scale[1] * pp->vy;
					midToVert[2] = m_BSphere.center[2] - m_Scale[2] * pp->vz;

					float radius = sqrt(midToVert[0] * midToVert[0] +
										midToVert[1] * midToVert[1] +
										midToVert[2] * midToVert[2] );
					if( radius > m_BSphere.radius ) {
						m_BSphere.radius = radius;
					}
				}
				m_BSphere.center[0] += m_Translate[0] ;
				m_BSphere.center[1] += m_Translate[1] ;
				m_BSphere.center[2] += m_Translate[2] ;
			#endif	
		#endif
	#endif
	*/

#if MD2_USE_NORMALS
		m_pModel->CalcNormals();
#endif
		
	unsigned short* pi = m_pModel->m_VertexMap;
	unsigned short* pe = pi + m_pModel->m_NumElements;
	
#if !MD2_USE_VBO
		Md2VertexNormal* pv = this->m_VertexData;
#else
		Md2VertexNormal* pv = g_TempBuffer;
#endif

	for( ; pi != pe; ++pi, ++pv ) {
		Md2VertexNormal* pInput = g_VertBuffer + *pi;

		pv->vx = pInput->vx;
		pv->vy = pInput->vy;
		pv->vz = pInput->vz;
		
#if MD2_USE_NORMALS
		pv->nx = pInput->nx;
		pv->ny = pInput->ny;
		pv->nz = pInput->nz;
#endif
	}

#if MD2_USE_VBO	//(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid* data);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Md2VertexNormal)* m_pModel->m_NumElements, g_TempBuffer);
#endif

}	
