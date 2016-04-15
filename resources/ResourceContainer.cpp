#include "ResourceContainer.h"
#include <D3D11.h>
#include <D3Dcompiler.h>
#include "..\io\json.h"
#include "ResourceDescriptors.h"
#include "..\graphics.h"
#include <string.h>
#include "..\utils\Log.h"
#include "..\renderer\render_types.h"

namespace ds {

	struct BlendStateMapping {
		const char* name;
		D3D11_BLEND blend;
		BlendStateMapping(const char* n, D3D11_BLEND b) : name(n), blend(b) {}
	};

	static const BlendStateMapping BLEND_STATE_MAPPINGS[] = {
		{ "ZERO", D3D11_BLEND_ZERO },
		{ "ONE", D3D11_BLEND_ONE },
		{ "SRC_COLOR", D3D11_BLEND_SRC_COLOR },
		{ "INV_SRC_COLOR", D3D11_BLEND_INV_SRC_COLOR },
		{ "SRC_ALPHA", D3D11_BLEND_SRC_ALPHA },
		{ "INV_SRC_ALPHA", D3D11_BLEND_INV_SRC_ALPHA },
		{ "DEST_ALPHA", D3D11_BLEND_DEST_ALPHA },
		{ "INV_DEST_ALPHA", D3D11_BLEND_INV_DEST_ALPHA },
		{ "DEST_COLOR", D3D11_BLEND_DEST_COLOR },
		{ "INV_DEST_COLOR", D3D11_BLEND_INV_DEST_COLOR },
		{ "SRC_ALPHA_SAT", D3D11_BLEND_SRC_ALPHA_SAT },
		{ "BLEND_FACTOR", D3D11_BLEND_BLEND_FACTOR },
		{ "INV_BLEND_FACTOR", D3D11_BLEND_INV_BLEND_FACTOR },
		{ "SRC1_COLOR", D3D11_BLEND_SRC1_COLOR },
		{ "INV_SRC1_COLOR", D3D11_BLEND_INV_SRC1_COLOR },
		{ "SRC1_ALPHA", D3D11_BLEND_SRC1_ALPHA },
		{ "INV_SRC1_ALPHA", D3D11_BLEND_INV_SRC1_ALPHA },
	};

	struct InputElementDescriptor {

		const char* semantic;
		uint32_t semanticIndex;
		DXGI_FORMAT format;
		uint32_t size;

		InputElementDescriptor(const char* sem, uint32_t index, DXGI_FORMAT f, uint32_t s) :
			semantic(sem), semanticIndex(index), format(f), size(s) {
		}
	};

	static const InputElementDescriptor INPUT_ELEMENT_DESCRIPTIONS[] = {

		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 12 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 12 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 8 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 8 },
		{ "TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT, 8 },
		{ "TEXCOORD", 3, DXGI_FORMAT_R32G32_FLOAT, 8 },
	};

	

	ResourceContainer::ResourceContainer(ID3D11Device* d3dDevice) : _device(d3dDevice) {
		_resourceIndex = 0;
		for (uint32_t i = 0; i < MAX_RESOURCES; ++i) {
			ResourceIndex& index = _resourceTable[i];
			index.id = INVALID_RID;
			index.index = 0;
			index.type = ResourceType::UNKNOWN;
		}
	}


	ResourceContainer::~ResourceContainer() {
		for (size_t i = 0; i < _indexBuffers.size(); ++i) {
			_indexBuffers[i]->Release();
		}
		for (size_t i = 0; i < _blendStates.size(); ++i) {
			_blendStates[i]->Release();
		}
		for (size_t i = 0; i < _layouts.size(); ++i) {
			_layouts[i]->Release();
		}
		for (size_t i = 0; i < _shaders.size(); ++i) {
			Shader* shader = _shaders[i];
			if (shader->vertexShader != 0) {
				shader->vertexShader->Release();
			}
			if (shader->pixelShader != 0) {
				shader->pixelShader->Release();
			}
			if (shader->vertexShaderBuffer != 0) {
				shader->vertexShaderBuffer->Release();
			}
			if (shader->samplerState != 0) {
				shader->samplerState->Release();
			}
			delete _shaders[i];
		}
		for (size_t i = 0; i < _shaderResourceViews.size(); ++i) {
			_shaderResourceViews[i]->Release();
		}
		for (size_t i = 0; i < _vertexBuffers.size(); ++i) {
			_vertexBuffers[i]->Release();
		}
		for (size_t i = 0; i < _constantBuffers.size(); ++i) {
			_constantBuffers[i]->Release();
		}
		for (size_t i = 0; i < _fonts.size(); ++i) {
			delete _fonts[i];
		}
		for (size_t i = 0; i < _spriteBuffers.size(); ++i) {
			delete _spriteBuffers[i];
		}
	}

