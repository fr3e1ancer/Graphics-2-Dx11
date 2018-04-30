struct TerrainVertex
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
	XMFLOAT2 BlendMapTexCoord;
};

	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};



void TerrainNode::LoadTerrainTextures()
{
	// Change the paths below as appropriate for your use
	wstring terrainTextureNames[5] = { L"C:\\temp\\grass.dds", L"C:\\temp\\darkdirt.dds", L"C:\\temp\\stone.dds", L"C:\\temp\\lightdirt.dds", L"C:\\temp\\snow.dds" };

	// Load the textures from the files
	ComPtr<ID3D11Resource> terrainTextures[5];
	for (int i = 0; i < 5; i++)
	{
		ThrowIfFailed(CreateDDSTextureFromFileEx(_device.Get(),
											     _deviceContext.Get(),
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
	ThrowIfFailed(_device->CreateTexture2D(&textureArrayDescription, 0, textureArray.GetAddressOf()));

	// Copy individual texture elements into texture array.

	for (UINT i = 0; i < 5; i++)
	{
		// For each mipmap level...
		for (UINT mipLevel = 0; mipLevel < textureDescription.MipLevels; mipLevel++)
		{
			_deviceContext->CopySubresourceRegion(textureArray.Get(),
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

	ThrowIfFailed(_device->CreateShaderResourceView(textureArray.Get(), &viewDescription, _texturesResourceView.GetAddressOf()));
}

void TerrainNode::GenerateBlendMap()
{	
	DWORD blendMap[NUMBER_OF_ROWS * NUMBER_OF_COLUMNS];

	for (DWORD i = 0; i < NUMBER_OF_COLUMNS; i++)
	{
		for (DWORD j = 0; j < NUMBER_OF_ROWS; j++)
		{
			// Calculate the appropriate blend colour for the 
			// current location in the blend map.  This has been
			// left as an exercise for you.  You need to calculate
			// appropriate values for the r, g, b and a values (each
			// between 0 and 255) and then combine then into a DWORD
			// (32-bit value) to store in the blend map.
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
	ThrowIfFailed(_device->CreateTexture2D(&blendMapDescription, &blendMapInitialisationData, blendMapTexture.GetAddressOf()));

	// Create a resource view for the blend map.
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDescription;
	viewDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	viewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	viewDescription.Texture2D.MostDetailedMip = 0;
	viewDescription.Texture2D.MipLevels = 1;

	ThrowIfFailed(_device->CreateShaderResourceView(blendMapTexture.Get(), &viewDescription, _blendMapResourceView.GetAddressOf()));

}