//FACE D3D11 Render
namespace RenderToolKit
{
	//Render Setting
	#define MaxVertexIndex 65000
	#define FontTextureWidth 256.f
	#define FontTextureHeight 256.f
	#define FontTextureMultipler 2.f
	#define FontBytesName FontTexture1
	#define FontDescBytesName FontDesc1

	//Struct & Data
	#include "RenderData.h"
	struct Vertex {
		vec2_t CurPos;
		vec2_t TexPos;
		DWORD CurColor;
	}; 
	struct DrawCmd {
		int IndexCount;
		int VertexCount;
	};
	struct DrawCmdOut {
		USHORT* IBuffer;
		Vertex* VBuffer;
	};
	struct ConstStruct {
		vec2_t ScreenSize;
		vec2_t TextureSize;
	};
	struct DrawListStruct {
		DrawCmd Cmds[MaxVertexIndex];
		USHORT* IndexBuffer; Vertex* VertexBuffer;
		int VertexCount, DrawCmdsCount, IndexCount;
	}
	*DrawList = (DrawListStruct*)&RenderBuff[0];
	
	//D3DX11 Context
	ID3D11Device* D3D_Device;
	ID3D11DeviceContext* D3D_Context;
	ID3D11BlendState* TransparencyState;
	ID3D11RenderTargetView* RenderTarget;
	ID3D11ShaderResourceView* FontResource;
	ID3D11RasterizerState* RasterizerState;

	//Shaders Context
	ID3D11Buffer* VBuffer;
	ID3D11Buffer* IBuffer;
	ID3D11Buffer* ConstBuffer;
	ID3D11InputLayout* SLayout;
	ID3D11PixelShader* PShader;
	ID3D11VertexShader* VShader;

	//Fast Math
	__forceinline float CosAdd(float x) {
		float x2 = x * x;
		const float c1 = 0.99940307f;
		const float c2 = -0.49558072f;
		const float c3 = 0.03679168f;
		return (c1 + x2 * (c2 + c3 * x2));
	}

	__forceinline float FastSqrt(float x) {
		union { int i; float x; } u; u.x = x;
		u.i = (1 << 29) + (u.i >> 1) - (1 << 22);
		return u.x;
	}

	__forceinline float FastCos(float angle) {
		angle = angle - floorf(angle * 0.15f) * 6.28f;
		angle = angle > 0.f ? angle : -angle;
		if (angle < 1.57f) return CosAdd(angle);
		if (angle < 3.14f) return -CosAdd(3.14f - angle);
		if (angle < 4.71f) return -CosAdd(angle - 3.14f);
		return CosAdd(6.28f - angle);
	}

	__forceinline float FastSin(float angle) {
		return FastCos(1.57f - angle);
	}

	//Utils
	template <typename T>
	__forceinline void ZeroSet(T DataPtr, int Size = 0) {
		if (Size) __stosb((PBYTE)DataPtr, 0, Size);
		else __stosb((PBYTE)DataPtr, 0, sizeof(*DataPtr));
	}

	template <typename T>
	__forceinline T DeEncPtr(IDXGISwapChain* SwapChain, T Ptr) {
		return T((DWORD64)Ptr ^ (DWORD64)SwapChain);
	}

	__forceinline void SafeDecrShader(PUCHAR BuffIn, PUCHAR BuffOut, int Size) {
		for (int i = 0; i < Size; ++i)
			BuffOut[i] = (UCHAR)(BuffIn[i] ^ ((i + 57 * i + 71) + 24 + i));
		ZeroSet(BuffIn, Size);
	}
	
	//D3D11 State Saver
	class D3DState
	{
	private:
		//basic states
		D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology;
		ID3D11DepthStencilView* DepthStencilView;
		ID3D11RenderTargetView* RTargetViews[8];
		ID3D11RasterizerState* RasterizerState;
		float BlendFactor[4]; UINT SampleMask;
		ID3D11BlendState* TransparencyState;
		ID3D11InputLayout* InputLayout;

		//vertex shader state
		DXGI_FORMAT IndexFormat;
		ID3D11Buffer* IndexBuffer;
		ID3D11Buffer* VertexBuffer;
		ID3D11VertexShader* VertexShader;
		ID3D11Buffer* VertexShaderConstantBuffer;
		UINT VertexOffset, IndexOffset, VertexStride;
		UINT VS_Num; ID3D11ClassInstance* VInstances[256];

		//pixel shader state
		ID3D11SamplerState* SamplerState;
		ID3D11ClassInstance* PInstances[256];
		ID3D11PixelShader* PixelShader; UINT PS_Num;
		ID3D11ShaderResourceView* PixelShaderResView;

		//geometry shader state
		ID3D11GeometryShader* GeometryShader;
		UINT GS_Num; ID3D11ClassInstance* GInstances[256];

