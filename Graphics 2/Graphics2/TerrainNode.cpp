#include "TerrainNode.h"

#define GRID_SIZE				257
#define NUMBER_OF_ROWS			256
#define NUMBER_OF_COLUMNS	    256

bool TerrainNode::Initialise()
{
	LoadHeightMap(_heightMapFile);
	GenerateVertsAndIndices();
	GenerateNormals();
	BuildGeometryBuffers();
	BuildShaders();
	BuildVertexLayout();
	BuildConstantBuffers();
	LoadTerrainTextures();
	GenerateBlendMap();
	BuildRenderStates();
	return true;
}

void TerrainNode::Render()
{
	XMMATRIX completeTransformation = XMLoadFloat4x4(&_worldTransformation) * DirectXFramework::GetDXFramework()->GetViewTransformation() * DirectXFramework::GetDXFramework()->GetProjectionTransformation();
	
	
	CBUFFER _cBuffer;
	_cBuffer.CompleteTransformation = completeTransformation;
	_cBuffer.WorldTransformation = XMLoadFloat4x4(&_localTransformation);
	_cBuffer.CameraTransformation = XMFLOAT4(0.0f, 50.0f, -500.0f, 0.0f);
	_cBuffer.LightVector = XMVector4Normalize(XMVectorSet(0.0f, 1.0f, 1.0f, 0.0f));
	_cBuffer.LightColour = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f);
	_cBuffer.AmbientColour = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	_cBuffer.DiffuseColour = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	_cBuffer.SpecularColour = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	_cBuffer.Shininess = 0.0f;
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
	DirectXFramework::GetDXFramework()->GetDeviceContext()->PSSetShaderResources(0, 1, _blendMapResourceView.GetAddressOf());
	DirectXFramework::GetDXFramework()->GetDeviceContext()->PSSetShaderResources(1, 1, _texturesResourceView.GetAddressOf());

	//DirectXFramework::GetDXFramework()->GetDeviceContext()->RSSetState(_wireframeRasteriserState.Get());
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
	for (unsigned int i = 0; i < (UINT)fileSize; i++)
	{
		_heightValues.push_back((float)rawFileValues[i] / 255);
	}
	delete[] rawFileValues;
	return true;
}

void TerrainNode::GenerateVertsAndIndices()
{
	Vertex v0;
	Vertex v1;
	Vertex v2;
	Vertex v3;

	//Clear everything but position
	v0.BlendMapTexCoord = XMFLOAT2(0, 0);
	v0.Normal = XMFLOAT3(0, 0, 0);
	v0.TexCoord = XMFLOAT2(0, 0);

	v1.BlendMapTexCoord = XMFLOAT2(0, 0);
	v1.Normal = XMFLOAT3(0, 0, 0);
	v1.TexCoord = XMFLOAT2(0, 0);

	v2.BlendMapTexCoord = XMFLOAT2(0, 0);
	v2.Normal = XMFLOAT3(0, 0, 0);
	v2.TexCoord = XMFLOAT2(0, 0);

	v3.BlendMapTexCoord = XMFLOAT2(0, 0);
	v3.Normal = XMFLOAT3(0, 0, 0);
	v3.TexCoord = XMFLOAT2(0, 0);

	for (size_t z = 0; z < NUMBER_OF_COLUMNS; z++)
	{
		for (size_t x = 0; x < NUMBER_OF_ROWS; x++)
		{
			//Work out which square we are on
			int square = x + (z * NUMBER_OF_COLUMNS);
			//Work out the X value for the top left vertex
			float tlx = (square % NUMBER_OF_COLUMNS) - (NUMBER_OF_COLUMNS / 2);
			//Work out the Z value for the top left vertex
			float tlz = (floor(square / (-NUMBER_OF_COLUMNS)) + (NUMBER_OF_COLUMNS / 2));

			//Set Position
			v0.Position = XMFLOAT3(tlx,		_heightValues[(z * GRID_SIZE) + x]	   * 50,  tlz);			    //Top Left vertex			o--------o
			v1.Position = XMFLOAT3(tlx + 1, _heightValues[(z * GRID_SIZE) + x + 1]   * 50,  tlz);			//Top Right Vertex  		|		 |
			v2.Position = XMFLOAT3(tlx,		_heightValues[(z * GRID_SIZE) + x + 257] * 50,  tlz - 1);		//Bottom Left Vertex		|		 |
			v3.Position = XMFLOAT3(tlx + 1, _heightValues[(z * GRID_SIZE) + x + 258] * 50,  tlz - 1);		//Bottom Right Vertex		o--------o
			
			//Set Texture Coordinates U,V
			v0.TexCoord = XMFLOAT2(0, 0);
			v1.TexCoord = XMFLOAT2(1, 0);
			v2.TexCoord = XMFLOAT2(0, 1);
			v3.TexCoord = XMFLOAT2(1, 1);

			float divisor = 257.0f; //Divisor for blend map coords because integer division is bad so dont use GRID_SIZE.

			//Do math for blend maps texture coords
			v0.BlendMapTexCoord = XMFLOAT2(		 x / divisor,		  z / divisor); //Top Left Vertex
			v1.BlendMapTexCoord = XMFLOAT2((x + 1) / divisor,		  z / divisor); //Top Right Vertex
			v2.BlendMapTexCoord = XMFLOAT2(      x / divisor,	(z + 1) / divisor); //Bottom Left Vertex
			v3.BlendMapTexCoord = XMFLOAT2((x + 1) / divisor,	(z + 1) / divisor); //Bottom Right Vertex

			//Push back Vertices
			_vertices.push_back(v0);
			_vertices.push_back(v1);
			_vertices.push_back(v2);
			_vertices.push_back(v3);

			//Push back indices

			//Triangle 1, v0
			_indices.push_back((square * 4));
			//Triangle 1, v1
			_indices.push_back((square * 4) + 1);
			//Triangle 1, v2
			_indices.push_back((square * 4) + 2);

			//------------------------------------------

			//Triangle 2, v2
			_indices.push_back((square * 4) + 2);
			//Triangle 2, v1
			_indices.push_back((square * 4) + 1);
			//Triangle 2, v3
			_indices.push_back((square * 4) + 3);

		}
	}
}

