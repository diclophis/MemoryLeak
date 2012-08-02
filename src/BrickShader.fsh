/*

File: BrickShader.fragment

Abstract: Brick Shader (from the "orange book")
			 
Copyright (C) 2002-2004  3Dlabs Inc. Ltd.
All rights reserved.

See 3Dlabs-License.txt for license information
		  
*/
precision highp float;

uniform vec3 BrickColor, MortarColor;
uniform vec2 BrickSize;
uniform vec2 BrickPct;

varying vec2  MCposition;
varying float LightIntensity;

void main() {
	vec3 color;
	vec2 position, useBrick;

	position = MCposition / BrickSize;
	
	if (fract(position.y * 0.5) > 0.5)
		position.x += 0.5;

	position = fract(position);

	useBrick = step(position, BrickPct);

	color    = mix(MortarColor, BrickColor, useBrick.x * useBrick.y);
	color   *= LightIntensity * 1.5;
	gl_FragColor = vec4(color, 1.0);
}