#pragma once
#include "Matrix4.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Mesh.h"
#include <vector>

class SceneNode
{
public:
	std::string nodeName = "Empty Node";

	SceneNode(Mesh* m = NULL, Vector4 col = Vector4());
	~SceneNode(void);

	void SetTransform(const Matrix4& matrix) { localTransform = matrix; }
	const Matrix4& GetTransform() const { return localTransform; }
	const Matrix4& GetWorldTransform() const { return worldTransform; }

	Vector4 GetColour() const { return colour; }
	void SetColor(Vector4 c) { colour = c; }

	Vector3 GetModelScale() const { return modelScale; }
	void SetModelScale(Vector3 s) { modelScale = s; }

	virtual Mesh* GetMesh() const { return mesh; }
	void SetMesh(Mesh* m) { mesh = m; }

	Shader* GetShader() const { return shader; }
	void SetShader(Shader* s) { shader = s; }

	void AddChild(SceneNode* s);
	SceneNode* GetChild(int index);

	virtual void Update(float dt);
	virtual void Draw(const OGLRenderer& r);

	std::vector<SceneNode*>::const_iterator GetChildIteratorStart() { return children.begin(); }
	std::vector<SceneNode*>::const_iterator GetChildIteratorEnd() { return children.end(); }

	float GetBoundingRadius() const { return boundingRadius; }
	void SetBoundingRadius(float f) { boundingRadius = f; }
	
	float GetCameraDistance() const { return distanceFromCamera; }
	void SetCameraDistance(float f) { distanceFromCamera = f; }
	
	void SetTexture(GLuint tex) { texture = tex; }
	GLuint GetTexture() const { return texture; }
	
	static bool CompareByCameraDistance(SceneNode* a, SceneNode* b) 
	{
		return (a->distanceFromCamera < b->distanceFromCamera) ? true : false;
	}

	int GetChildCount() { return children.size(); }

protected:	
	SceneNode* parent;
	Mesh* mesh;
	Matrix4 localTransform;
	Matrix4 worldTransform;
	Vector3 modelScale;
	Vector4 colour;
	Shader* shader;
	std::vector<SceneNode*> children;

	float distanceFromCamera;
	float boundingRadius;
	GLuint texture;
};