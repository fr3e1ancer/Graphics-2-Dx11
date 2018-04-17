#include "TerrainNode.h"

#define GRID_SIZE 256
#define NUMBER_OF_ROWS			256
#define NUMBER_OF_COLUMNS	    256

bool TerrainNode::Initialise()
{
	LoadHeightMap(L"rollinghills.bmp");
	GenerateVertsAndIndices();
	BuildGeometryBuffers();
	BuildShaders();
	BuildVertexLayout();
	BuildConstantBuffers();
	BuildRenderStates();
	return true;
}

void TerrainNode::Render()
{
	XMMATRIX completeTransformation = XMLoadFloat4x4(&_worldTransformation) * DirectXFramework::GetDXFramework()->GetViewTransformation() * DirectXFramework::GetDXFramework()->GetProjectionTransformation();
	
	
	CBUFFER _cBuffer;
	_cBuffer.CompleteTransformation = completeTransformation;
	_cBuffer.WorldTransformation = XMLoadFloat4x4(&_localTransformation);
	_cBuffer.CameraTransformation = XMFLOAT4(0.0f, 50.0f, -50.0f, 0.0f);
	_cBuffer.AmbientColour = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	_cBuffer.LightVector = XMVector4Normalize(XMVectorSet(0.0f, 1.0f, 1.0f, 0.0f));
	_cBuffer.LightColour = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	_cBuffer.AmbientColour = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	_cBuffer.DiffuseColour = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	_cBuffer.SpecularColour = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	_cBuffer.Shininess = 1.0f;
	_cBuffer.Padding = XMFLOAT3(1.0f, 1.0f, 1.0f);

	DirectXFramework::GetDXFramework()->GetDeviceContext()->VSSetShader(_vertexShader.Get(), 0, 0);
	DirectXFramework::GetDXFramework()->GetDeviceContext()->PSSetShader(_pixelShader.Get(), 0, 0);
	DirectXFramework::GetDXFramework()->GetDeviceContext()->IASetInputLayout(_layout.Get());

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	DirectXFramework::GetDXFramework()->GetDeviceContext()->VSSetConstantBuffers(0, 1, _constantBuffer.GetAddressOf());
	DirectXFramework::GetDXFramework()->GetDeviceContext()->UpdateSubresource(_constantBuffer.Get(), 0, 0, &_cBuffer, 0, 0);

	DirectXFramework::GetDXFramework()->GetDeviceContext()->IASetVertexBuffers(0, 1, _vertexBuffer.GetAddressOf(), &stride, &offset);
	DirectXFramework::GetDXFramework()->GetDeviceContext()->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	DirectXFramework::GetDXFramework()->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set the texture to be used by the pixel shader
	DirectXFramework::GetDXFramework()->GetDeviceContext()->PSSetShaderResources(0, 1, _texture.GetAddressOf());

	DirectXFramework::GetDXFramework()->GetDeviceContext()->RSSetState(_wireframeRasteriserState.Get());
	DirectXFramework::GetDXFramework()->GetDeviceContext()->DrawIndexed(_indices.size(), 0, 0);
	DirectXFramework::GetDXFramework()->GetDeviceContext()->RSSetState(_defaultRasteriserState.Get());
}

void TerrainNode::Shutdown()
{
}

bool TerrainNode::LoadHeightMap(wstring heightMapFilename)
{
	int fileSize = (NUMBER_OF_COLUMNS + 1) * (NUMBER_OF_ROWS + 1);
	BYTE * rawFileValues = new BYTE[fileSize];

	std::ifstream inputHeightMap;
	inputHeightMap.open(heightMapFilename.c_str(), std::ios_base::binary);
	if (!inputHeightMap)
	{
		return false;
	}

	inputHeightMap.read((char*)rawFileValues, fileSize);
	inputHeightMap.close();

	// Normalise BYTE values to the range 0.0f - 1.0f;
	for (unsigned int i = 0; i < fileSize; i++)
	{
		_heightValues.push_back((float)rawFileValues[i] / 255);
	}
	delete[] rawFileValues;
	return true;
}

