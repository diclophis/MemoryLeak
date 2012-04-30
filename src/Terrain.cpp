// Jon Bardin GPL


#include "MemoryLeak.h"

#define kMaxHillKeyPoints 100
#define kMaxHillVertices 5000
#define kMaxBorderVertices 5000
#define kHillSegmentWidth 15

// in_Position was bound to attribute index 0 and in_Color was bound to attribute index 1
// We output the ex_Color variable to the next shader in the chain
// Since we are using flat lines, our input only had two points: x and y.
// Set the Z coordinate to 0 and W coordinate to 1
// GLSL allows shorthand use of vectors too, the following is also valid:
// gl_Position = vec4(in_Position, 0.0, 1.0);
// We're simply passing the color through unmodified
static const char color_only_vertex_shader[] =
"attribute vec2 in_Position;\n"
"attribute vec4 in_Color;\n"
"varying vec4 ex_Color;\n"
"uniform mat4 ModelViewProjectionMatrix;\n"
"void main(void) {\n"
//"gl_Position = vec4(in_Position.x, in_Position.y, 0.0, 1.0);\n"
"gl_Position = ModelViewProjectionMatrix * vec4(in_Position, 1.0, 1.0);\n"
"ex_Color = in_Color;\n"
"}\n";

// It was expressed that some drivers required this next line to function properly
// Pass through our original color with full opacity.
static const char color_only_fragment_shader[] =
"#ifdef GL_ES\n"
"precision mediump float;\n"
"#endif\n"
"varying vec4 ex_Color;\n"
"void main(void) {\n"
"gl_FragColor = ex_Color;\n"
"}\n";

Terrain::Terrain(b2World *w, GLuint t) {
  m_InterleavedBuffer = 0;
  m_ElementBuffer = 0;

#ifdef HAS_VAO

  m_VertexArrayObject = 0;
  nHillVertices_Last = 0;

#endif

  hillKeyPoints = (MLPoint *) malloc(sizeof(MLPoint) * kMaxHillKeyPoints);
  hillVertices = (MLPoint *) malloc(sizeof(MLPoint) * kMaxHillVertices);
  hillElements = (GLshort *) malloc(sizeof(GLshort) * kMaxHillVertices);
  hillTexCoords = (MLPoint *) malloc(sizeof(MLPoint) * kMaxHillVertices);
  borderVertices = (MLPoint *) malloc(sizeof(MLPoint) * kMaxBorderVertices);
  nHillVertices = 0;
  firstTime = false;
  world = w;
  screenW = 320;
  screenH = 480;
  offsetX = 0.0;
  textureSize = 512;
  GenerateStripesTexture();
  GenerateHillKeyPoints();
  GenerateBorderVertices();
  CreateBox2DBody();

  glGenBuffers(1, &m_InterleavedBuffer);
  glGenBuffers(1, &m_ElementBuffer);

  //SetOffsetX(0.0);

}


Terrain::~Terrain() {
  LOGV("delloc terrain\n");
  free(hillKeyPoints);
  free(hillVertices);
  free(hillTexCoords);
  free(borderVertices);
  delete rt;
}


void Terrain::Reset() {
  fromKeyPointI = 0;
  toKeyPointI = 0;
}


void Terrain::GenerateHillKeyPoints() {

  nHillKeyPoints = 0;

  float x, y, dx, dy, ny;

  x = -screenW/4;
  y = 0.0;
  //y = screenH*3/4;
  hillKeyPoints[nHillKeyPoints++] = MLPointMake(x, y);

  // starting point
  x = 0;
  //y = screenH/2;
  hillKeyPoints[nHillKeyPoints++] = MLPointMake(x, y);

  int minDX = 160, rangeDX = 80;
  int minDY = 60,  rangeDY = 60;
  float sign = -1; // +1 - going up, -1 - going  down
  float maxHeight = screenH;
  float minHeight = 0;
  while (nHillKeyPoints < kMaxHillKeyPoints-1) {
    dx = random() % rangeDX + minDX;
    x += dx;
    dy = minDY; //random() % rangeDY + minDY;
    ny = y + dy * sign;
    if(ny > maxHeight) ny = maxHeight;
    if(ny < minHeight) ny = minHeight;
    y = ny;
    sign *= -1;
    hillKeyPoints[nHillKeyPoints++] = MLPointMake(x, y);
  }

  // cliff
  x += minDX+rangeDX;
  y = 0;
  hillKeyPoints[nHillKeyPoints++] = MLPointMake(x, y);

  fromKeyPointI = 0;
  toKeyPointI = 0;
}


