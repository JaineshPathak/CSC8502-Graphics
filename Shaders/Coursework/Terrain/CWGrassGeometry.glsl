#version 430

layout (points) in;
layout (triangle_strip, max_vertices = 12) out;

layout(std140, binding = 0) uniform Matrices
{
	mat4 projMatrix;
	mat4 viewMatrix;
};

struct EnvironmentData
{
	vec4 fogData;
	vec4 fogColor;
};

layout(std140, binding = 3) uniform u_EnvironmentData
{
	EnvironmentData envData;
};

const float PI = 3.141592653589793;

float grass_size;
const float c_min_size = 1.5f;

//This should be same as from Vertex Shader
in Vertex
{
    vec4 colour;
	vec2 texCoord;
	vec3 normal;
    vec3 worldPos;
	mat4 instanceModelMat;
} IN[];

//This should same for Fragment Shader
out Vertex
{
    vec4 colour;
    vec2 texCoord;
    vec3 normal;
    vec3 worldPos;
    float visibility;
} OUT;

float random(vec2 st);
float randomRange(float min, float max, vec2 seed);
mat4 rotationX(in float angle);
mat4 rotationY(in float angle);
mat4 rotationZ(in float angle);

void CreateQuad(in vec4 basePosition, in mat4 crossModel)
{
    mat4 modelRandY = rotationY(random(basePosition.zx) * PI);
    
    vec4 vertexPosition[4];
    vertexPosition[0] = vec4(-0.2, 0.0, 0.0, 0.0); 	// down left
    vertexPosition[1] = vec4( 0.2, 0.0, 0.0, 0.0);  // down right
    vertexPosition[2] = vec4(-0.2, 0.2, 0.0, 0.0);	// up left
    vertexPosition[3] = vec4( 0.2, 0.2, 0.0, 0.0);  // up right

    vec2 textCoords[4];
    textCoords[0] = vec2(0.0, 0.0); // down left
    textCoords[1] = vec2(1.0, 0.0); // down right
    textCoords[2] = vec2(0.0, 1.0); // up left
    textCoords[3] = vec2(1.0, 1.0); // up right

//    vec3 normal = normalize(cross(vertexPosition[1].xyz - vertexPosition[0].xyz, vertexPosition[2].xyz - vertexPosition[0].xyz));
//    mat3 normalMat = transpose(inverse(mat3(IN[0].instanceMat)));
//    OUT.normal = normalize(normalMat * normalize(normal));

    for(int i = 0; i < 4; i++) 
    {

        //TODO: FIX THIS
        //float grassAmount = texture(diffuseSplatmapTex, textCoords[i] / 16.0).r;
        /*if(grassAmount < 0.2f)
            grass_size = 1.5f;
        else
            grass_size = 0.0f;*/

        //vertexPosition[i] *= grass_size;	         
        gl_Position = projMatrix * viewMatrix * IN[0].instanceModelMat * (gl_in[0].gl_Position + crossModel * vertexPosition[i] * grass_size);
        OUT.texCoord = textCoords[i];

        //grass_size = mix(0.0f, 1.5f, grassAmount);
        //gl_Position = projMatrix * viewMatrix * IN[0].instanceMat * (basePosition + crossModel * vertexPosition[i] * grass_size);
	    EmitVertex();
    }
    EndPrimitive();
}

void CreateGrass()
{
    mat4 model0, model45, modelm45;
	model0 = mat4(1.0f);
	model45 = rotationY(radians(45.0f));
	modelm45 = rotationY(-radians(45.0f));
 
	CreateQuad(gl_in[0].gl_Position, model0);
	CreateQuad(gl_in[0].gl_Position, model45);
	CreateQuad(gl_in[0].gl_Position, modelm45);
}

void main()
{
    OUT.colour = IN[0].colour;
    OUT.normal = IN[0].normal; 
    OUT.worldPos = IN[0].worldPos;

    bool fogEnabled = bool(envData.fogData.x);
    if(fogEnabled)
	{
		float fogDensity = envData.fogData.y;
		float fogGradient = envData.fogData.z;
        vec4 posRelativeToCam = (viewMatrix * vec4(IN[0].worldPos, 1.0));

		float distance = length(posRelativeToCam.xyz);
		OUT.visibility = exp(-pow((distance * fogDensity), fogGradient));
		OUT.visibility = clamp(OUT.visibility, 0.0, 1.0);
	}

    grass_size = random(gl_in[0].gl_Position.xz) * (1.0f - c_min_size) + c_min_size;

    CreateGrass();
}


//------------------------------------------------------------------------------------------------------------------------
//UTILITIES
float random(vec2 st) 
{
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

float randomRange(float min, float max, vec2 seed) 
{
    return mix(min, max, random(seed));
}

mat4 rotationX(in float angle) 
{
	return mat4(1.0,		0,			0,			0,
			 	0, 	cos(angle),	-sin(angle),		0,
				0, 	sin(angle),	 cos(angle),		0,
				0, 			0,			  0, 		1);
}
 
mat4 rotationY(in float angle)
{
	return mat4(cos(angle),		0,		sin(angle),	0,
			 			0,		1.0,			 0,	0,
				-sin(angle),	0,		cos(angle),	0,
						0, 		0,				0,	1);
}
 
mat4 rotationZ(in float angle) 
{
	return mat4(cos(angle),		-sin(angle),	0,	0,
			 	sin(angle),		cos(angle),		0,	0,
						0,				0,		1,	0,
						0,				0,		0,	1);
}
//------------------------------------------------------------------------------------------------------------------------