void TerrainNode::GenerateNormals()
{
	int numberOfSquares = NUMBER_OF_COLUMNS * NUMBER_OF_ROWS;
	for (int i = 0; i < numberOfSquares; i++)
	{
		XMFLOAT3 v0 = _vertices[(i * 4)].Position;
		XMFLOAT3 v1 = _vertices[(i * 4) + 1].Position;
		XMFLOAT3 v2 = _vertices[(i * 4) + 2].Position;
		XMFLOAT3 v3 = _vertices[(i * 4) + 3].Position;

		XMVECTOR n1 = CalculateNormals(v0, v1, v2);
		XMVECTOR n2 = CalculateNormals(v2, v1, v3);

		//Setting the first Vertex normal in polygon 1
		XMStoreFloat3(&_vertices[i * 4].Normal, n1);
		//Setting the last Vertex normal in polygon 2
		XMStoreFloat3(&_vertices[(i * 4) + 3].Normal, n2);
		//Averaging the normal of polygon 1 and polygon 2
		XMVECTOR avgNormal = XMVector3Normalize((n1 + n2) / 2);
		//Setting the second and thid vertex to the average
		XMStoreFloat3(&_vertices[(i * 4) + 1].Normal, avgNormal);
		XMStoreFloat3(&_vertices[(i * 4) + 2].Normal, avgNormal);
	}
}