void TerrainNode::GenerateVertsAndIndices()
{
	Vertex v1;
	Vertex v2;
	Vertex v3;
	Vertex v4;

	//Clear everything but position
	v1.BlendMapTexCoord = XMFLOAT4(0, 0, 0, 0);
	v1.Normal = XMFLOAT3(0, 0, 0);
	v1.TexCoord = XMFLOAT2(0, 0);

	v2.BlendMapTexCoord = XMFLOAT4(0, 0, 0, 0);
	v2.Normal = XMFLOAT3(0, 0, 0);
	v2.TexCoord = XMFLOAT2(0, 0);

	v3.BlendMapTexCoord = XMFLOAT4(0, 0, 0, 0);
	v3.Normal = XMFLOAT3(0, 0, 0);
	v3.TexCoord = XMFLOAT2(0, 0);

	v4.BlendMapTexCoord = XMFLOAT4(0, 0, 0, 0);
	v4.Normal = XMFLOAT3(0, 0, 0);
	v4.TexCoord = XMFLOAT2(0, 0);

	for (size_t z = 0; z < GRID_SIZE; z++)
	{
		for (size_t x = 0; x < GRID_SIZE; x++)
		{
			//Work out which square we are on
			int square = x + (z * GRID_SIZE);
			//Work out the X value for the top left vertex
			float tlx = (square % GRID_SIZE) - (GRID_SIZE / 2);
			//float tly = (_heightValues[square] * GRID_SIZE) - (GRID_SIZE / 2);
			//Work out the Z value for the top left vertex
			float tlz = (floor(square / GRID_SIZE) - (GRID_SIZE / 2));

			//Set Position
			v1.Position = XMFLOAT3(tlx,		(tly), tlz);			//Top Left vertex			o--------o
			v2.Position = XMFLOAT3(tlx + 1, (tly), tlz);			//Top Right Vertex			|		 |
			v3.Position = XMFLOAT3(tlx,		(tly), tlz - 1);		//Bottom Left Vertex		|		 |
			v4.Position = XMFLOAT3(tlx + 1, (tly), tlz - 1);		//Bottom Right Vertex		o--------o

			//Push back Vertices
			_vertices.push_back(v1);
			_vertices.push_back(v2);
			_vertices.push_back(v3);
			_vertices.push_back(v4);

			//Push back indices

			//Triangle 1, v1
			_indices.push_back((square * 4)); 
			//Triangle 1, v2
			_indices.push_back((square * 4) + 1);
			//Triangle 1, v3
			_indices.push_back((square * 4) + 2);

			//------------------------------------------

			//Triangle 2, v3
			_indices.push_back((square * 4) + 2);
			//Triangle 2, v2
			_indices.push_back((square * 4) + 1);
			//Triangle 2, v4
			_indices.push_back((square * 4) + 3);

		}
	}
}

void TerrainNode::BuildGeometryBuffers()
{
	// Setup the structure that specifies how big the vertex 
	// buffer should be
	D3D11_BUFFER_DESC vertexBufferDescriptor;
	vertexBufferDescriptor.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDescriptor.ByteWidth = sizeof(Vertex) * _vertices.size();
	vertexBufferDescriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDescriptor.CPUAccessFlags = 0;
	vertexBufferDescriptor.MiscFlags = 0;
	vertexBufferDescriptor.StructureByteStride = 0;

	// Now set up a structure that tells DirectX where to get the
	// data for the vertices from
	D3D11_SUBRESOURCE_DATA vertexInitialisationData;
	vertexInitialisationData.pSysMem = &_vertices[0];

	// and create the vertex buffer
	ThrowIfFailed(DirectXFramework::GetDXFramework()->GetDevice()->CreateBuffer(&vertexBufferDescriptor, &vertexInitialisationData, _vertexBuffer.GetAddressOf()));

	// Setup the structure that specifies how big the index 
	// buffer should be
	D3D11_BUFFER_DESC indexBufferDescriptor;
	indexBufferDescriptor.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDescriptor.ByteWidth = sizeof(UINT) * _indices.size();
	indexBufferDescriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDescriptor.CPUAccessFlags = 0;
	indexBufferDescriptor.MiscFlags = 0;
	indexBufferDescriptor.StructureByteStride = 0;

	// Now set up a structure that tells DirectX where to get the
	// data for the indices from
	D3D11_SUBRESOURCE_DATA indexInitialisationData;
	indexInitialisationData.pSysMem = &_indices[0];

	// and create the index buffer
	ThrowIfFailed(DirectXFramework::GetDXFramework()->GetDevice()->CreateBuffer(&indexBufferDescriptor, &indexInitialisationData, _indexBuffer.GetAddressOf()));
}