	void ResourceContainer::parseJSONFile() {
		JSONReader reader;
		bool ret = reader.parse("content\\resources.json");
		assert(ret);
		int children[256];
		int num = reader.get_categories(children, 256);
		for (int i = 0; i < num; ++i) {
			if (reader.matches(children[i], "quad_index_buffer")) {
				QuadIndexBufferDescriptor descriptor;
				reader.get(children[i], "id", &descriptor.id);
				reader.get(children[i], "size", &descriptor.size);
				createQuadIndexBuffer(descriptor);
			}
			else if (reader.matches(children[i], "constant_buffer")) {
				ConstantBufferDescriptor descriptor;
				reader.get(children[i], "id", &descriptor.id);
				reader.get(children[i], "size", &descriptor.size);
				createConstantBuffer(descriptor);
			}
			else if (reader.matches(children[i], "vertex_buffer")) {
				VertexBufferDescriptor descriptor;
				reader.get(children[i], "id", &descriptor.id);
				reader.get(children[i], "size", &descriptor.size);
				reader.get(children[i], "dynamic", &descriptor.dynamic);
				reader.get(children[i], "layout", &descriptor.layout);
				createVertexBuffer(descriptor);
			}
			else if (reader.matches(children[i], "shader")) {
				ShaderDescriptor descriptor;
				reader.get(children[i], "id", &descriptor.id);
				descriptor.file = reader.get_string(children[i], "file");
				descriptor.vertexShader = reader.get_string(children[i], "vertex_shader");
				descriptor.pixelShader = reader.get_string(children[i], "pixel_shader");
				descriptor.model = reader.get_string(children[i], "shader_model");
				createShader(descriptor);
			}
			else if (reader.matches(children[i], "blendstate")) {
				// FIXME: assert that every entry is != -1
				BlendStateDescriptor descriptor;
				reader.get(children[i], "id", &descriptor.id);
				const char* entry = reader.get_string(children[i], "src_blend");
				descriptor.srcBlend = findBlendState(entry);
				entry = reader.get_string(children[i], "dest_blend");
				descriptor.destBlend = findBlendState(entry);
				entry = reader.get_string(children[i], "src_blend_alpha");
				descriptor.srcAlphaBlend = findBlendState(entry);
				entry = reader.get_string(children[i], "dest_blend_alpha");
				descriptor.destAlphaBlend = findBlendState(entry);
				reader.get(children[i], "alpha_enabled", &descriptor.alphaEnabled);
				createBlendState(descriptor);
			}
			else if (reader.matches(children[i], "texture")) {
				TextureDescriptor descriptor;
				reader.get(children[i], "id", &descriptor.id);
				descriptor.name = reader.get_string(children[i], "file");
				loadTexture(descriptor);
			}
			else if (reader.matches(children[i], "input_layout")) {
				InputLayoutDescriptor descriptor;
				descriptor.num = 0;
				reader.get(children[i], "id", &descriptor.id);
				const char* attributes = reader.get_string(children[i], "attributes");
				char buffer[256];
				sprintf_s(buffer, 256, "%s", attributes);
				char *token = token = strtok(buffer, ",");
				while (token) {
					int element = findInputElement(token);
					descriptor.indices[descriptor.num++] = element;
					token = strtok(NULL, ",");
				}
				reader.get(children[i], "shader", &descriptor.shader);
				createInputLayout(descriptor);
			}
			else if (reader.matches(children[i], "font")) {
				BitmapfontDescriptor descriptor;
				reader.get(children[i], "id", &descriptor.id);
				descriptor.name = reader.get_string(children[i], "file");
				loadFont(descriptor);
			}
			else if (reader.matches(children[i], "sprite_buffer")) {
				SpriteBufferDescriptor descriptor;
				reader.get(children[i], "id", &descriptor.id);
				reader.get(children[i], "size", &descriptor.size);
				reader.get(children[i], "index_buffer", &descriptor.indexBuffer);
				reader.get(children[i], "constant_buffer", &descriptor.constantBuffer);
				reader.get(children[i], "vertex_buffer", &descriptor.vertexBuffer);
				reader.get(children[i], "shader", &descriptor.shader);
				reader.get(children[i], "blend_state", &descriptor.blendstate);
				reader.get(children[i], "color_map", &descriptor.colormap);
				reader.get(children[i], "input_layout", &descriptor.inputlayout);
				if (reader.contains_property(children[i], "font")) {
					reader.get(children[i], "font", &descriptor.font);
				}
				else {
					descriptor.font = INVALID_RID;
				}
				createSpriteBuffer(descriptor);
			}
		}
	}

