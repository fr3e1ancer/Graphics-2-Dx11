#pragma once
#include "SceneNode.h"
#include "DirectXFramework.h"
#include <vector>

class TerrainNode : public SceneNode
{
	struct Vertex
	{
		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT2 TexCoord;
		XMFLOAT4 BlendMapTexCoord;
	};

	struct CBUFFER
	{
		XMMATRIX    CompleteTransformation;
		XMMATRIX	WorldTransformation;
		XMFLOAT4	CameraTransformation;
		XMVECTOR    LightVector;
		XMFLOAT4    LightColour;
		XMFLOAT4    AmbientColour;
		XMFLOAT4	DiffuseColour;
		XMFLOAT4	SpecularColour;
		float		Shininess;
		XMFLOAT3	Padding;
	};

public:
	TerrainNode(wstring name, wstring heightMap) : SceneNode(name) { _heightMapFile = heightMap; }
	bool Initialise();
	void Render();
	void Shutdown();
	void LoadHeightMap();
	void GenerateVertsAndIndices();
	void BuildGeometryBuffers();
	void BuildShaders();
	void BuildVertexLayout();
	void BuildConstantBuffers();
	void BuildTextures();
	void BuildRenderStates();
	void GenerateBlendMap();
private:
	wstring _heightMapFile;
	
	//Geometry Bufers
	ComPtr<ID3D11Buffer>						_vertexBuffer;
	ComPtr<ID3D11Buffer>						_indexBuffer;

	//Shader Variables
	ComPtr<ID3DBlob>							_vertexShaderByteCode = nullptr;
	ComPtr<ID3DBlob>							_pixelShaderByteCode = nullptr;
	ComPtr<ID3D11VertexShader>					_vertexShader;
	ComPtr<ID3D11PixelShader>					_pixelShader;
	ComPtr<ID3D11InputLayout>					_layout;
	ComPtr<ID3D11Buffer>						_constantBuffer;

	//Texture Variables
	ComPtr<ID3D11ShaderResourceView>			_texture;

	//Rasteriser States
	ComPtr<ID3D11RasterizerState>				_defaultRasteriserState;
	ComPtr<ID3D11RasterizerState>				_wireframeRasteriserState;

	//Buffer data vectors
	vector<Vertex>								_vertices;
	vector<UINT>								_indices;
};

