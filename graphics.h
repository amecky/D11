#pragma once
#include <stdint.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>
#include "base\Settings.h"
#include "math\math_types.h"
#include "resources\ResourceDescriptors.h"
#include "Common.h"
#include "renderer\Bitmapfont.h"

struct Access{

	enum ENUM {
		Read,
		Write,
		ReadWrite,
		Count
	};
};


namespace ds {

	class ResourceContainer;
}

namespace graphics {
	
	bool initialize(HINSTANCE hInstance, HWND hwnd, const ds::Settings& settings);

	void setResourceContainer(ds::ResourceContainer* container);

	ID3D11DeviceContext* getContext();

	ID3D11Device* getDevice();

	void updateConstantBuffer(RID rid, void* data);

	ds::Bitmapfont* getFont(RID rid);

	bool createSamplerState(ID3D11SamplerState** sampler);

	const ds::mat4& getViewProjectionMaxtrix();

	void beginRendering(const Color& color);

	void setIndexBuffer(RID rid);

	void setVertexBuffer(RID rid, uint32_t* stride, uint32_t* offset);

	void mapData(RID rid, void* data, uint32_t size);

	void setShader(RID rid);

	void setInputLayout(RID rid);

	void setPixelShaderResourceView(RID rid, uint32_t slot = 0);

	void setVertexShaderConstantBuffer(RID rid);

	void setBlendState(RID rid);

	void drawIndexed(uint32_t num);

	void endRendering();

	void shutdown();
}