	public:
		void SaveState(IDXGISwapChain* SwapChain, ID3D11DeviceContext* Context)
		{
			//alloc vars in stack
			ID3D11BlendState* TState;
			ID3D11InputLayout* ILayout;
			ID3D11RasterizerState* RState;
			ID3D11RenderTargetView* RTViews[8];
			ID3D11DepthStencilView* DStencilView;

			//get basic states
			VCall(Context, 94, &RState);
			VCall(Context, 78, &ILayout);
			VCall(Context, 83, &PrimitiveTopology);
			VCall(Context, 89, 8, RTViews, &DStencilView);
			VCall(Context, 91, &TState, BlendFactor, &SampleMask);

			//crypt basic states
			InputLayout = DeEncPtr(SwapChain, ILayout);
			RasterizerState = DeEncPtr(SwapChain, RState);
			TransparencyState = DeEncPtr(SwapChain, TState);
			DepthStencilView = DeEncPtr(SwapChain, DStencilView);
			for (int i = 0; i < 8; ++i) RTargetViews[i] =
				DeEncPtr(SwapChain, RTViews[i]);

			VS_Num = 256; //vertex
			ID3D11VertexShader* VShader;
			ID3D11ClassInstance* SInstances[256];
			ID3D11Buffer* VSCBuffer, *VBuffer, *IBuffer;
			VCall(Context, 76, &VShader, SInstances, &VS_Num);
			VCall(Context, 80, &IBuffer, &IndexFormat, &IndexOffset);
			VCall(Context, 79, 0, 1, &VBuffer, &VertexStride, &VertexOffset);
			VCall(Context, 72, 0, 1, &VSCBuffer);

			//crypt vertex states
			IndexBuffer = DeEncPtr(SwapChain, IBuffer);
			VertexBuffer = DeEncPtr(SwapChain, VBuffer);
			VertexShader = DeEncPtr(SwapChain, VShader);
			VertexShaderConstantBuffer = DeEncPtr(SwapChain, VSCBuffer);
			for (int i = 0; i < VS_Num; ++i) VInstances[i] =
				DeEncPtr(SwapChain, SInstances[i]);

			PS_Num = 256; //pixel
			ID3D11SamplerState* SState;
			ID3D11PixelShader* PShader;
			ID3D11ShaderResourceView* PShaderResView;
			VCall(Context, 74, &PShader, SInstances, &PS_Num);
			VCall(Context, 73, 0, 1, &PShaderResView);
			VCall(Context, 75, 0, 1, &SState);

			//crypt pixel states
			SamplerState = DeEncPtr(SwapChain, SState);
			PixelShader = DeEncPtr(SwapChain, PShader);
			PixelShaderResView = DeEncPtr(SwapChain, PShaderResView);
			for (int i = 0; i < PS_Num; ++i) PInstances[i] =
				DeEncPtr(SwapChain, SInstances[i]);

			GS_Num = 256; //geometry
			ID3D11GeometryShader* GShader;
			VCall(Context, 82, &GShader, SInstances, &GS_Num);

			//crypt geometry states
			GeometryShader = DeEncPtr(SwapChain, GShader);
			for (int i = 0; i < GS_Num; ++i) GInstances[i] =
				DeEncPtr(SwapChain, SInstances[i]);
		}

		void LoadState(IDXGISwapChain* SwapChain, ID3D11DeviceContext* Context)
		{
			//decrypt render targets
			ID3D11RenderTargetView* RTViews[8];
			for (int i = 0; i < 8; ++i) RTViews[i] = 
				DeEncPtr(SwapChain, RTargetViews[i]);

			//set basic states
			VCall(Context, 24, PrimitiveTopology);
			VCall(Context, 17, DeEncPtr(SwapChain, InputLayout));
			VCall(Context, 43, DeEncPtr(SwapChain, RasterizerState));
			VCall(Context, 33, 8, RTViews, DeEncPtr(SwapChain, DepthStencilView));
			VCall(Context, 35, DeEncPtr(SwapChain, TransparencyState), BlendFactor,
				SampleMask);

			//decrypt vertex instances
			ID3D11ClassInstance* SInstances[256];
			for (int i = 0; i < GS_Num; ++i)
				SInstances[i] = DeEncPtr(
					SwapChain, VInstances[i]);

			//set vertex shader data
			ID3D11Buffer* IBuffer = DeEncPtr(SwapChain, IndexBuffer);
			ID3D11Buffer* VBuffer = DeEncPtr(SwapChain, VertexBuffer);
			ID3D11VertexShader* VShader = DeEncPtr(SwapChain, VertexShader);
			ID3D11Buffer* VSConstantBuffer = DeEncPtr(SwapChain, VertexShaderConstantBuffer);
			VCall(Context, 18, 0, 1, &VBuffer, &VertexStride, &VertexOffset);
			VCall(Context, 19, IBuffer, IndexFormat, IndexOffset);
			VCall(Context, 11, VShader, SInstances, VS_Num);
			VCall(Context, 7, 0, 1, &VSConstantBuffer);

			//decrypt pixel instances
			for (int i = 0; i < PS_Num; ++i)
				SInstances[i] = DeEncPtr(
					SwapChain, PInstances[i]);

			//set pixel shader data
			ID3D11ShaderResourceView* PSResView = DeEncPtr(SwapChain, PixelShaderResView);
			ID3D11SamplerState* SState = DeEncPtr(SwapChain, SamplerState);
			VCall(Context, 9, DeEncPtr(SwapChain, PixelShader), SInstances, PS_Num);
			VCall(Context, 8, 0, 1, &PSResView); VCall(Context, 10, 0, 1, &SState);

			//set geometry shader data
			for (int i = 0; i < GS_Num; ++i) SInstances[i] =
				DeEncPtr(SwapChain, GInstances[i]);
			VCall(Context, 23, DeEncPtr(SwapChain,
				GeometryShader), SInstances, GS_Num);
		}
	} D3DState;

	//D3D11 Text Engine
	__forceinline bool GetCharDesc(wchar_t Char, CharDesc* Desc)
	{
		int CharID = (int)Char;
		CharDesc* FDBuff = (CharDesc*)
			&FontDescBytesName;

		if ((CharID >= 32) && (CharID <= 126)) {
			*Desc = FDBuff[0 + (CharID - 32)];
			return true;
		}

		if ((CharID >= 1025) && (CharID <= 1025)) {
			*Desc = FDBuff[95 + (CharID - 1025)];
			return true;
		}

		if ((CharID >= 1040) && (CharID <= 1103)) {
			*Desc = FDBuff[96 + (CharID - 1040)];
			return true;
		}

		if ((CharID >= 1105) && (CharID <= 1105)) {
			*Desc = FDBuff[160 + (CharID - 1105)];
			return true;
		}

		return false;
	}

