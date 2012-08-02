/*

File: BrickShader.vertex

Abstract: Brick Shader (from the "orange book")
			 
Copyright (C) 2002-2004  3Dlabs Inc. Ltd.
All rights reserved.

See 3Dlabs-License.txt for license information

*/

attribute vec4 position;
attribute vec4 normal;

uniform vec3 LightPosition;
uniform mat4 modelViewProjMatrix;

const float SpecularContribution = 0.3;
const float DiffuseContribution  = 1.0 - SpecularContribution;

varying float LightIntensity;
varying vec2  MCposition;

void main() 
{
	vec3 ecPosition = (modelViewProjMatrix * position).xyz;
	vec3 tnorm      = normalize((modelViewProjMatrix * normal).xyz);
	vec3 lightVec   = normalize(LightPosition - ecPosition);
	vec3 reflectVec = reflect(-lightVec, tnorm);	
	vec3 viewVec    = normalize(-ecPosition);
	float diffuse   = max(dot(lightVec, tnorm), 0.0);
	float spec      = 0.0;

	if (diffuse > 0.0) {
		spec = dot(reflectVec, viewVec);
		spec = pow(spec, 16.0);
	}

	LightIntensity = DiffuseContribution * diffuse + 
				   SpecularContribution * spec + 0.15;

	MCposition  = position.xy;
	gl_Position = modelViewProjMatrix * position;
}