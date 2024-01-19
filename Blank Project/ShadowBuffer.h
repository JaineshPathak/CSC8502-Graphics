#pragma once

class ShadowBuffer
{
public:
	ShadowBuffer(const int& width, const int& height);
	~ShadowBuffer();

	void Bind();
	void UnBind();

	inline const unsigned int GetDepthTexture() const { return m_DepthTexture; }
	inline const unsigned int GetDepthFrameBufferID() const { return m_DepthFrameBufferID; }

private:
	int m_Width;
	int m_Height;
	unsigned int m_DepthFrameBufferID;
	unsigned int m_DepthTexture;
};