	__forceinline bool InitFontTexture(IDXGISwapChain* SwapChain, ID3D11Device* Device)
	{
		//create texture describe
		ID3D11Texture2D* FontTexture = nullptr;
		D3D11_TEXTURE2D_DESC Desc; ZeroSet(&Desc);
		Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		struct CByte { BYTE Series; BYTE DataByte; };
		Desc.MipLevels = Desc.ArraySize = Desc.SampleDesc.Count = 1; 
		Desc.Width = (UINT)FontTextureWidth; Desc.Height = (UINT)FontTextureHeight;
		D3D11_SUBRESOURCE_DATA TextureResource = { &RenderBuff[0], Desc.Width * 4, 0 };
		
		//decompress font bytes
		CByte* CCByte = (CByte*)&FontBytesName[0];
		int CCBytesSize = (sizeof(FontBytesName) / sizeof(CByte));
		for (int Pos = 0, Off = 0; Pos < CCBytesSize; ++Pos, ++CCByte) {
			__stosb(&RenderBuff[Off], CCByte->DataByte, CCByte->Series);
			Off += CCByte->Series;
		}

		//create font texture & cleanup buffer
		bool Stat = FAILED(VCall<HRESULT>(Device, 5, &Desc, &TextureResource, &FontTexture));
		ZeroSet(RenderBuff, sizeof(RenderBuff)); if (Stat) return false;

		//create font resource & cleanup
		D3D11_SHADER_RESOURCE_VIEW_DESC ResourceDesc; ID3D11ShaderResourceView* FTexture;
		ZeroSet(&ResourceDesc); ResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		ResourceDesc.Texture2D.MipLevels = Desc.MipLevels; ResourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		Stat = SUCCEEDED(VCall<HRESULT>(Device, 7, FontTexture, &ResourceDesc, &FTexture));
		VCall(FontTexture, 2); FontResource = DeEncPtr(SwapChain, FTexture); return Stat;
	}

	vec2_t GetTextSize(const wchar_t* String, RenderToolKit::CharDesc* DescArray, int* DescSize)
	{
		wchar_t WChar;
		int DescArrayOffset = 0;
		vec2_t TextSize = { 0.f, 0.f };
		for (int i = 0; (WChar = String[i]); ++i)
		{
			//get char description
			if (GetCharDesc(WChar, &DescArray[DescArrayOffset]))
			{
				//get char size
				CharDesc CDesc = DescArray[DescArrayOffset++];
				float YSize = float(CDesc.yOff + CDesc.srcH) / FontTextureMultipler;
				TextSize.x += ((float)CDesc.xAdv / FontTextureMultipler);
				TextSize.y = max(YSize, TextSize.y);
			}
		}

		//save size & ret text size
		*DescSize = DescArrayOffset;
		return TextSize;
	}