void Terrain::GenerateBorderVertices() {
  nBorderVertices = 0;
  MLPoint p0, p1, pt0, pt1;
  p0 = hillKeyPoints[0];
  for (int i=1; i<nHillKeyPoints; i++) {
    p1 = hillKeyPoints[i];
    int hSegments = floorf((p1.x-p0.x)/kHillSegmentWidth);
    float dx = (p1.x - p0.x) / hSegments;
    float da = M_PI / hSegments;
    float ymid = (p0.y + p1.y) / 2;
    float ampl = (p0.y - p1.y) / 2;
    pt0 = p0;
    borderVertices[nBorderVertices++] = pt0;
    for (int j=1; j<hSegments+1; j++) {
      pt1.x = p0.x + j*dx;
      pt1.y = ymid + ampl * cosf(da*j);
      borderVertices[nBorderVertices++] = pt1;
      pt0 = pt1;
    }
    p0 = p1;
  }
}


void Terrain::CreateBox2DBody() {
  b2BodyDef bd;
  bd.position.Set(0, 0);

  body = world->CreateBody(&bd);

  b2Vec2 b2vertices[kMaxBorderVertices];
  int nVertices = 0;
  for (int i=0; i<nBorderVertices; i++) {
    b2vertices[nVertices++].Set(borderVertices[i].x / PTM_RATIO, borderVertices[i].y / PTM_RATIO);
  }
  b2vertices[nVertices++].Set(borderVertices[nBorderVertices-1].x/PTM_RATIO,0);
  b2vertices[nVertices++].Set(-screenW/4,0);

  b2LoopShape shape;
  shape.Create(b2vertices, nVertices);
  body->CreateFixture(&shape, 0);
}

void Terrain::SetOffsetX(float x, StateFoo *sf) {
  firstTime = true;
  if (offsetX != x || firstTime) {
    offsetX = x;
    firstTime = false;
    position = MLPointMake(320 / 8 - offsetX * 1, 0);
    ResetHillVertices(sf);
  }
}

void Terrain::ResetHillVertices(StateFoo *sf) {
  prevFromKeyPointI = -1;
  prevToKeyPointI = -1;

  // key points interval for drawing
  float leftSideX = offsetX - (screenW * 2); // / 8 / 1; //scale;
  float rightSideX = offsetX + (screenW * 2); // * 7 / 8 / 1; //scale;

  //int element = 0;

  while (hillKeyPoints[fromKeyPointI+1].x < leftSideX) {
    fromKeyPointI++;
    if (fromKeyPointI > nHillKeyPoints-1) {
      fromKeyPointI = nHillKeyPoints-1;
      break;
    }
  }

  while (hillKeyPoints[toKeyPointI].x < rightSideX) {
    toKeyPointI++;
    if (toKeyPointI > nHillKeyPoints-1) {
      toKeyPointI = nHillKeyPoints-1;
      break;
    }
  }
    
  if (prevFromKeyPointI != fromKeyPointI || prevToKeyPointI != toKeyPointI) {
    // vertices for visible area
    nHillVertices = 0;
    MLPoint p0, p1, pt0, pt1;
    p0 = hillKeyPoints[fromKeyPointI];

    for (int i=fromKeyPointI+1; i<toKeyPointI+1; i++) {
      p1 = hillKeyPoints[i];

      // triangle strip between p0 and p1
      int hSegments = floorf((p1.x-p0.x)/kHillSegmentWidth);
      int vSegments = 1;
      float dx = (p1.x - p0.x) / hSegments;
      float da = M_PI / hSegments;
      float ymid = (p0.y + p1.y) / 2;
      float ampl = (p0.y - p1.y) / 2;
      pt0 = p0;
      for (int j=1; j<hSegments+1; j++) {
        pt1.x = p0.x + j*dx;
        pt1.y = ymid + ampl * cosf(da*j);
        for (int k=0; k<vSegments+1; k++) {
          //hillElements[element++] = nHillVertices;
          hillVertices[nHillVertices] = MLPointMake(pt0.x, pt0.y-(float)textureSize/vSegments*k);
          hillTexCoords[nHillVertices++] = MLPointMake(pt0.x/(float)textureSize, (float)(k)/vSegments);
          //hillElements[element++] = nHillVertices;
          hillVertices[nHillVertices] = MLPointMake(pt1.x, pt1.y-(float)textureSize/vSegments*k);
          hillTexCoords[nHillVertices++] = MLPointMake(pt1.x/(float)textureSize, (float)(k)/vSegments);
          //if ((nHillVertices % 3) == 0) {
          //  hillElements[element] = element;
          //  element++;
          //}
        }
        pt0 = pt1;
      }

      p0 = p1;
    }
        
    prevFromKeyPointI = fromKeyPointI;
    prevToKeyPointI = toKeyPointI;
  } else {
    LOGV("BADD!@#$!@#!@# %d %d %d %d\n", prevFromKeyPointI, fromKeyPointI, prevToKeyPointI, toKeyPointI);
  }

  //hillElements[0] = 0;
  //hillElements[1] = 1;
  //hillElements[2] = 2;
  for (int i=0; i<(nHillVertices); i++) {
    hillElements[i] = i;
  }

  //if (m_InterleavedBuffer != 0) {
  //  glDeleteBuffers(1, &m_InterleavedBuffer);
  //}

  //if (m_ElementBuffer != 0) {
  //  glDeleteBuffers(1, &m_ElementBuffer);
  //}


  //glBindBuffer(GL_ARRAY_BUFFER, m_InterleavedBuffer);
  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ElementBuffer);