	// ------------------------------------------------------
	// cretate quad index buffer
	// ------------------------------------------------------
	RID ResourceContainer::createQuadIndexBuffer(const QuadIndexBufferDescriptor& descriptor) {
		int idx = _indexBuffers.size();
		ResourceIndex& ri = _resourceTable[descriptor.id];
		assert(ri.type == ResourceType::UNKNOWN);
		// FIXME: check that size % 6 == 0 !!!
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.ByteWidth = sizeof(uint32_t) * descriptor.size;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.MiscFlags = 0;

		uint32_t* data = new uint32_t[descriptor.size];
		int base = 0;
		int cnt = 0;
		int num = descriptor.size / 6;
		for (int i = 0; i < num; ++i) {
			data[base] = cnt;
			data[base + 1] = cnt + 1;
			data[base + 2] = cnt + 3;
			data[base + 3] = cnt + 1;
			data[base + 4] = cnt + 2;
			data[base + 5] = cnt + 3;
			base += 6;
			cnt += 4;
		}
		// Define the resource data.
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = data;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;
		// Create the buffer with the device.
		ID3D11Buffer* buffer;
		HRESULT hr = _device->CreateBuffer(&bufferDesc, &InitData, &buffer);
		if (FAILED(hr)) {
			delete[] data;
			DXTRACE_MSG("Failed to create quad index buffer!");
			return -1;
		}
		delete[] data;
		_indexBuffers.push_back(buffer);
		ri.index = idx;
		ri.id = descriptor.id;
		ri.type = ResourceType::INDEXBUFFER;
		return ri.id;
	}

