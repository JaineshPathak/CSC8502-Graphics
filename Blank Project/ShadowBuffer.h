#pragma once
#include <memory>
#include <vector>

#include <nclgl/Matrix4.h>

class Camera;
class ShadowBuffer
{
public:
	ShadowBuffer(const int& width, const int& height);
	~ShadowBuffer();

	void Bind();
	void UnBind();

	void Update(float DeltaTime);

	void CalcWidthHeight();
	void CalcFrustumVertices(const Matrix4& camRotMat, const Vector3& camForward, const Vector3& nearCenter, const Vector3& farCenter);
	
	//Calculates one of the corner vertices of the view frustum in world space and converts it to light space.
	Vector4 CalcFrustumCornerLightSpace(const Vector3& startPoint, const Vector3& direction, const float& width);

	Vector3 GetBoxCenter();
	float GetBoxWidth();
	float GetBoxHeight();
	float GetBoxLength();

	inline const unsigned int GetDepthTexture() const { return m_DepthTexture; }
	inline const unsigned int GetDepthFrameBufferID() const { return m_DepthFrameBufferID; }

private:
	int m_Width;
	int m_Height;
	unsigned int m_DepthFrameBufferID;
	unsigned int m_DepthTexture;

	//Directional Light's View Matrix;
	Matrix4 m_LightViewMatrix;

	//Shadow Orthographic Bounding Box Parameters
	float m_NearWidth;
	float m_NearHeight;
	float m_FarWidth;
	float m_FarHeight;

	//Bounding Box Parameters
	float m_MinX, m_MaxX;
	float m_MinY, m_MaxY;
	float m_MinZ, m_MaxZ;

	std::vector<Vector4> m_VertexPoints;

	std::shared_ptr<Camera> m_Camera;
};