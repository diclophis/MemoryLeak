// Jon Bardin GPL


#include "MemoryLeak.h"

#define kMaxHillKeyPoints 100
#define kMaxHillVertices 5000
#define kMaxBorderVertices 5000
#define kHillSegmentWidth 15

Terrain::Terrain(b2World *w, GLuint t) {
  hillKeyPoints = (MLPoint *) malloc(sizeof(MLPoint) * kMaxHillKeyPoints);
  hillVertices = (MLPoint *) malloc(sizeof(MLPoint) * kMaxHillVertices);
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
  SetOffsetX(0.0);
}


Terrain::~Terrain() {
  //delete stripes;
  LOGV("del;ete terrain\n");
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

void Terrain::SetOffsetX(float x) {
  firstTime = true;
  if (offsetX != x || firstTime) {
    offsetX = x;
    firstTime = false;
    position = MLPointMake(320 / 8 - offsetX * 1, 0);
    ResetHillVertices();
  }
}

void Terrain::ResetHillVertices() {
  prevFromKeyPointI = -1;
  prevToKeyPointI = -1;

  // key points interval for drawing
  float leftSideX = offsetX - (screenW * 2); // / 8 / 1; //scale;
  float rightSideX = offsetX + (screenW * 2); // * 7 / 8 / 1; //scale;

  //LOGV("left right %f %f\n", leftSideX, rightSideX);

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
  //LOGV("good!\n");
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
          hillVertices[nHillVertices] = MLPointMake(pt0.x, pt0.y-(float)textureSize/vSegments*k);
          hillTexCoords[nHillVertices++] = MLPointMake(pt0.x/(float)textureSize, (float)(k)/vSegments);
          hillVertices[nHillVertices] = MLPointMake(pt1.x, pt1.y-(float)textureSize/vSegments*k);
          hillTexCoords[nHillVertices++] = MLPointMake(pt1.x/(float)textureSize, (float)(k)/vSegments);
          //LOGV("WHA %d\n", nHillVertices);
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

  //LOGV("nHillVertices should be set here %d\n", nHillVertices);
}


void Terrain::Render() {
  glEnable(GL_TEXTURE_2D);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  
  //glPushMatrix();
  //{
  //glEnableClientState(GL_VERTEX_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
Engine::CheckGL("Render in T2");
  if (true) {
    //glEnable(GL_TEXTURE_2D);
    //glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    //LOGV("bind: %d\n", rt->name);
    //glBindTexture(GL_TEXTURE_2D, rt->name - 2);
    glBindTexture(GL_TEXTURE_2D, rt->name);
    glVertexPointer(2, GL_FLOAT, 0, hillVertices);
    glTexCoordPointer(2, GL_FLOAT, 0, hillTexCoords);
Engine::CheckGL("Render in T1");
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)nHillVertices);
    //glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    //glBindTexture(GL_TEXTURE_2D, 0);
    //glDisable(GL_TEXTURE_2D);
  } else {
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glColor4f(1.0, 1.0, 1.0, 0.5);
    glLineWidth(2.0);
    glVertexPointer(2, GL_FLOAT, 0, hillVertices);
    glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)nHillVertices);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  }
  //glDisableClientState(GL_VERTEX_ARRAY);
  //Engine::CheckGL("Render in T");
  //}
  //glPopMatrix();
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisable(GL_TEXTURE_2D);
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
  
  rt = new RenderTexture(textureSize, textureSize);

  //glPushMatrix();
  //Engine::CheckGL("glPushMatrix in T");
  //{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthof(512.0, 0.0, 512.0, 0.0, -1.0, 1.0);

    //glMatrixMode(GL_MODELVIEW);
    
    glViewport(0, 0, texSize.x, texSize.y);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glFinish();

    rt->Begin();
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

      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_COLOR_ARRAY);
      glEnable(GL_BLEND);
      glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
      glColor4f(1, 1, 1, 1);
      glVertexPointer(2, GL_FLOAT, 0, vertices);
      glColorPointer(4, GL_FLOAT, 0, colors);
      glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);

      glDrawArrays(GL_TRIANGLES, 0, (GLsizei)nVertices);
      
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);

      glDisable(GL_BLEND);
      glDisableClientState(GL_COLOR_ARRAY);
      glDisableClientState(GL_VERTEX_ARRAY);
    }
    

    if (true) {
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
    
    
    if (true) {
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


    if (true) {
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

    
    if (true) {
      // layer: noise
      glEnable(GL_BLEND);
      glBlendFunc(GL_DST_COLOR, GL_ZERO);
      //stripes->SetPosition(textureSize / 2, textureSize / 2);
      glColor4f(1, 1, 1, 1);
      //stripes->Render();
      //stripes->Render();
      //stripes->Render();
      glDisable(GL_BLEND);
    }
    
    rt->End();
    
  //}
  //glPopMatrix();

  glViewport(0, 0, size.x, size.y);
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  glFinish();

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
}
