//
// Vertex shader for cartoon-style shading
//
// Author: Philip Rideout
//
// Copyright (c) 2005-2006 3Dlabs Inc. Ltd.
//
// See 3Dlabs-License.txt for license information
//

attribute vec4 position;
attribute vec4 normal;

uniform mat4 modelViewProjMatrix;

varying vec3 currentNormal;

void main(void)
{
	currentNormal = normalize((modelViewProjMatrix * normal).xyz);
	gl_Position = modelViewProjMatrix * position;
}
