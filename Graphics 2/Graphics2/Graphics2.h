#pragma once
#include "DirectXFramework.h"
#include "CubeNode.h"
#include "MeshNode.h"
#include "TerrainNode.h"

class Graphics2 : public DirectXFramework
{
public:
	void CreateSceneGraph();
	void UpdateSceneGraph();

private:
	float _rotation;
};