void TerrainNode::BuildShaders()
{
	DWORD shaderCompileFlags = 0;
#if defined( _DEBUG )
	shaderCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> compilationMessages = nullptr;

	//Compile vertex shader
	HRESULT hr = D3DCompileFromFile(L"TerrainShader.hlsl",
		nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"VShader", "vs_5_0",
		shaderCompileFlags, 0,
		_vertexShaderByteCode.GetAddressOf(),
		compilationMessages.GetAddressOf());

	if (compilationMessages.Get() != nullptr)
	{
		// If there were any compilation messages, display them
		MessageBoxA(0, (char*)compilationMessages->GetBufferPointer(), 0, 0);
	}
	// Even if there are no compiler messages, check to make sure there were no other errors.
	ThrowIfFailed(hr);
	ThrowIfFailed(DirectXFramework::GetDXFramework()->GetDevice()->CreateVertexShader(_vertexShaderByteCode->GetBufferPointer(), _vertexShaderByteCode->GetBufferSize(), NULL, _vertexShader.GetAddressOf()));
	DirectXFramework::GetDXFramework()->GetDeviceContext()->VSSetShader(_vertexShader.Get(), 0, 0);

	// Compile pixel shader
	hr = D3DCompileFromFile(L"TerrainShader.hlsl",
		nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"PShader", "ps_5_0",
		shaderCompileFlags, 0,
		_pixelShaderByteCode.GetAddressOf(),
		compilationMessages.GetAddressOf());

	if (compilationMessages.Get() != nullptr)
	{
		// If there were any compilation messages, display them
		MessageBoxA(0, (char*)compilationMessages->GetBufferPointer(), 0, 0);
	}
	ThrowIfFailed(hr);
	ThrowIfFailed(DirectXFramework::GetDXFramework()->GetDevice()->CreatePixelShader(_pixelShaderByteCode->GetBufferPointer(), _pixelShaderByteCode->GetBufferSize(), NULL, _pixelShader.GetAddressOf()));
	DirectXFramework::GetDXFramework()->GetDeviceContext()->PSSetShader(_pixelShader.Get(), 0, 0);
}

void TerrainNode::BuildVertexLayout()
{
	// Create the vertex input layout. This tells DirectX the format
	// of each of the vertices we are sending to it.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "Position",			0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,								D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "Normal",				0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TexCoord",			0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BlendMapTexCoord",	0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ThrowIfFailed(DirectXFramework::GetDXFramework()->GetDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), _vertexShaderByteCode->GetBufferPointer(), _vertexShaderByteCode->GetBufferSize(), _layout.GetAddressOf()));
	DirectXFramework::GetDXFramework()->GetDeviceContext()->IASetInputLayout(_layout.Get());
}

void TerrainNode::BuildConstantBuffers()
{
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(CBUFFER);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	ThrowIfFailed(DirectXFramework::GetDXFramework()->GetDevice()->CreateBuffer(&bufferDesc, NULL, _constantBuffer.GetAddressOf()));
}

void TerrainNode::BuildTextures()
{
}

void TerrainNode::BuildRenderStates()
{
	// Set default and wireframe rasteriser states
	D3D11_RASTERIZER_DESC rasteriserDesc;
	rasteriserDesc.FillMode = D3D11_FILL_SOLID;
	rasteriserDesc.CullMode = D3D11_CULL_NONE;
	rasteriserDesc.FrontCounterClockwise = false;
	rasteriserDesc.DepthBias = 0;
	rasteriserDesc.SlopeScaledDepthBias = 0.0f;
	rasteriserDesc.DepthBiasClamp = 0.0f;
	rasteriserDesc.DepthClipEnable = true;
	rasteriserDesc.ScissorEnable = false;
	rasteriserDesc.MultisampleEnable = false;
	rasteriserDesc.AntialiasedLineEnable = false;
	ThrowIfFailed(DirectXFramework::GetDXFramework()->GetDevice()->CreateRasterizerState(&rasteriserDesc, _defaultRasteriserState.GetAddressOf()));
	rasteriserDesc.FillMode = D3D11_FILL_WIREFRAME;
	ThrowIfFailed(DirectXFramework::GetDXFramework()->GetDevice()->CreateRasterizerState(&rasteriserDesc, _wireframeRasteriserState.GetAddressOf()));
}

void TerrainNode::GenerateBlendMap()
{
}
