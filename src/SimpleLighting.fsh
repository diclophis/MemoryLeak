//
//  Shader.fsh
//  CubeExample
//
//  Created by Brad Larson on 4/20/2010.
//
varying highp float lightIntensity;

void main()
{
	lowp vec4 yellow = vec4(1.0, 1.0, 0.0, 1.0);
	gl_FragColor = vec4((yellow * lightIntensity * 0.2).rgb, 1.0);
}