#ifdef HAS_VAO

  if (m_VertexArrayObject == 0) {
    glGenVertexArraysOES(1, &m_VertexArrayObject);
  }

  //if (m_VertexArrayObject != sf->g_lastVertexArrayObject) {

  if (m_VertexArrayObject != sf->g_lastVertexArrayObject) {
    sf->g_lastVertexArrayObject = m_VertexArrayObject;
    glBindVertexArrayOES(sf->g_lastVertexArrayObject);
  }

  if (m_ElementBuffer != sf->g_lastElementBuffer) {
    sf->g_lastElementBuffer = m_ElementBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sf->g_lastElementBuffer);
  }

  if (m_InterleavedBuffer != sf->g_lastInterleavedBuffer) {
    sf->g_lastInterleavedBuffer = m_InterleavedBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, sf->g_lastInterleavedBuffer);
  }

  if (nHillVertices_Last == 0) {
    glVertexPointer(2, GL_FLOAT, 0, (char *)NULL + (0));
  }

  if (nHillVertices != nHillVertices_Last) { 
    glTexCoordPointer(2, GL_FLOAT, 0, (char *)NULL + (nHillVertices * sizeof(MLPoint)));
    nHillVertices_Last = nHillVertices;
  }

#else

  if (m_ElementBuffer != sf->g_lastElementBuffer) {
    sf->g_lastElementBuffer = m_ElementBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sf->g_lastElementBuffer);
  }

  if (m_InterleavedBuffer != sf->g_lastInterleavedBuffer) {
    sf->g_lastInterleavedBuffer = m_InterleavedBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, sf->g_lastInterleavedBuffer);
  }

#endif

  glBufferData(GL_ARRAY_BUFFER, nHillVertices * sizeof(MLPoint) * 2, NULL, GL_DYNAMIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, nHillVertices * sizeof(MLPoint), hillVertices);
  glBufferSubData(GL_ARRAY_BUFFER, nHillVertices * sizeof(MLPoint), nHillVertices * sizeof(MLPoint), hillTexCoords);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER, nHillVertices * sizeof(GLshort), NULL, GL_DYNAMIC_DRAW);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, nHillVertices * sizeof(GLshort), hillElements);

  //glBindBuffer(GL_ARRAY_BUFFER, 0);
  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  //sf->g_lastInterleavedBuffer = 0;
  //sf->g_lastElementBuffer = 0;
  //glBindBuffer(GL_ARRAY_BUFFER, sf->g_lastInterleavedBuffer);
  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sf->g_lastElementBuffer);

}


void Terrain::Render(StateFoo *sf) {

	if (rt->name != sf->g_lastTexture) {
    sf->g_lastTexture = rt->name;
		glBindTexture(GL_TEXTURE_2D, sf->g_lastTexture);
	}
  
#ifdef HAS_VAO  

  if (m_VertexArrayObject != sf->g_lastVertexArrayObject) {
    sf->g_lastVertexArrayObject = m_VertexArrayObject;
    glBindVertexArrayOES(sf->g_lastVertexArrayObject);
  }

  /*
  if (m_ElementBuffer != sf->g_lastElementBuffer) {
    sf->g_lastElementBuffer = m_ElementBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sf->g_lastElementBuffer);
  }

  */

  if (m_InterleavedBuffer != sf->g_lastInterleavedBuffer) {
    sf->g_lastInterleavedBuffer = m_InterleavedBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, sf->g_lastInterleavedBuffer);
  }

