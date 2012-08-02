attribute vec4 position;
attribute vec3 normal;

uniform mat4 modelViewProjMatrix;
uniform vec3 lightDirection;
uniform mat3 Model;
uniform vec3 EyePosition;

varying vec3 ReflectDir;
varying float lightIntensity;

void main(void)
{
	vec4 newNormal =  modelViewProjMatrix * vec4(normal,1.0);
	vec4 newPosition = modelViewProjMatrix * position;
    gl_Position = newPosition;
	
//	lightIntensity = max(0.0, dot(newNormal.xyz, lightDirection));
    mediump vec3 eyeDir = normalize(newPosition.xyz - EyePosition);

	lightIntensity = 0.0000000002 * pow(max(0.0, dot(reflect(lightDirection, newNormal.xyz), EyePosition)), 4.0 );
    
    // Compute eye direction in object space:
//    mediump vec3 eyeDir = normalize(position.xyz - (modelViewProjMatrix * EyePosition).xyz);
//    mediump vec3 eyeDir = normalize(position.xyz - EyePosition);
//    mediump vec3 eyeDir = (position * modelViewProjMatrix).xyz;
//    mediump vec3 eyeDir = position.xyz;

    // Reflect eye direction over normal and transform to world space:
//    ReflectDir = (vec4(reflect(eyeDir, normal), 1.0) * modelViewProjMatrix).xyz;

    /*
     vec3 normal = vec3(impostorSpaceCoordinate, normalizedDepth);
     float ambientLightingIntensityFactor = clamp(dot(lightPosition, normal), 0.0, 1.0);
     
     float lightingIntensity = 0.2 + 1.7 * ambientLightingIntensityFactor * ambientOcclusionIntensity;
     //            float lightingIntensity = 0.2 + 1.7 * precalculatedDepthAndLighting.g * ambientOcclusionIntensity;
     vec3 finalSphereColor = sphereColor * lightingIntensity;
     
     // Specular lighting
     float specularLightingIntensityFactor = pow(ambientLightingIntensityFactor, 60.0) * 0.6;
     //            finalSphereColor = finalSphereColor + (precalculatedDepthAndLighting.b * ambientOcclusionIntensity);
     finalSphereColor = finalSphereColor + (specularLightingIntensityFactor * ambientOcclusionIntensity);
*/
    
    
	ReflectDir = reflect(eyeDir, newNormal.xyz);
}
