varying highp vec3 ReflectDir;
varying highp float lightIntensity;

uniform samplerCube cubeMap;

const highp vec3 materialColor = vec3(0.7, 0.7, 0.7);

void main(void)
{
	highp float reflectivity = 0.8;
    
    highp vec4 reflectionContribution = vec4(materialColor * textureCube(cubeMap, ReflectDir).rgb + vec3(0.1), 1.0);
    
    gl_FragColor = (reflectionContribution * reflectivity) + (lightIntensity * (1.0 - reflectivity));
}