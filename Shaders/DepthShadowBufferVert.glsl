#version 400 core

uniform mat4 modelMatrix;
uniform mat4 lightSpaceMatrix;

uniform bool isAnimated = false;
uniform mat4 joints[128];

in vec3 position;
in vec2 texCoord;
in vec4 jointWeights;
in ivec4 jointIndices;

out vec2 TexCoordsOUT;

void main(void) 
{
	TexCoordsOUT = texCoord;

	vec4 pos = vec4(position, 1.0);
	vec4 final = vec4(1.0);
	if(isAnimated)
	{
		vec4 skelPos = vec4(0, 0, 0, 0);

		for(int i = 0; i < 4; ++i)
		{
			int jointIndex = jointIndices[i];
			float jointWeight = jointWeights[i];

			skelPos += joints[jointIndex] * pos * jointWeight;
		}

		final = lightSpaceMatrix * modelMatrix * vec4(skelPos.xyz, 1.0);
	}
	else
	{
		final = lightSpaceMatrix * modelMatrix * pos;
	}
	
	gl_Position = final;
}