XMVECTOR TerrainNode::CalculateNormals(XMFLOAT3 v0, XMFLOAT3 v1, XMFLOAT3 v2)
{
	return XMVector3Normalize(XMVector3Cross(XMVectorSet(v0.x - v2.x,
														 v0.y - v2.y,
														 v0.z - v2.z,
														 0.0f),
											 XMVectorSet(v0.x - v1.x,
												         v0.y - v1.y,
												         v0.z - v1.z,
												         0.0f)));
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
		{ "Position",			0, DXGI_FORMAT_R32G32B32_FLOAT, 0,							  0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
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
	rasteriserDesc.CullMode = D3D11_CULL_BACK;
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

void TerrainNode::LoadTerrainTextures()
{
	// Change the paths below as appropriate for your use
	wstring terrainTextureNames[] = { L"grass.dds", L"darkdirt.dds", L"stone.dds", L"lightdirt.dds", L"snow.dds" };

	// Load the textures from the files
	ComPtr<ID3D11Resource> terrainTextures[5];
	for (int i = 0; i < 5; i++)
	{
		ThrowIfFailed(CreateDDSTextureFromFileEx(DirectXFramework::GetDXFramework()->GetDevice().Get(),
					  DirectXFramework::GetDXFramework()->GetDeviceContext().Get(),
					  terrainTextureNames[i].c_str(),
					  0,
					  D3D11_USAGE_IMMUTABLE,
					  D3D11_BIND_SHADER_RESOURCE,
					  0,
					  0,
					  false,
					  terrainTextures[i].GetAddressOf(),
					  nullptr
		));
	}
	// Now create the Texture2D arrary.  We assume all textures in the
	// array have the same format and dimensions

	D3D11_TEXTURE2D_DESC textureDescription;
	ComPtr<ID3D11Texture2D> textureInterface;
	terrainTextures[0].As<ID3D11Texture2D>(&textureInterface);
	textureInterface->GetDesc(&textureDescription);

	D3D11_TEXTURE2D_DESC textureArrayDescription;
	textureArrayDescription.Width = textureDescription.Width;
	textureArrayDescription.Height = textureDescription.Height;
	textureArrayDescription.MipLevels = textureDescription.MipLevels;
	textureArrayDescription.ArraySize = 5;
	textureArrayDescription.Format = textureDescription.Format;
	textureArrayDescription.SampleDesc.Count = 1;
	textureArrayDescription.SampleDesc.Quality = 0;
	textureArrayDescription.Usage = D3D11_USAGE_DEFAULT;
	textureArrayDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureArrayDescription.CPUAccessFlags = 0;
	textureArrayDescription.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> textureArray = 0;
	ThrowIfFailed(DirectXFramework::GetDXFramework()->GetDevice()->CreateTexture2D(&textureArrayDescription, 0, textureArray.GetAddressOf()));

	// Copy individual texture elements into texture array.

	for (UINT i = 0; i < 5; i++)
	{
		// For each mipmap level...
		for (UINT mipLevel = 0; mipLevel < textureDescription.MipLevels; mipLevel++)
		{
			DirectXFramework::GetDXFramework()->GetDeviceContext()->CopySubresourceRegion(textureArray.Get(),
																						  D3D11CalcSubresource(mipLevel, i, textureDescription.MipLevels),
																						  NULL,
																						  NULL,
																						  NULL,
																						  terrainTextures[i].Get(),
																						  mipLevel,
																						  nullptr
			);
		}
	}

	// Create a resource view to the texture array.
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDescription;
	viewDescription.Format = textureArrayDescription.Format;
	viewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDescription.Texture2DArray.MostDetailedMip = 0;
	viewDescription.Texture2DArray.MipLevels = textureArrayDescription.MipLevels;
	viewDescription.Texture2DArray.FirstArraySlice = 0;
	viewDescription.Texture2DArray.ArraySize = 5;

	ThrowIfFailed(DirectXFramework::GetDXFramework()->GetDevice()->CreateShaderResourceView(textureArray.Get(), &viewDescription, _texturesResourceView.GetAddressOf()));
}

void TerrainNode::GenerateBlendMap()
{
	DWORD blendMap[GRID_SIZE * GRID_SIZE];

	for (DWORD z = 0; z < GRID_SIZE; z++)
	{
		for (DWORD x = 0; x < GRID_SIZE; x++)
		{
			int square = (z * NUMBER_OF_ROWS) + x;
			float height = _heightValues[square + z];

			BYTE r = 0;
			if (height >= 0.1f)
			{
				r = 64;
			}

			BYTE g = 0;
			if (height >= 0.4f && height <= 0.6f)
			{
				g = 64;
			} else if (height >= 0.6f && height <= 0.8f) {
				g = 255;
			}

			BYTE b = 0;
			if (height >= 0.2f && height <= 0.6f)
			{
				b = 64;
			}

			BYTE a = 0;
			if (height >= 0.6f)
			{
				g = 255;
			}

			blendMap[square] = (r) | (g << 8) | (b << 16) | (a << 24);
		}
	}
	// Now create the texture from the raw blend map data
	D3D11_TEXTURE2D_DESC blendMapDescription;
	blendMapDescription.Width = NUMBER_OF_ROWS;
	blendMapDescription.Height = NUMBER_OF_COLUMNS;
	blendMapDescription.MipLevels = 1;
	blendMapDescription.ArraySize = 1;
	blendMapDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	blendMapDescription.SampleDesc.Count = 1;
	blendMapDescription.SampleDesc.Quality = 0;
	blendMapDescription.Usage = D3D11_USAGE_DEFAULT;
	blendMapDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	blendMapDescription.CPUAccessFlags = 0;
	blendMapDescription.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA blendMapInitialisationData;
	blendMapInitialisationData.pSysMem = &blendMap;
	blendMapInitialisationData.SysMemPitch = 4 * NUMBER_OF_ROWS;

	ComPtr<ID3D11Texture2D> blendMapTexture;
	ThrowIfFailed(DirectXFramework::GetDXFramework()->GetDevice()->CreateTexture2D(&blendMapDescription, &blendMapInitialisationData, blendMapTexture.GetAddressOf()));

	// Create a resource view for the blend map.
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDescription;
	viewDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	viewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	viewDescription.Texture2D.MostDetailedMip = 0;
	viewDescription.Texture2D.MipLevels = 1;

	ThrowIfFailed(DirectXFramework::GetDXFramework()->GetDevice()->CreateShaderResourceView(blendMapTexture.Get(), &viewDescription, _blendMapResourceView.GetAddressOf()));
}