	// ------------------------------------------------------
	// create index buffer
	// ------------------------------------------------------
	RID ResourceContainer::createIndexBuffer(const IndexBufferDescriptor& descriptor) {
		int idx = _indexBuffers.size();
		ResourceIndex& ri = _resourceTable[descriptor.id];
		assert(ri.type == ResourceType::UNKNOWN);
		D3D11_BUFFER_DESC bufferDesc;
		if (descriptor.dynamic) {
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		else {
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.CPUAccessFlags = 0;
		}
		bufferDesc.ByteWidth = sizeof(uint32_t) * descriptor.size;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.MiscFlags = 0;
		ID3D11Buffer* buffer;
		HRESULT hr = _device->CreateBuffer(&bufferDesc, 0, &buffer);
		if (FAILED(hr)) {
			DXTRACE_MSG("Failed to create index buffer!");
			return -1;
		}
		_indexBuffers.push_back(buffer);
		ri.index = idx;
		ri.id = descriptor.id;
		ri.type = ResourceType::INDEXBUFFER;
		return ri.id;
	}

	// ------------------------------------------------------
	// create constant buffer
	// ------------------------------------------------------
	RID ResourceContainer::createConstantBuffer(const ConstantBufferDescriptor& descriptor) {
		ResourceIndex& ri = _resourceTable[descriptor.id];
		assert(ri.type == ResourceType::UNKNOWN);
		int index = _constantBuffers.size();
		D3D11_BUFFER_DESC constDesc;
		ZeroMemory(&constDesc, sizeof(constDesc));
		constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constDesc.ByteWidth = descriptor.size;
		constDesc.Usage = D3D11_USAGE_DEFAULT;
		ID3D11Buffer* buffer = 0;
		HRESULT d3dResult = _device->CreateBuffer(&constDesc, 0, &buffer);
		if (FAILED(d3dResult))	{
			DXTRACE_MSG("Failed to create constant buffer!");
			return -1;
		}
		_constantBuffers.push_back(buffer);
		ri.index = index;
		ri.id = descriptor.id;
		ri.type = ResourceType::CONSTANTBUFFER;
		return ri.id;
	}

	// ------------------------------------------------------
	// create sprite buffer
	// ------------------------------------------------------
	RID ResourceContainer::createSpriteBuffer(const SpriteBufferDescriptor& descriptor) {
		ResourceIndex& ri = _resourceTable[descriptor.id];
		assert(ri.type == ResourceType::UNKNOWN);
		int index = _spriteBuffers.size();
		SpriteBuffer* buffer = new SpriteBuffer(descriptor);
		_spriteBuffers.push_back(buffer);
		ri.index = index;
		ri.id = descriptor.id;
		ri.type = ResourceType::SPRITEBUFFER;
		return ri.id;
	}

	// ------------------------------------------------------
	// load texture
	// ------------------------------------------------------
	RID ResourceContainer::loadTexture(const TextureDescriptor& descriptor) {
		ResourceIndex& ri = _resourceTable[descriptor.id];
		assert(ri.type == ResourceType::UNKNOWN);
		int idx = _shaderResourceViews.size();
		ID3D11ShaderResourceView* srv = 0;
		char buffer[256];
		sprintf_s(buffer, 256, "content\\textures\\%s", descriptor.name);
		HRESULT d3dResult = D3DX11CreateShaderResourceViewFromFile(_device, buffer, 0, 0, &srv, 0);
		if (FAILED(d3dResult)) {
			DXTRACE_MSG("Failed to load the texture image!");
			return INVALID_RID;
		}
		_shaderResourceViews.push_back(srv);
		ri.index = idx;
		ri.id = descriptor.id;
		ri.type = ResourceType::TEXTURE;
		return ri.id;
	}

	// ------------------------------------------------------
	// load bitmap font
	// ------------------------------------------------------
	RID ResourceContainer::loadFont(const BitmapfontDescriptor& descriptor) {
		ResourceIndex& ri = _resourceTable[descriptor.id];
		assert(ri.type == ResourceType::UNKNOWN);
		int idx = _fonts.size();
		
		char buffer[256];
		sprintf_s(buffer, 256, "content\\resources\\%s", descriptor.name);
		Bitmapfont* font = new Bitmapfont;


		JSONReader reader;
		bool ret = reader.parse(buffer);
		assert(ret);
		int num = reader.find_category("characters");
		char tmp[16];
		Rect rect;
		if (num != -1) {
			for (int i = 32; i < 255; ++i) {
				sprintf_s(tmp, 16, "C%d", i);
				if (reader.contains_property(num, tmp)) {
					reader.get(num, tmp, &rect);
					font->add(i, rect);
				}
			}
		}
		_fonts.push_back(font);
		ri.index = idx;
		ri.id = descriptor.id;
		ri.type = ResourceType::BITMAPFONT;
		return ri.id;
	}

	// ------------------------------------------------------
	// create blend state
	// ------------------------------------------------------
	RID ResourceContainer::createBlendState(const BlendStateDescriptor& descriptor) {
		ResourceIndex& ri = _resourceTable[descriptor.id];
		assert(ri.type == ResourceType::UNKNOWN);
		int idx = _blendStates.size();
		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(blendDesc));
		if (descriptor.alphaEnabled) {
			blendDesc.RenderTarget[0].BlendEnable = TRUE;
		}
		else {
			blendDesc.RenderTarget[0].BlendEnable = FALSE;
			//blendDesc.RenderTarget[0].BlendEnable = (srcBlend != D3D11_BLEND_ONE) || (destBlend != D3D11_BLEND_ZERO);
		}
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlend = BLEND_STATE_MAPPINGS[descriptor.srcBlend].blend;
		blendDesc.RenderTarget[0].DestBlend = BLEND_STATE_MAPPINGS[descriptor.destBlend].blend;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = BLEND_STATE_MAPPINGS[descriptor.srcAlphaBlend].blend;
		blendDesc.RenderTarget[0].DestBlendAlpha = BLEND_STATE_MAPPINGS[descriptor.destAlphaBlend].blend;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;

		ID3D11BlendState* state;
		HRESULT d3dResult = _device->CreateBlendState(&blendDesc, &state);
		if (FAILED(d3dResult)) {
			DXTRACE_MSG("Failed to create blendstate!");
			return INVALID_RID;
		}
		_blendStates.push_back(state);
		ri.index = idx;
		ri.id = descriptor.id;
		ri.type = ResourceType::BLENDSTATE;
		return ri.id;
	}