	//D3D11 Render
	bool InitRender(IDXGISwapChain* SwapChain)
	{
		//VIRTUALIZER_MUTATE_ONLY_START

		//get d3d device
		ID3D11Device* D3Device;
		if (FAILED(VCall<HRESULT>(SwapChain, 7, IID_PPV_ARGS(&D3Device)))) return false;
		VCall(D3Device, 2);
		D3D_Device = DeEncPtr(SwapChain, D3Device);

		//get d3d context
		ID3D11DeviceContext* D3Context = nullptr;
		VCall(D3Device, 40, &D3Context);
		if (!D3Context)
			return false;
		VCall(D3Context, 2);
		D3D_Context = DeEncPtr(SwapChain, D3Context);

		//init text engine
		if (!InitFontTexture(SwapChain, D3Device))
			return false;

		//create pixel shader
		ID3D11PixelShader* PShaderDecr;
		UCHAR ShaderData[sizeof(VertexShader)]; SafeDecrShader(PixelShader, ShaderData, sizeof(PixelShader));
		if (SUCCEEDED(VCall<HRESULT>(D3Device, 15, ShaderData, sizeof(PixelShader), nullptr, &PShaderDecr))) {
			PShader = DeEncPtr(SwapChain, PShaderDecr);
		} else return false;
	
		//create vertex shader
		ID3D11VertexShader* VShaderDecr; SafeDecrShader(VertexShader, ShaderData, sizeof(VertexShader));
		if (FAILED(VCall<HRESULT>(D3Device, 12, ShaderData, sizeof(VertexShader), nullptr, &VShaderDecr))) {
			VCall(PShaderDecr, 2); return false;
		} VShader = DeEncPtr(SwapChain, VShaderDecr);

		//create rasterizer state
		ID3D11RasterizerState* RState;
		D3D11_RASTERIZER_DESC RDesc; ZeroSet(&RDesc);
		RDesc.CullMode = D3D11_CULL_NONE; RDesc.FillMode = D3D11_FILL_SOLID;
		if (FAILED(VCall<HRESULT>(D3Device, 22, &RDesc, &RState))) {
			VCall(PShaderDecr, 2); VCall(VShaderDecr, 2); return false;
		} RasterizerState = DeEncPtr(SwapChain, RState);

		//create blend state
		ID3D11BlendState* TState;
		D3D11_BLEND_DESC BDesc; ZeroSet(&BDesc);
		BDesc.RenderTarget[0].BlendEnable = true;
		BDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		BDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		BDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		BDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		BDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		BDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		BDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		if (FAILED(VCall<HRESULT>(D3Device, 20, &BDesc, &TState))) {
			VCall(PShaderDecr, 2); VCall(VShaderDecr, 2);
			VCall(RState, 2); return false;
		} TransparencyState = DeEncPtr(SwapChain, TState);

		//create vertex buffer
		ID3D11Buffer* VBufferDecr;
		D3D11_BUFFER_DESC VBufferDesc; 
		ZeroSet(&VBufferDesc);
		VBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		VBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		VBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		VBufferDesc.ByteWidth = MaxVertexIndex * sizeof(Vertex);
		if (FAILED(VCall<HRESULT>(D3Device, 3, &VBufferDesc, nullptr, &VBufferDecr))) {
			VCall(PShaderDecr, 2); VCall(VShaderDecr, 2); VCall(RState, 2);
			VCall(TState, 2); return false;
		} VBuffer = DeEncPtr(SwapChain, VBufferDecr);

		//create index buffer
		ID3D11Buffer* IBufferDecr;
		VBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		VBufferDesc.ByteWidth = MaxVertexIndex * sizeof(USHORT);
		if (FAILED(VCall<HRESULT>(D3Device, 3, &VBufferDesc, nullptr, &IBufferDecr))) {
			VCall(PShaderDecr, 2); VCall(VShaderDecr, 2); VCall(RState, 2);
			VCall(TState, 2); VCall(VBufferDecr, 2); return false;
		} IBuffer = DeEncPtr(SwapChain, IBufferDecr);

		//create const buffer
		ID3D11Buffer* CBufferDecr;
		VBufferDesc.ByteWidth = 16; VBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		if (FAILED(VCall<HRESULT>(D3Device, 3, &VBufferDesc, nullptr, &CBufferDecr))) {
			VCall(PShaderDecr, 2); VCall(VShaderDecr, 2); VCall(RState, 2); VCall(TState, 2);
			VCall(VBufferDecr, 2); VCall(IBufferDecr, 2); return false;
		} ConstBuffer = DeEncPtr(SwapChain, CBufferDecr);

		//create vertex layout template
		char ColorArray[] = { 'C', 'O', 'L', 'O', 'R', 0 };
		char PosArray[] = { 'P', 'O', 'S', 'I', 'T', 'I', 'O', 'N', 0 };
		char TexPosArray[] = { 'T', 'E', 'X', 'C', 'O', 'O', 'R', 'D', 0 };

		D3D11_INPUT_ELEMENT_DESC ShaderLayout[] = {
			{ PosArray,    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ TexPosArray, 0, DXGI_FORMAT_R32G32_FLOAT,    0, 8,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ ColorArray,  0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		//create shader layout
		ID3D11InputLayout* SLayoutDecr;
		if (FAILED(VCall<HRESULT>(D3Device, 11, ShaderLayout, 3, ShaderData, sizeof(VertexShader), &SLayoutDecr))) {
			VCall(PShaderDecr, 2); VCall(VShaderDecr, 2); VCall(RState, 2); VCall(TState, 2);
			VCall(VBufferDecr, 2); VCall(IBufferDecr, 2); VCall(CBufferDecr, 2); return false;
		} SLayout = DeEncPtr(SwapChain, SLayoutDecr);

		//get backbuffer
		ID3D11Texture2D* D3D_BackBuffer;
		bool Stat = SUCCEEDED(VCall<HRESULT>(SwapChain, 9, 0, IID_PPV_ARGS(&D3D_BackBuffer)));
		if (Stat) VCall(D3D_BackBuffer, 2);

		//VIRTUALIZER_MUTATE_ONLY_END

		//create render target
		ID3D11RenderTargetView* RenderTargetDecr;
		if (!Stat || FAILED(VCall<HRESULT>(D3Device, 9, D3D_BackBuffer, nullptr, &RenderTargetDecr))) {
			VCall(PShaderDecr, 2); VCall(VShaderDecr, 2); VCall(RState, 2);
			VCall(TState, 2); VCall(VBufferDecr, 2); VCall(IBufferDecr, 2);
			VCall(CBufferDecr, 2); VCall(SLayoutDecr, 2); return false;
		} RenderTarget = DeEncPtr(SwapChain, RenderTargetDecr);
		return true;
	}

	bool NewFrame(IDXGISwapChain* SwapChain, vec2_t& ScreenSize)
	{
		//init d3d11
		if (!SwapChain) return false; static bool Once = false;
		if (!Once) { if (!InitRender(SwapChain)) return false; Once = true; }

		//get screen size & backup d3d old state
		UINT Number = 1; D3D11_VIEWPORT ViewPort;
		ID3D11DeviceContext* Ctx = DeEncPtr(SwapChain, D3D_Context);
		VCall(Ctx, 95, &Number, &ViewPort); D3DState.SaveState(SwapChain, Ctx);
		
		//map index buffer
		ID3D11Buffer* IBuffDecr = DeEncPtr(SwapChain, IBuffer); D3D11_MAPPED_SUBRESOURCE MappedIndexBuff;
		if (FAILED(VCall<HRESULT>(Ctx, 14, IBuffDecr, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedIndexBuff)))
			return false;

		//map vertex buffer
		ID3D11Buffer* VBuffDecr = DeEncPtr(SwapChain, VBuffer);
		ID3D11Buffer* ConstBuffDecr = DeEncPtr(SwapChain, ConstBuffer);
		D3D11_MAPPED_SUBRESOURCE MappedVertexBuff; D3D11_MAPPED_SUBRESOURCE MappedConstBuff;
		if (FAILED(VCall<HRESULT>(Ctx, 14, VBuffDecr, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedVertexBuff))) {
			VCall(Ctx, 15, IBuffDecr, 0); return false;
		}

		//map const vertex buffer
		if (FAILED(VCall<HRESULT>(Ctx, 14, ConstBuffDecr, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedConstBuff))) {
			VCall(Ctx, 15, VBuffDecr, 0); VCall(Ctx, 15, IBuffDecr, 0); return false;
		}

		//setup buffers
		ScreenSize = { ViewPort.Width, ViewPort.Height };
		DrawList->IndexBuffer = (USHORT*)MappedIndexBuff.pData;
		DrawList->VertexBuffer = (Vertex*)MappedVertexBuff.pData;
		ConstStruct* ConstBuff = (ConstStruct*)MappedConstBuff.pData;
		ConstBuff->TextureSize = { FontTextureWidth, FontTextureHeight };
		ConstBuff->ScreenSize = ScreenSize; VCall<HRESULT>(Ctx, 15, ConstBuffDecr, 0);
		
		//setup shaders buffers
		UINT Stride = sizeof(Vertex);
		auto IndexSize = sizeof(*DrawCmdOut::IBuffer);
		DXGI_FORMAT IFormat = ((IndexSize == 2) ?
			DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT);
		UINT Offset = 0; VCall(Ctx, 19, IBuffDecr, IFormat, 0);
		ID3D11InputLayout* SLayOutDecr = DeEncPtr(SwapChain, SLayout);
		VCall(Ctx, 17, SLayOutDecr); VCall(Ctx, 7, 0, 1, &ConstBuffDecr);
		ID3D11ShaderResourceView* FTexture = DeEncPtr(SwapChain, FontResource);
		VCall(Ctx, 8, 0, 1, &FTexture); VCall(Ctx, 18, 0, 1, &VBuffDecr, &Stride, &Offset);

		//setup shaders
		VCall(Ctx, 23, nullptr, nullptr, 0);
		VCall(Ctx, 9, DeEncPtr(SwapChain, PShader), nullptr, 0);
		VCall(Ctx, 11, DeEncPtr(SwapChain, VShader), nullptr, 0);

		//decrypt canvas states & flush cmd buffers
		ID3D11BlendState* TState = DeEncPtr(SwapChain, TransparencyState);
		ID3D11RenderTargetView* RTarget = DeEncPtr(SwapChain, RenderTarget);
		ID3D11RasterizerState* RState = DeEncPtr(SwapChain, RasterizerState);
		DrawList->DrawCmdsCount = DrawList->IndexCount = DrawList->VertexCount = 0;

		//set canvas states
		float BlendFactor[4] = { 0.f , 0.f , 0.f , 0.f };
		VCall(Ctx, 35, TState, BlendFactor, 0xffffffff);
		VCall(Ctx, 33, 1, &RTarget, nullptr);
		VCall(Ctx, 43, RState); return true;
	}

	__forceinline DrawCmdOut AddDrawCmd(int NumVertex, int NumIndex)
	{
		//lock add cmd
		static bool Lock;
		while (Lock) {};
		Lock = true;

		//get buffs cur pos
		DrawCmdOut OutData = {
			&DrawList->IndexBuffer[DrawList->IndexCount],
			&DrawList->VertexBuffer[DrawList->VertexCount]
		};

		//add entry to draw list
		DrawList->IndexCount += NumIndex;
		DrawList->VertexCount += NumVertex;
		int CurCmd = DrawList->DrawCmdsCount++;
		DrawList->Cmds[CurCmd].IndexCount = NumIndex;
		DrawList->Cmds[CurCmd].VertexCount = NumVertex;
		Lock = false; return OutData;
	}

	void EndFrame(IDXGISwapChain* SwapChain)
	{
		//unmap vertex & index buffers
		ID3D11DeviceContext* Ctx = DeEncPtr(SwapChain, D3D_Context);
		VCall(Ctx, 15, DeEncPtr(SwapChain, VBuffer), 0);
		VCall(Ctx, 15, DeEncPtr(SwapChain, IBuffer), 0);

		//process draw list
		int VertexOffset = 0, IndexOffset = 0;
		VCall(Ctx, 24, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		for (int i = 0; i < DrawList->DrawCmdsCount; ++i) {
			auto Cmd = &DrawList->Cmds[i];
			VCall(Ctx, 12, Cmd->IndexCount, IndexOffset, VertexOffset);
			IndexOffset += Cmd->IndexCount; VertexOffset += Cmd->VertexCount;
		}

		//restore old d3d state
		D3DState.LoadState(SwapChain, Ctx);
	}

	//D3D11 Polygon Generator (Software Antialiased)
	__declspec(noinline) void AddPolyFilledAA(const vec2_t* Points, int PointsCount, DWORD Color)
	{
		//basic check's (max 90 points) & set temp vars
		if (PointsCount < 3 || PointsCount > 90) return;
		int idx_count = (PointsCount - 2) * 3 + PointsCount * 6;
		DWORD col_trans = Color & ~0xFF000000;
		int vtx_count = (PointsCount * 2);

		//add draw cmd
		auto CmdBuff = AddDrawCmd(vtx_count, idx_count);
		auto _IdxWritePtr = CmdBuff.IBuffer;
		auto _VtxWritePtr = CmdBuff.VBuffer;

		//add indexes for fill
		for (int i = 2; i < PointsCount; i++) {
			_IdxWritePtr[1] = ((i - 1) << 1);
			_IdxWritePtr[2] = (i << 1);
			_IdxWritePtr[0] = 0;
			_IdxWritePtr += 3;
		}

		//calculate normals
		vec2_t* temp_normals = (vec2_t*)_alloca(720);
		for (int i0 = PointsCount - 1, i1 = 0; i1 < PointsCount; i0 = i1++) {
			vec2_t diff = Points[i1] - Points[i0];
			float len = (diff.x * diff.x) + (diff.y * diff.y);
			if (len != 0.f) { len = 1.f / FastSqrt(len); diff *= len; }
			temp_normals[i0].x = diff.y; temp_normals[i0].y = -diff.x;
		}

		for (int i0 = PointsCount - 1, i1 = 0; i1 < PointsCount; i0 = i1++)
		{
			//average normals
			vec2_t dm = (temp_normals[i0] + temp_normals[i1]) * 0.5f;
			float dmr2 = dm.x * dm.x + dm.y * dm.y;
			if (dmr2 > 0.000001f) {
				float scale = 1.f / dmr2;
				dm *= min(scale, 100.f);
			} dm *= 0.5f;

			//add vertexes
			_VtxWritePtr[1] = { Points[i1] + dm, { -1.f, 0.f }, col_trans };
			_VtxWritePtr[0] = { Points[i1] - dm, { -1.f, 0.f }, Color };
			_VtxWritePtr += 2;

			//add indexes
			_IdxWritePtr[2] = (1 + (i0 << 1)); _IdxWritePtr[5] = (i1 << 1);
			_IdxWritePtr[4] = (1 + (i1 << 1)); _IdxWritePtr[0] = (i1 << 1);
			_IdxWritePtr[1] = (i0 << 1); _IdxWritePtr[3] = (1 + (i0 << 1));
			_IdxWritePtr += 6;
		}
	}

	__declspec(noinline) void AddPolyLineAA(const vec2_t* Points, int PointsCount, DWORD color, bool Closed, float Thick)
	{
		//basic check's (max 90 points)
		if (PointsCount < 2 || PointsCount > 90)
			return; int count = PointsCount;
		if (!Closed) count = PointsCount - 1;
		
		//set temp vars
		bool thick_line = Thick > 1.f;
		DWORD col_trans = color & ~0xFF000000;
		int idx_count = thick_line ? count * 18 : count * 12;
		int vtx_count = thick_line ? PointsCount * 4 : PointsCount * 3;
	
		//add draw cmd
		auto CmdBuff = AddDrawCmd(vtx_count, idx_count);
		auto _IdxWritePtr = CmdBuff.IBuffer;
		auto _VtxWritePtr = CmdBuff.VBuffer;

		//alloc temporary buffer (in stack)
		vec2_t* temp_normals = (vec2_t*)_alloca(3600);
		vec2_t* temp_points = temp_normals + PointsCount;

		//calculate normals
		for (int i1 = 0; i1 < count; ++i1) {
			int i2 = (i1 + 1) == PointsCount ? 0 : i1 + 1;
			vec2_t diff = Points[i2] - Points[i1];
			float len = (diff.x * diff.x) + (diff.y * diff.y);
			if (len != 0.f) { len = 1.f / FastSqrt(len); diff *= len; }
			temp_normals[i1].x = diff.y; temp_normals[i1].y = -diff.x;
		}

		//fix closed
		if (!Closed)
			temp_normals[PointsCount - 1] = temp_normals[PointsCount - 2];

		//simple line
		if (!thick_line)
		{
			//fix closed
			if (!Closed) {
				temp_points[0] = Points[0] + temp_normals[0]; temp_points[1] = Points[0] - temp_normals[0];
				temp_points[(PointsCount - 1) * 2 + 0] = Points[PointsCount - 1] + temp_normals[PointsCount - 1];
				temp_points[(PointsCount - 1) * 2 + 1] = Points[PointsCount - 1] - temp_normals[PointsCount - 1];
			}

			unsigned int idx1 = 0;
			for (int i1 = 0; i1 < count; ++i1)
			{
				//average normals
				int i2 = (i1 + 1) == PointsCount ? 0 : i1 + 1;
				unsigned int idx2 = (i1 + 1) == PointsCount ? 0 : idx1 + 3;
				vec2_t dm = (temp_normals[i1] + temp_normals[i2]) * 0.5f;
				float dmr2 = dm.x * dm.x + dm.y * dm.y;
				if (dmr2 > 0.000001f) {
					float scale = 1.f / dmr2;
					dm *= min(scale, 100.f);
				}

				//add temp vertexes
				temp_points[i2 * 2 + 0] = Points[i2] + dm;
				temp_points[i2 * 2 + 1] = Points[i2] - dm;

				//add indexes
				_IdxWritePtr[0] = (idx2 + 0); _IdxWritePtr[1] =  (idx1 + 0); _IdxWritePtr[2] =  (idx1 + 2);
				_IdxWritePtr[3] = (idx1 + 2); _IdxWritePtr[4] =  (idx2 + 2); _IdxWritePtr[5] =  (idx2 + 0);
				_IdxWritePtr[6] = (idx2 + 1); _IdxWritePtr[7] =  (idx1 + 1); _IdxWritePtr[8] =  (idx1 + 0);
				_IdxWritePtr[9] = (idx1 + 0); _IdxWritePtr[10] = (idx2 + 0); _IdxWritePtr[11] = (idx2 + 1);
				_IdxWritePtr += 12; idx1 = idx2;
			}

			//add vertexes
			for (int i = 0; i < PointsCount; ++i) {
				_VtxWritePtr[0] = { Points[i], { -1.f, 0.f }, color };
				_VtxWritePtr[1] = { temp_points[i * 2 + 0], { -1.f, 0.f }, col_trans };
				_VtxWritePtr[2] = { temp_points[i * 2 + 1], { -1.f, 0.f }, col_trans };
				_VtxWritePtr += 3;
			}
		}

		else 
		{
			//calc half thickness line
			float half_inner_thickness = (Thick - 1.f) * 0.5f;
			
			//fix closed
			if (!Closed) {
				temp_points[1] = Points[0] + temp_normals[0] * (half_inner_thickness);
				temp_points[2] = Points[0] - temp_normals[0] * (half_inner_thickness);
				temp_points[3] = Points[0] - temp_normals[0] * (half_inner_thickness + 1.f);
				temp_points[0] = Points[0] + temp_normals[0] * (half_inner_thickness + 1.f);
				temp_points[(PointsCount - 1) * 4 + 1] = Points[PointsCount - 1] + temp_normals[PointsCount - 1] * (half_inner_thickness);
				temp_points[(PointsCount - 1) * 4 + 2] = Points[PointsCount - 1] - temp_normals[PointsCount - 1] * (half_inner_thickness);
				temp_points[(PointsCount - 1) * 4 + 0] = Points[PointsCount - 1] + temp_normals[PointsCount - 1] * (half_inner_thickness + 1.f);
				temp_points[(PointsCount - 1) * 4 + 3] = Points[PointsCount - 1] - temp_normals[PointsCount - 1] * (half_inner_thickness + 1.f);
			}

			unsigned int idx1 = 0;
			for (int i1 = 0; i1 < count; ++i1)
			{
				//average normals
				int i2 = (i1 + 1) == PointsCount ? 0 : i1 + 1;
				unsigned int idx2 = (i1 + 1) == PointsCount ? 0 : idx1 + 4;
				vec2_t dm = (temp_normals[i1] + temp_normals[i2]) * 0.5f;
				float dmr2 = ((dm.x * dm.x) + (dm.y * dm.y));
				if (dmr2 > 0.000001f) { 
					float scale = 1.f / dmr2;
					dm *= min(scale, 100.f);
				}

				//add vertexes
				vec2_t dm_in = dm * half_inner_thickness;
				vec2_t dm_out = dm * (half_inner_thickness + 1.f);
				temp_points[i2 * 4 + 0] = Points[i2] + dm_out;
				temp_points[i2 * 4 + 3] = Points[i2] - dm_out;
				temp_points[i2 * 4 + 1] = Points[i2] + dm_in;
				temp_points[i2 * 4 + 2] = Points[i2] - dm_in;

				//add indexes
				_IdxWritePtr[0] =  (idx2 + 1); _IdxWritePtr[1] =  (idx1 + 1); _IdxWritePtr[2] =  (idx1 + 2);
				_IdxWritePtr[3] =  (idx1 + 2); _IdxWritePtr[4] =  (idx2 + 2); _IdxWritePtr[5] =  (idx2 + 1);
				_IdxWritePtr[6] =  (idx2 + 1); _IdxWritePtr[7] =  (idx1 + 1); _IdxWritePtr[8] =  (idx1 + 0);
				_IdxWritePtr[9] =  (idx1 + 0); _IdxWritePtr[10] = (idx2 + 0); _IdxWritePtr[11] = (idx2 + 1);
				_IdxWritePtr[12] = (idx2 + 2); _IdxWritePtr[13] = (idx1 + 2); _IdxWritePtr[14] = (idx1 + 3);
				_IdxWritePtr[15] = (idx1 + 3); _IdxWritePtr[16] = (idx2 + 3); _IdxWritePtr[17] = (idx2 + 2);
				_IdxWritePtr += 18; idx1 = idx2;
			}

			//add vertexes
			for (int i = 0; i < PointsCount; ++i) {
				_VtxWritePtr[1] = { temp_points[i * 4 + 1], { -1.f, 0.f }, color };
				_VtxWritePtr[2] = { temp_points[i * 4 + 2], { -1.f, 0.f }, color };
				_VtxWritePtr[0] = { temp_points[i * 4 + 0], { -1.f, 0.f }, col_trans };
				_VtxWritePtr[3] = { temp_points[i * 4 + 3], { -1.f, 0.f }, col_trans };
				_VtxWritePtr += 4;
			}
		}
	}
}

namespace Render
{
	class Color
	{
	public:
		DWORD DXColor;

		__forceinline Color() {
			DXColor = 0;
		}

		__forceinline Color(int R, int G, int B, int A = 255) {
			DXColor = ((UCHAR)R | ((UCHAR)G << 8) | ((UCHAR)B << 16) | ((UCHAR)A << 24));
		}

		__forceinline Color(float R, float G, float B, float A = 1.f) {
			UCHAR RC = (UCHAR)(R * 255.f), GC = (UCHAR)(G * 255.f); 
			UCHAR BC = (UCHAR)(B * 255.f), AC = (UCHAR)(A * 255.f);
			DXColor = ((RC) | ((GC) << 8) | ((BC) << 16) | ((AC) << 24));
		}
	};

	Color Grey()
	{
		return Color((int)150, (int)150, (int)150);
	}

	Color Gray()
	{
		return Color((int)150, (int)150, (int)150);
	}

	Color Blue()
	{
		return Color((int)0, (int)100, (int)250);
	}

	Color Purple()
	{
		return Color((int)150, (int)0, (int)200);
	}

	Color Gold()
	{
		return Color((int)250, (int)200, (int)0);
	}

	Color Red()
	{
		return Color((int)250, (int)0, (int)0);
	}

	Color Orange()
	{
		return Color((int)250, (int)200, (int)70);
	}

	Color Green()
	{
		return Color((int)0, (int)150, (int)50);
	}

	Color LightGreen()
	{
		return Color((int)50, (int)250, (int)75);
	}

	Color LightPurple()
	{
		return Color((int)250, (int)200, (int)250);
	}

	Color Aqua()
	{
		return Color((int)0, (int)200, (int)225);
	}

	//render mgr
	__forceinline bool NewFrame(IDXGISwapChain* SwapChain, vec2_t& ScreenSize) {
		return RenderToolKit::NewFrame(SwapChain, ScreenSize);
	}

	__forceinline void EndFrame(IDXGISwapChain* SwapChain) {
		RenderToolKit::EndFrame(SwapChain);
	}

	//render line
	__forceinline void Line(const vec2_t& Start, const vec2_t& End, const Color& Color, float Thickness = 1.f) {
		vec2_t Dots[2] = { Start, End }; RenderToolKit::AddPolyLineAA(Dots, 2, Color.DXColor, false, Thickness);
	}

	//render circle
	void Circle(const vec2_t& Start, const Color& Color, float Radius, float Thickness = 1.f) 
	{
		//gen dots
		vec2_t Dots[90]; int NumDots = 0; //max 90
		for (float i = 0.f; i < 6.28f; i += 0.14f) {
			Dots[NumDots++] = {
				Start.x + Radius * RenderToolKit::FastCos(i),
				Start.y + Radius * RenderToolKit::FastSin(i)
			};
		}

		//draw curcle
		RenderToolKit::AddPolyLineAA(Dots, NumDots, Color.DXColor, true, Thickness);
	}
	
	void FillCircle(const vec2_t& Start, const Color& Color, float Radius)
	{
		//gen dots
		vec2_t Dots[90]; int NumDots = 0; //max 90
		for (float i = 0.f; i < 6.28f; i += 0.14f) {
			Dots[NumDots++] = {
				Start.x + Radius * RenderToolKit::FastCos(i),
				Start.y + Radius * RenderToolKit::FastSin(i)
			};
		}

		//draw filled curcle
		RenderToolKit::AddPolyFilledAA(Dots, NumDots, Color.DXColor);
	}

	//render rectangle
	__forceinline void Rectangle(const vec2_t& Start, const vec2_t& Size, const Color& Color, float Thickness = 1.f)
	{
		//check rounded
		vec2_t Pnts[4] = {
			Start, { Start.x, Start.y + Size.y },
			Start + Size, { Start.x + Size.x, Start.y },
		}; RenderToolKit::AddPolyLineAA(Pnts, 4,
			Color.DXColor, true, Thickness);
	}

	void RoundedRectangle(const vec2_t& Start, const vec2_t& Size, const Color& Color, float Radius, float Thickness = 1.f)
	{
		//gen dots
		vec2_t Dots[24];
		vec2_t Add = { 0.f, 0.f };
		for (int i = 0; i < 24; ++i)
		{
			//gen dot
			float angle = (float(i) / -24.f) * 6.28f - (6.28f / 16.f);
			Dots[i].x = Radius + Start.x + Add.x + (Radius * RenderToolKit::FastSin(angle));
			Dots[i].y = Size.y - Radius + Start.y + Add.y + (Radius * RenderToolKit::FastCos(angle));

			//calc offset
			if (i == 4) { Add.y = -Size.y + (Radius * 2.f); }
			else if (i == 10) { Add.x = Size.x - (Radius * 2.f); }
			else if (i == 16) Add.y = 0.f; else if (i == 22) Add.x = 0.f;
		} RenderToolKit::AddPolyLineAA(Dots, 24, Color.DXColor, true, Thickness);
	}

	void FillRectangle(const vec2_t& Start, const vec2_t& Size, const Color& Color)
	{
		//add draw cmd
		auto FillRect = RenderToolKit::AddDrawCmd(4, 6);

		//add vertexes
		auto _VtxWritePtr = FillRect.VBuffer;
		_VtxWritePtr[0] = { Start, { -1.f, 0.f }, Color.DXColor }; //top left
		_VtxWritePtr[3] = { Start + Size,  { -1.f, 0.f }, Color.DXColor }; //bottom right
		_VtxWritePtr[1] = { { Start.x + Size.x, Start.y }, { -1.f, 0.f }, Color.DXColor }; //upper right
		_VtxWritePtr[2] = { { Start.x, Start.y + Size.y }, { -1.f, 0.f }, Color.DXColor }; //bottom left

		//add indexes
		FillRect.IBuffer[0] = 0; FillRect.IBuffer[1] = 1; FillRect.IBuffer[2] = 2;
		FillRect.IBuffer[3] = 3; FillRect.IBuffer[4] = 1; FillRect.IBuffer[5] = 2;
	}

	void FillRoundedRectangle(const vec2_t& Start, const vec2_t& Size, const Color& Color, float Radius)
	{
		//gen dots
		vec2_t Dots[24];
		vec2_t Add = { 0.f, 0.f };
		for (int i = 0; i < 24; ++i) 
		{
			//gen dot
			float angle = (float(i) / -24.f) * 6.28f - (6.28f / 16.f);
			Dots[i].x = Radius + Start.x + Add.x + (Radius * RenderToolKit::FastSin(angle));
			Dots[i].y = Size.y - Radius + Start.y + Add.y + (Radius * RenderToolKit::FastCos(angle));

			//calc offset
			if (i == 4) { Add.y = -Size.y + (Radius * 2.f); }
			else if (i == 10) { Add.x = Size.x - (Radius * 2.f); }
			else if (i == 16) Add.y = 0.f;
			else if (i == 22) Add.x = 0.f;
		} RenderToolKit::AddPolyFilledAA(Dots, 24, Color.DXColor);
	}

	//render polygons
	__forceinline void PolyLineArray(vec2_t* Dots, int DotsCount, const Color& Color, bool Closed, float Thickness = 1.f) {
		RenderToolKit::AddPolyLineAA(Dots, DotsCount, Color.DXColor, Closed, Thickness);
	}

	__forceinline void PolyFillArray(vec2_t* Dots, int DotsCount, const Color& Color) {
		RenderToolKit::AddPolyFilledAA(Dots, DotsCount, Color.DXColor);
	}

	//render text
	vec2_t String(vec2_t Start, const wchar_t* String, bool Center = false, const Color& Color = Render::Color(255, 255, 255))
	{
		//get char desc & string size (in px)
		if (String == nullptr) return vec2_t{ 0.f, 0.f };
		int CharDescSize = 0; RenderToolKit::CharDesc CharDesc[256];
		vec2_t TextSize = GetTextSize(String, CharDesc, &CharDescSize);
		if (Center) { Start -= (TextSize / 2.f); }

		//build vertex array
		for (int i = 0; i < CharDescSize; ++i)
		{
			//add vertex to draw list
			auto FillChar = RenderToolKit::AddDrawCmd(4, 6);
			float srcX = (float)CharDesc[i].srcX, srcY = (float)CharDesc[i].srcY,
				  srcW = (float)CharDesc[i].srcW, srcH = (float)CharDesc[i].srcH,
				  xOff = (float)CharDesc[i].xOff, yOff = (float)CharDesc[i].yOff,
				  xAdv = (float)CharDesc[i].xAdv;

			//add vertexes
			auto _VtxWritePtr = FillChar.VBuffer;
			_VtxWritePtr[0] = { { Start.x + (xOff / FontTextureMultipler), //top left
				Start.y + (yOff / FontTextureMultipler) }, { srcX, srcY }, Color.DXColor };
			_VtxWritePtr[1] = { { Start.x + ((xOff + srcW) / FontTextureMultipler), //upper right
				Start.y + (yOff / FontTextureMultipler) }, { srcX + srcW, srcY }, Color.DXColor };
			_VtxWritePtr[2] = { { Start.x + (xOff / FontTextureMultipler), //bottom left
				Start.y + ((yOff + srcH) / FontTextureMultipler) }, { srcX, srcY + srcH }, Color.DXColor };
			_VtxWritePtr[3] = { { Start.x + ((xOff + srcW) / FontTextureMultipler), //bottom right
				Start.y + ((yOff + srcH) / FontTextureMultipler) }, { srcX + srcW, srcY + srcH }, Color.DXColor };

			//add indexes
			FillChar.IBuffer[0] = 0; FillChar.IBuffer[1] = 1; FillChar.IBuffer[2] = 2;
			FillChar.IBuffer[3] = 3; FillChar.IBuffer[4] = 1; FillChar.IBuffer[5] = 2;

			//set char spacing
			Start.x += (xAdv / FontTextureMultipler);
		} return TextSize;
	}
}