#else

  if (m_ElementBuffer != sf->g_lastElementBuffer) {
    sf->g_lastElementBuffer = m_ElementBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sf->g_lastElementBuffer);
  }

  if (m_InterleavedBuffer != sf->g_lastInterleavedBuffer) {
    sf->g_lastInterleavedBuffer = m_InterleavedBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, sf->g_lastInterleavedBuffer);
  }

#ifdef USE_GLES2

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (char *)NULL + (0));
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (char *)NULL + (nHillVertices * sizeof(MLPoint)));

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

#else

  glVertexPointer(2, GL_FLOAT, 0, (char *)NULL + (0));
  glTexCoordPointer(2, GL_FLOAT, 0, (char *)NULL + (nHillVertices * sizeof(MLPoint)));

#endif

#endif
 
  if (!sf->m_EnabledStates) {
    glEnable(GL_BLEND);

#ifdef USE_GLES2

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);

#else

    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

#endif

    sf->m_EnabledStates = true;
  }
  
  if (true) {
    glDrawElements(GL_TRIANGLE_STRIP, (nHillVertices), GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));
  } else {

    glColor4f(1.0, 1.0, 1.0, 1.0);
    glLineWidth(1.0);

#ifdef USE_GLES2

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (char *)NULL + (0));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (char *)NULL + (nHillVertices * sizeof(MLPoint)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    //glDisableVertexAttribArray(1);

#else

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, (char *)NULL + (0));

#endif

    glDrawElements(GL_LINE_STRIP, (nHillVertices / 2), GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));

#ifdef USE_GLES2

    glEnableVertexAttribArray(1);

#else

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

#endif

  }

}


GLuint Terrain::GenerateStripesTexture() {  
  MLPoint texSize = MLPointMake(textureSize, textureSize);
	// Calculate the adjustment ratios based on the old and new projections
	MLPoint size = MLPointMake(320.0, 480.0);
  // random number of stripes (even)
  const int minStripes = 20;
  const int maxStripes = 30;
  int nStripes = random() % (maxStripes - minStripes) + minStripes;
  if (nStripes % 2) {
    nStripes++;
  }
  
  MLPoint vertices[maxStripes*6];
  ccColor4F colors[maxStripes*6];
  int nVertices = 0;
  
  float x1, x2, y1, y2, dx, dy;
  ccColor4F c;
  
  glViewport(0, 0, texSize.x, texSize.y);

  rt = new RenderTexture(textureSize, textureSize);
  rt->Begin();


#ifdef USE_GLES2


    //ortho(Engine::GetProjectionMatrix(), aa, bb, cc, dd, ee, ff);
    //glUniformMatrix4fv(Engine::GetProjectionMatrixLocation(), 1, GL_FALSE, Engine::GetProjectionMatrix());

    const GLchar *source = color_only_vertex_shader;
    vertexshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexshader, 1, &source, NULL);
    glCompileShader(vertexshader);
    glGetShaderInfoLog(vertexshader, sizeof msg, NULL, msg);
    LOGV("vertex shader info: %s\n", msg);

    source = color_only_fragment_shader;
    fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentshader, 1, &source, NULL);
    glCompileShader(fragmentshader);
    glGetShaderInfoLog(fragmentshader, sizeof msg, NULL, msg);
    LOGV("fragment shader info: %s\n", msg);

    shaderprogram = glCreateProgram();
    glAttachShader(shaderprogram, vertexshader);
    glAttachShader(shaderprogram, fragmentshader);
    glLinkProgram(shaderprogram);
    glGetProgramInfoLog(shaderprogram, sizeof msg, NULL, msg);
    LOGV("info: %s\n", msg);

    glUseProgram(shaderprogram);

    float m_ScreenHalfHeight = 512.0 / 2.0;
    float m_ScreenAspect = 1.0;
    float m_Zoom = 1.0;
    float aa = (-m_ScreenHalfHeight * m_ScreenAspect) * m_Zoom;
    float bb = (m_ScreenHalfHeight * m_ScreenAspect) * m_Zoom;
    float cc = (-m_ScreenHalfHeight) * m_Zoom;
    float dd = m_ScreenHalfHeight * m_Zoom;
    float ee = 1.0;
    float ff = -1.0;

    ModelViewProjectionMatrix_location = glGetUniformLocation(shaderprogram, "ModelViewProjectionMatrix");
    //ortho(ProjectionMatrix, aa, bb, cc, dd, ee, ff);
    ortho(ProjectionMatrix, 512.0, 0, 512.0, 0, -1.0, 1.0);
    glUniformMatrix4fv(ModelViewProjectionMatrix_location, 1, GL_FALSE, ProjectionMatrix);

