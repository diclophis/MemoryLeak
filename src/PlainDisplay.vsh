//
//  Shader.vsh
//  CubeExample
//
//  Created by Brad Larson on 4/20/2010.
//

attribute vec4 position;

varying float zDepth;

uniform mat4 modelViewProjMatrix;

void main()
{
//    gl_Position = position;
	vec4 newPosition = modelViewProjMatrix * position;
    gl_Position = newPosition;
	zDepth = (2.0 - (1.0 + newPosition.z))/2.0;
}
