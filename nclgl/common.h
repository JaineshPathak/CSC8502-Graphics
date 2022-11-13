/******************************************************************************
Author:Rich Davison
Description: Some random variables and functions, for lack of a better place 
to put them.

-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef  WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // ! WIN32_LEAN_AND_MEAN

#ifndef  NOMINMAX
#define NOMINMAX
#endif // ! NOMINMAX


//It's pi(ish)...
static const float		PI = 3.14159265358979323846f;

//It's pi...divided by 360.0f!
static const float		PI_OVER_360 = PI / 360.0f;

//Radians to degrees
static inline float RadToDeg(const float deg)	{
	return deg * 180.0f / PI;
};

//Degrees to radians
static inline float DegToRad(const float rad)	{
	return rad * PI / 180.0f;
};

static inline float naive_lerp(float a, float b, float t)
{
	return a + t * (b - a);
}

static inline float Clamp(const float& val, const float& min, const float& max)
{
	if (val < min)
		return min;
	else if (val > max)
		return max;

	return val;
}

#define SHADERDIR	"../Shaders/"
#define MESHDIR		"../Meshes/"
#define TEXTUREDIR  "../Textures/"
#define SOUNDSDIR	"../Sounds/"

#define SHADERDIRCOURSE	"../Shaders/Coursework/"
#define TEXTUREDIRCOURSE  "../Textures/Coursework/"

#define SHADERDIRCOURSETERRAIN	"../Shaders/Coursework/Terrain/"
#define TEXTUREDIRCOURSETERRAIN  "../Textures/Coursework/Terrain/"