#else

  //glMatrixMode(GL_PROJECTION);
  //glLoadIdentity();
  glOrthof(512.0, 0.0, 512.0, 0.0, -1.0, 1.0);

#endif

  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  if (true) {      
    // layer 1: stripes
    if (random() % 2) {
      dx = (float)textureSize*2 / (float)nStripes;
      dy = 0;
      x1 = -textureSize;
      y1 = 0;
      x2 = 0;
      y2 = textureSize;
      for (int i=0; i<nStripes/2; i++) {
        c = GenerateColor();
        for (int j=0; j<2; j++) {
          vertices[nVertices] = MLPointMake(x1+j*textureSize, y1);
          colors[nVertices++] = c;
          vertices[nVertices] = MLPointMake(x1+j*textureSize+dx, y1);
          colors[nVertices++] = c;
          vertices[nVertices] = MLPointMake(x2+j*textureSize, y2);
          colors[nVertices++] = c;
          vertices[nVertices] = vertices[nVertices-2];
          colors[nVertices++] = c;
          vertices[nVertices] = vertices[nVertices-2];
          colors[nVertices++] = c;
          vertices[nVertices] = MLPointMake(x2+j*textureSize+dx, y2);
          colors[nVertices++] = c;
        }
        x1 += dx;
        x2 += dx;
      }
    } else {
      // horizontal stripes
      dx = 0;
      dy = (float)textureSize / (float)nStripes;
      x1 = 0;
      y1 = 0;
      x2 = textureSize;
      y2 = 0;
      for (int i=0; i<nStripes; i++) {
        c = GenerateColor();
        vertices[nVertices] = MLPointMake(x1, y1);
        colors[nVertices++] = c;
        vertices[nVertices] = MLPointMake(x2, y2);
        colors[nVertices++] = c;
        vertices[nVertices] = MLPointMake(x1, y1+dy);
        colors[nVertices++] = c;
        vertices[nVertices] = vertices[nVertices-2];
        colors[nVertices++] = c;
        vertices[nVertices] = vertices[nVertices-2];
        colors[nVertices++] = c;
        vertices[nVertices] = MLPointMake(x2, y2+dy);
        colors[nVertices++] = c;
        y1 += dy;
        y2 += dy;
      }
    }

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(1.0, 1.0, 1.0, 1.0);

    glGenBuffers(1, &m_TextureInterlacedBuffer);
    glGenBuffers(1, &m_TextureElementBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, m_TextureInterlacedBuffer);
    glBufferData(GL_ARRAY_BUFFER, (nVertices * sizeof(MLPoint)) + (nVertices * sizeof(ccColor4F)), NULL, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, (nVertices * sizeof(MLPoint)), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, (nVertices * sizeof(MLPoint)), nVertices * sizeof(ccColor4F), colors);

    GLuint *textureElements = (GLuint *)malloc(nVertices * 2 * sizeof(GLuint));
    for (int i=0; i<(nVertices * 2); i++) {
      textureElements[i] = i;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_TextureElementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (nVertices * sizeof(GLuint)), textureElements, GL_STATIC_DRAW);

    free(textureElements);

#ifdef USE_GLES2

    // Specify that our coordinate data is going into attribute index 0, and contains two floats per vertex 
    // Specify that our color data is going into attribute index 1, and contains three floats per vertex

    GLuint pL = glGetAttribLocation(shaderprogram, "in_Position");
    GLuint cL = glGetAttribLocation(shaderprogram, "in_Color");

    glEnableVertexAttribArray(pL);
    glVertexAttribPointer(pL, 2, GL_FLOAT, GL_FALSE, 0, (char *)NULL + (0));

    glEnableVertexAttribArray(cL);
    glVertexAttribPointer(cL, 4, GL_FLOAT, GL_FALSE, 0, (char *)NULL + (nVertices * sizeof(MLPoint)));

#else

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, (char *)NULL + (0));
    glColorPointer(4, GL_FLOAT, 0, (char *)NULL + (nVertices * sizeof(MLPoint)));

