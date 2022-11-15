#include "SceneNode.h"

SceneNode::SceneNode(Mesh* m, Vector4 col)
{
	mesh = m;
	colour = col;
	parent = NULL;
	modelScale = Vector3(1, 1, 1);
	modelRotation = Vector3(0, 0, 0);

	boundingRadius = 1.0f;
	distanceFromCamera = 0.0f;
	texture = 0;
}

SceneNode::~SceneNode(void)
{
	for (size_t i = 0; i < children.size(); i++)
		delete children[i];
}

void SceneNode::AddChild(SceneNode* s)
{
	children.push_back(s);
	s->parent = this;
}

SceneNode* SceneNode::GetChild(int index)
{
	if (index < 0 || index > children.size())
		return NULL;

	return children[index];
}

void SceneNode::Draw(const OGLRenderer& r)
{
	if (mesh)
		mesh->Draw();
}

void SceneNode::Update(float dt)
{
	worldTransform = parent ? parent->worldTransform * localTransform : localTransform;

	for (vector<SceneNode*>::iterator i = children.begin(); i != children.end(); ++i)
		(*i)->Update(dt);
}