//
//  Shader.fsh
//  CubeExample
//
//  Created by Brad Larson on 4/20/2010.
//
varying highp float zDepth;

void main()
{
	gl_FragColor = vec4(zDepth, zDepth, 0.0, 1.0);
}