	// ------------------------------------------------------
	// create vertex buffer
	// ------------------------------------------------------
	RID ResourceContainer::createVertexBuffer(const VertexBufferDescriptor& descriptor) {
		ResourceIndex& ri = _resourceTable[descriptor.id];
		assert(ri.type == ResourceType::UNKNOWN);
		D3D11_BUFFER_DESC bufferDesciption;
		ZeroMemory(&bufferDesciption, sizeof(bufferDesciption));
		if (descriptor.dynamic) {
			bufferDesciption.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesciption.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		else {
			bufferDesciption.Usage = D3D11_USAGE_DEFAULT;
			bufferDesciption.CPUAccessFlags = 0;
		}
		bufferDesciption.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesciption.ByteWidth = descriptor.size;

		ID3D11Buffer* buffer = 0;
		HRESULT d3dResult = _device->CreateBuffer(&bufferDesciption, 0, &buffer);
		if (FAILED(d3dResult))	{
			DXTRACE_MSG("Failed to create buffer!");
			return -1;
		}
		int idx = _vertexBuffers.size();
		_vertexBuffers.push_back(buffer);
		ri.index = idx;
		ri.id = descriptor.id;
		ri.type = ResourceType::VERTEXBUFFER;
		return ri.id;
	}

	// ------------------------------------------------------
	// create vertex buffer
	// ------------------------------------------------------
	RID ResourceContainer::createInputLayout(const InputLayoutDescriptor& descriptor) {
		ResourceIndex& ri = _resourceTable[descriptor.id];
		assert(ri.type == ResourceType::UNKNOWN);
		int idx = _layouts.size();
		D3D11_INPUT_ELEMENT_DESC* descriptors = new D3D11_INPUT_ELEMENT_DESC[descriptor.num];
		uint32_t index = 0;
		uint32_t counter = 0;
		for (int i = 0; i < descriptor.num; ++i) {
			const InputElementDescriptor& d = INPUT_ELEMENT_DESCRIPTIONS[descriptor.indices[i]];
			D3D11_INPUT_ELEMENT_DESC& descriptor = descriptors[counter++];
			descriptor.SemanticName = d.semantic;
			descriptor.SemanticIndex = d.semanticIndex;
			descriptor.Format = d.format;
			descriptor.InputSlot = 0;
			descriptor.AlignedByteOffset = index;
			descriptor.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			descriptor.InstanceDataStepRate = 0;
			index += d.size;
		}
		ID3D11InputLayout* layout = 0;
		const ResourceIndex& res_idx = _resourceTable[descriptor.shader];
		assert(res_idx.type == ResourceType::SHADER);
		Shader* s = _shaders[res_idx.index];
		HRESULT d3dResult = _device->CreateInputLayout(descriptors, descriptor.num, s->vertexShaderBuffer->GetBufferPointer(), s->vertexShaderBuffer->GetBufferSize(), &layout);
		if (d3dResult < 0) {
			return INVALID_RID;
		}
		delete[] descriptors;
		_layouts.push_back(layout);
		ri.index = idx;
		ri.id = descriptor.id;
		ri.type = ResourceType::INPUTLAYOUT;
		return ri.id;
	}

	// ------------------------------------------------------
	// find blendstate by name
	// ------------------------------------------------------
	int ResourceContainer::findBlendState(const char* text) {
		for (int i = 0; i < 17; ++i) {
			if (strcmp(BLEND_STATE_MAPPINGS[i].name, text) == 0) {
				return i;
			}
		}
		return -1;
	}

	// ------------------------------------------------------
	// find inputelement by name
	// ------------------------------------------------------
	int ResourceContainer::findInputElement(const char* name) {
		for (int i = 0; i < 6; ++i) {
			if (strcmp(INPUT_ELEMENT_DESCRIPTIONS[i].semantic, name) == 0) {
				return i;
			}
		}
		return -1;
	}

	RID ResourceContainer::createShader(const ShaderDescriptor& descriptor) {
		ResourceIndex& ri = _resourceTable[descriptor.id];
		assert(ri.type == ResourceType::UNKNOWN);
		Shader* s = new Shader;
		int idx = _shaders.size();
		s->vertexShaderBuffer = 0;
		bool compileResult = compileShader(descriptor.file, descriptor.vertexShader, "vs_4_0", &s->vertexShaderBuffer);
		if (!compileResult)	{
			DXTRACE_MSG("Error compiling the vertex shader!");
			return -1;
		}
		HRESULT d3dResult;

		if (!createVertexShader(s->vertexShaderBuffer, &s->vertexShader)) {
			DXTRACE_MSG("Error creating the vertex shader!");
			return -1;
		}
		ID3DBlob* psBuffer = 0;
		compileResult = compileShader(descriptor.file, descriptor.pixelShader, "ps_4_0", &psBuffer);
		if (!compileResult)	{
			DXTRACE_MSG("Error compiling pixel shader!");
			return -1;
		}

		if (!createPixelShader(psBuffer, &s->pixelShader)) {
			DXTRACE_MSG("Error creating pixel shader!");
			return -1;
		}
		psBuffer->Release();
		D3D11_SAMPLER_DESC colorMapDesc;
		ZeroMemory(&colorMapDesc, sizeof(colorMapDesc));
		colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;

		d3dResult = _device->CreateSamplerState(&colorMapDesc, &s->samplerState);
		if (FAILED(d3dResult)) {
			DXTRACE_MSG("Failed to create SamplerState!");
			return -1;
		}
		_shaders.push_back(s);
		ri.index = idx;
		ri.id = descriptor.id;
		ri.type = ResourceType::SHADER;
		return ri.id;
	}

	bool ResourceContainer::createVertexShader(ID3DBlob* buffer, ID3D11VertexShader** shader) {
		HRESULT d3dResult = _device->CreateVertexShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), 0, shader);
		if (d3dResult < 0) {
			if (buffer) {
				buffer->Release();
			}
			return false;
		}
		return true;
	}

	bool ResourceContainer::createPixelShader(ID3DBlob* buffer, ID3D11PixelShader** shader) {
		HRESULT d3dResult = _device->CreatePixelShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), 0, shader);
		if (d3dResult < 0) {
			if (buffer) {
				buffer->Release();
			}
			return false;
		}
		return true;
	}

	bool ResourceContainer::compileShader(const char* filePath, const  char* entry, const  char* shaderModel, ID3DBlob** buffer) {
		DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined( DEBUG ) || defined( _DEBUG )
		shaderFlags |= D3DCOMPILE_DEBUG;
#endif

		ID3DBlob* errorBuffer = 0;
		HRESULT result;

		result = D3DX11CompileFromFile(filePath, 0, 0, entry, shaderModel, shaderFlags, 0, 0, buffer, &errorBuffer, 0);

		if (FAILED(result))	{
			if (errorBuffer != 0) {
				OutputDebugStringA((char*)errorBuffer->GetBufferPointer());
				errorBuffer->Release();
			}
			return false;
		}
		if (errorBuffer != 0) {
			errorBuffer->Release();
		}
		return true;
	}

	ID3D11Buffer* ResourceContainer::getIndexBuffer(RID rid) {
		const ResourceIndex& res_idx = _resourceTable[rid];
		assert(res_idx.type == ResourceType::INDEXBUFFER);
		return _indexBuffers[res_idx.index];
	}

	ID3D11BlendState* ResourceContainer::getBlendState(RID rid) {
		const ResourceIndex& res_idx = _resourceTable[rid];
		assert(res_idx.type == ResourceType::BLENDSTATE);
		return _blendStates[res_idx.index];
	}

	ID3D11Buffer* ResourceContainer::getConstantBuffer(RID rid) {
		const ResourceIndex& res_idx = _resourceTable[rid];
		assert(res_idx.type == ResourceType::CONSTANTBUFFER);
		return _constantBuffers[res_idx.index];
	}

	ID3D11Buffer* ResourceContainer::getVertexBuffer(RID rid) {
		const ResourceIndex& res_idx = _resourceTable[rid];
		assert(res_idx.type == ResourceType::VERTEXBUFFER);
		return _vertexBuffers[res_idx.index];
	}

	ID3D11InputLayout* ResourceContainer::getInputLayout(RID rid) {
		const ResourceIndex& res_idx = _resourceTable[rid];
		assert(res_idx.type == ResourceType::INPUTLAYOUT);
		return _layouts[res_idx.index];
	}

	ID3D11ShaderResourceView* ResourceContainer::getShaderResourceView(RID rid) {
		const ResourceIndex& res_idx = _resourceTable[rid];
		assert(res_idx.type == ResourceType::TEXTURE);
		return _shaderResourceViews[res_idx.index];
	}

	Shader* ResourceContainer::getShader(RID rid) {
		const ResourceIndex& res_idx = _resourceTable[rid];
		assert(res_idx.type == ResourceType::SHADER);
		return _shaders[res_idx.index];
	}

	Bitmapfont* ResourceContainer::getFont(RID rid) {
		const ResourceIndex& res_idx = _resourceTable[rid];
		assert(res_idx.type == ResourceType::BITMAPFONT);
		return _fonts[res_idx.index];
	}

	SpriteBuffer* ResourceContainer::getSpriteBuffer(RID rid) {
		const ResourceIndex& res_idx = _resourceTable[rid];
		assert(res_idx.type == ResourceType::SPRITEBUFFER);
		return _spriteBuffers[res_idx.index];
	}
}