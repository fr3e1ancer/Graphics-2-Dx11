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
	TerrainNode(wstring name, wstring heightMap) : SceneNode(name) { _heightMapFile = heightMap; }
	bool Initialise();
	void GenerateVertsAndIndices();
	void CreateBuffers();
	void BuildTextures();
	void GenerateBlendMap();
private:
	wstring _heightMapFile;
};

