#pragma once
class TerrainNode : public SceneNode
{
	struct VERTEX
	{
		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT2 TexCoord;
		XMFLOAT4 Colour;
	};

public:
	TerrainNode();

	bool Initialise();
	void GeneratePolygons();
	void CreateBuffers();
	void BuildTextures();
	void GenerateBlendMap();
};

