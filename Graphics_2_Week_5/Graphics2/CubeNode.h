#pragma once
#include "SceneNode.h"
#include "DirectXFramework.h"
#include "WICTextureLoader.h"

class CubeNode : public SceneNode
{
	struct Vertex
	{
		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT2 TextureCoordinate;
	};

	struct CBUFFER
	{
		XMMATRIX    CompleteTransformation;
		XMMATRIX	WorldTransformation;
		XMVECTOR    LightVector;
		XMFLOAT4    LightColour;
		XMFLOAT4    AmbientColour;
	};

public:
	CubeNode(wstring name, wstring file) : SceneNode(name) { _file = file; };
	~CubeNode();

	virtual bool Initialise();
	virtual void Update(FXMMATRIX& currentWorldTransformation) { XMStoreFloat4x4(&_worldTransformation, XMLoadFloat4x4(&_localTransformation) * currentWorldTransformation); }
	virtual void Render();
	virtual void Shutdown();
	

	//Uneccessary Methods used only in SceneGraph
	virtual void Add(SceneNodePointer node) {}
	virtual void Remove(SceneNodePointer node) {};
	virtual	SceneNodePointer Find(wstring name) { return (_name == name) ? shared_from_this() : nullptr; }

	//DX11 Methods to do the thing
	void BuildGeometryBuffers();
	void BuildShaders();
	void BuildVertexLayout();
	void BuildConstantBuffer();
	void BuildTexture();

private:
	wstring										_file;

	//Geometry Buffers
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
	
	CBUFFER										_cBuffer;
};