#endif

    //glPointSize(10.0);
    glDrawElements(GL_TRIANGLES, nVertices * 2, GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));

    //glDrawArrays(GL_TRIANGLES, 0, (GLsizei)nVertices);
    //glDrawArrays(GL_TRIANGLES, 0, 3);

#ifdef USE_GLES2
    

#else

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

#endif

    glDisable(GL_BLEND);

  }

  

  if (false) {
    // layer: gradient
    float gradientAlpha = 0.5f;
    float gradientWidth = textureSize;
    nVertices = 0;
    vertices[nVertices] = MLPointMake(0, 0);
    colors[nVertices++] = (ccColor4F){0, 0, 0, 0};
    vertices[nVertices] = MLPointMake(textureSize, 0);
    colors[nVertices++] = (ccColor4F){0, 0, 0, 0};
    vertices[nVertices] = MLPointMake(0, gradientWidth);
    colors[nVertices++] = (ccColor4F){0, 0, 0, gradientAlpha};
    vertices[nVertices] = MLPointMake(textureSize, gradientWidth);
    colors[nVertices++] = (ccColor4F){0, 0, 0, gradientAlpha};
    if (gradientWidth < textureSize) {
      vertices[nVertices] = MLPointMake(0, textureSize);
      colors[nVertices++] = (ccColor4F){0, 0, 0, gradientAlpha};
      vertices[nVertices] = MLPointMake(textureSize, textureSize);
      colors[nVertices++] = (ccColor4F){0, 0, 0, gradientAlpha};
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glColorPointer(4, GL_FLOAT, 0, colors);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)nVertices);
    glDisableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_BLEND);
  }
  
  
  if (false) {
    // layer: highlight
    float highlightAlpha = 0.5f;
    nVertices = 0;
    vertices[nVertices] = MLPointMake(0.0, 512.0);
    colors[nVertices++] = (ccColor4F){1, 1, 0.5f, highlightAlpha}; // yellow
    vertices[nVertices] = MLPointMake(textureSize, 512.0);
    colors[nVertices++] = (ccColor4F){1, 1, 0.5f, highlightAlpha};
    vertices[nVertices] = MLPointMake(0, 128.0);
    colors[nVertices++] = (ccColor4F){0, 0, 0, 0};
    vertices[nVertices] = MLPointMake(textureSize, 128.0);
    colors[nVertices++] = (ccColor4F){0, 0, 0, 0};
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glColorPointer(4, GL_FLOAT, 0, colors);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)nVertices);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_BLEND);
  }


  if (false) {
    // layer: top border
    float borderAlpha = 0.5f;
    float borderWidth = 10.0f;
    nVertices = 0;
    vertices[nVertices++] = MLPointMake(0, (borderWidth / 2.0) + 512.0 - borderWidth);
    vertices[nVertices++] = MLPointMake(textureSize, (borderWidth / 2.0) + 512.0 - borderWidth);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnableClientState(GL_VERTEX_ARRAY);
    glLineWidth(borderWidth);
    glColor4f(1.0, 0.0, 0.0, borderAlpha);
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)nVertices);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_BLEND);
  }

  
  if (false) {
    // layer: noise
    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR, GL_ZERO);
    //stripes->SetPosition(textureSize / 2, textureSize / 2);
    glColor4f(1, 1, 1, 1);
    //render an AtlasSprite full "screen" here!
    glDisable(GL_BLEND);
  }
  
  rt->End();

  //glViewport(0, 0, size.x, size.y);
  //glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  return rt->name;
}


static inline ccColor3B ccc3(const GLubyte r, const GLubyte g, const GLubyte b) {
  ccColor3B c = {r, g, b};
  return c;
}


static inline ccColor4F ccc4FFromccc3B(ccColor3B c) {
  return (ccColor4F){c.r/255.f, c.g/255.f, c.b/255.f, 1.f};
}


ccColor4F Terrain::GenerateColor() {
  const int minSum = 450;
  const int minDelta = 150;
  int r, g, b, min, max;
  /*
  while (true) {
    r = random()%256;
    g = random()%256;
    b = random()%256;
    min = MIN(MIN(r, g), b);
    max = MAX(MAX(r, g), b);
    if (max-min < minDelta) continue;
    if (r+g+b < minSum) continue;
    break;
  }
  return ccc4FFromccc3B(ccc3(r, g, b));
  */
  return ccc4FFromccc3B(ccc3(256, 125, 125));

}
