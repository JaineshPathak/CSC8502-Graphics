#include "ShadowBuffer.h"
#include "SceneRenderer.h"

#include <nclgl/Camera.h>

#include <iostream>
#include <glad/glad.h>

const float MAX_SHADOW_DISTANCE = 1000.0f;

ShadowBuffer::ShadowBuffer(const int& width, const int& height) : 
	m_Width(width), m_Height(height),
	m_DepthFrameBufferID(-1), m_DepthTexture(-1),
	m_NearWidth(0.0f), m_NearHeight(0.0f),
	m_FarWidth(0.0f), m_FarHeight(0.0f),
	m_MinX(0.0f), m_MaxX(0.0f),
	m_MinY(0.0f), m_MaxY(0.0f),
	m_MinZ(0.0f), m_MaxZ(0.0f)
{
	//Creating Depth Texture
	glGenTextures(1, &m_DepthTexture);
	glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	//Creating Frame Buffer
	glGenFramebuffers(1, &m_DepthFrameBufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_DepthFrameBufferID);
	//Attaching the Depth Texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Shadow Buffer: Ready" << std::endl;

	glDrawBuffer(GL_NONE);	//No Color Attachment needed
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_Camera = SceneRenderer::Get()->GetCamera();
	m_VertexPoints.resize(8, Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	m_LightViewMatrix = Matrix4::BuildViewMatrix(Vector3(-0.15f, -1.0f, -1.0f), Vector3(0, 0, 0), Vector3(0.0f, 1.0f, 0.0f));

	CalcWidthHeight();
}

ShadowBuffer::~ShadowBuffer()
{
	glDeleteTextures(1, &m_DepthTexture);
	glDeleteFramebuffers(1, &m_DepthFrameBufferID);
}

void ShadowBuffer::Bind()
{
	glViewport(0, 0, m_Width, m_Height);
	glBindFramebuffer(GL_FRAMEBUFFER, m_DepthFrameBufferID);
}

void ShadowBuffer::UnBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowBuffer::Update(float DeltaTime)
{
	if (!m_Camera) return;

	const Matrix4 camRotMat = m_Camera->GetRotationMatrix();
	const Vector3 camForward = m_Camera->GetForward();

	Vector3 toNear = camForward * m_Camera->GetNear();
	Vector3 toFar = camForward * MAX_SHADOW_DISTANCE;

	Vector3 centerNear = toNear + m_Camera->getPosition();
	Vector3 centerFar = toFar + m_Camera->getPosition();

	CalcFrustumVertices(camRotMat, camForward, centerNear, centerFar);

	bool firstTime = true;
	for (Vector4 vertexPoint : m_VertexPoints)
	{
		if (firstTime)
		{
			m_MinX = vertexPoint.x;
			m_MaxX = vertexPoint.x;

			m_MinY = vertexPoint.y;
			m_MaxY = vertexPoint.y;

			m_MinZ = vertexPoint.z;
			m_MaxZ = vertexPoint.z;

			firstTime = false;
		}

		if (vertexPoint.x > m_MaxX)			m_MaxX = vertexPoint.x;
		else if (vertexPoint.x < m_MinX)	m_MinX = vertexPoint.x;

		if (vertexPoint.y > m_MaxY)			m_MaxY = vertexPoint.y;
		else if (vertexPoint.y < m_MinY)	m_MinY = vertexPoint.y;

		if (vertexPoint.z > m_MaxZ)			m_MaxZ = vertexPoint.z;
		else if (vertexPoint.z < m_MinZ)	m_MinZ = vertexPoint.z;

		//TODO: ADD OFFSET to m_MaxZ?
	}
}

void ShadowBuffer::CalcWidthHeight()
{
	const float camFOV = m_Camera->GetFOV();
	const float camNear = m_Camera->GetNear();
	const float screenAspect = m_Camera->GetScreenWidth() / m_Camera->GetScreenHeight();

	m_NearWidth = (float)(camNear * std::tanf(DegToRad(camFOV)));
	m_FarWidth = (float)(MAX_SHADOW_DISTANCE * std::tanf(DegToRad(camFOV)));

	m_NearHeight = m_NearWidth / screenAspect;
	m_FarHeight = m_FarWidth / screenAspect;

	/*std::cout << "Near Width: " << m_NearWidth << std::endl;
	std::cout << "Far Width: " << m_FarWidth << std::endl;
	std::cout << "Near Height: " << m_NearHeight << std::endl;
	std::cout << "Far Height: " << m_FarHeight << std::endl;*/
}

void ShadowBuffer::CalcFrustumVertices(const Matrix4& camRotMat, const Vector3& camForward, const Vector3& nearCenter, const Vector3& farCenter)
{
	const Vector3 camUp = m_Camera->GetUp();
	const Vector3 camRight = m_Camera->GetRight();
	
	const Vector3 camLeft = -camRight;
	const Vector3 camDown = -camUp;

	const Vector3 farTop = farCenter + (camUp * m_FarHeight);
	const Vector3 farBottom = farCenter + (camDown * m_FarHeight);
	
	const Vector3 nearTop = nearCenter + (camUp * m_NearHeight);
	const Vector3 nearBottom = nearCenter + (camDown * m_NearHeight);

	m_VertexPoints[0] = CalcFrustumCornerLightSpace(farTop, camRight, m_FarWidth);
	m_VertexPoints[1] = CalcFrustumCornerLightSpace(farTop, camLeft, m_FarWidth);
	m_VertexPoints[2] = CalcFrustumCornerLightSpace(farBottom, camRight, m_FarWidth);
	m_VertexPoints[3] = CalcFrustumCornerLightSpace(farBottom, camLeft, m_FarWidth);

	m_VertexPoints[4] = CalcFrustumCornerLightSpace(nearTop, camRight, m_NearWidth);
	m_VertexPoints[5] = CalcFrustumCornerLightSpace(nearTop, camLeft, m_NearWidth);
	m_VertexPoints[6] = CalcFrustumCornerLightSpace(nearBottom, camRight, m_NearWidth);
	m_VertexPoints[7] = CalcFrustumCornerLightSpace(nearBottom, camLeft, m_NearWidth);
}

Vector4 ShadowBuffer::CalcFrustumCornerLightSpace(const Vector3& startPoint, const Vector3& direction, const float& width)
{
	const Vector3 point = startPoint + (direction * width);
	
	Vector4 point4 = Vector4(point.x, point.y, point.z, 1.0f);
	point4 = m_LightViewMatrix * point4;
	
	return point4;
}

Vector3 ShadowBuffer::GetBoxCenter()
{
	float x = (m_MinX + m_MaxX) / 2.0f;
	float y = (m_MinY + m_MaxY) / 2.0f;
	float z = (m_MinZ + m_MaxZ) / 2.0f;

	Vector4 cen = Vector4(x, y, z, 1.0f);
	Matrix4 invertedLightView = Matrix4();
	invertedLightView.Invert();

	Vector4 cenInv = invertedLightView * cen;

	return Vector3(cenInv.x, cenInv.y, cenInv.z);
}

float ShadowBuffer::GetBoxWidth()
{
	return m_MaxX - m_MinX;
}

float ShadowBuffer::GetBoxHeight()
{
	return m_MaxY - m_MinY;
}

float ShadowBuffer::GetBoxLength()
{
	return m_MaxZ - m_MinZ;
}
