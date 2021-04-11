/*
	This file is part of D2DX.

	Copyright (C) 2021  Bolrog

	D2DX is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	D2DX is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with D2DX.  If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once

#include "IRenderContext.h"
#include "ISimd.h"
#include "ITextureCache.h"
#include "Types.h"

namespace d2dx
{
	class Vertex;
	class Batch;

	enum class RenderContextSyncStrategy
	{
		AllowTearing = 0,
		Interval0 = 1,
		FrameLatencyWaitableObject = 2,
		Interval1 = 3,
	};

	enum class RenderContextSwapStrategy
	{
		FlipDiscard = 0,
		Discard = 1,
	};

	enum class RenderContextBackbufferSizingStrategy
	{
		SetSourceSize = 0,
		ResizeBuffers = 1,
	};

	class RenderContext final : public RuntimeClass<
		RuntimeClassFlags<RuntimeClassType::ClassicCom>,
		IRenderContext
	>
	{
	public:
		RenderContext(
			HWND hWnd,
			Size gameSize,
			Size windowSize,
			Options& options,
			ISimd* simd);

		virtual ~RenderContext();

		virtual void LoadGammaTable(
			_In_reads_(valueCount) const uint32_t* values,
			_In_ uint32_t valueCount) override;

		virtual void BulkWriteVertices(
			_In_reads_(vertexCount) const Vertex* vertices,
			_In_ uint32_t vertexCount) override;

		virtual TextureCacheLocation UpdateTexture(
			_In_ const Batch& batch,
			_In_reads_(tmuDataSize) const uint8_t* tmuData,
			_In_ uint32_t tmuDataSize) override;

		virtual void Draw(
			_In_ const Batch& batch) override;

		virtual void Present() override;

		virtual void WriteToScreen(
			_In_reads_(width* height) const uint32_t* pixels,
			_In_ int32_t width,
			_In_ int32_t height) override;
		
		virtual void SetPalette(
			_In_ int32_t paletteIndex,
			_In_reads_(256) const uint32_t* palette) override;

		virtual const Options& GetOptions() const override;

		virtual ITextureCache* GetTextureCache(const Batch& batch) const override;

		virtual void SetSizes(
			_In_ Size gameSize,
			_In_ Size windowSize) override;

		virtual void GetCurrentMetrics(
			_Out_ Size* gameSize,
			_Out_ Rect* renderRect,
			_Out_ Size* desktopSize) const override;

		virtual void ToggleFullscreen() override;

	private:
		void CreateRasterizerState();
		void CreateVertexAndIndexBuffers();
		void CreateShadersAndInputLayout();
		void CreateConstantBuffers();
		void CreateGammaTexture();
		void CreatePaletteTexture();
		void CreateVideoTextures();
		void CreateGameTexture();
		void CreateSamplerStates();
		void CreateTextureCaches();
		uint32_t DetermineMaxTextureArraySize();
		bool IsIntegerScale() const;

		void UpdateViewport(Rect rect);

		void SetBlendState(AlphaBlend alphaBlend);

		void AdjustWindowPlacement(
			_In_ HWND hWnd,
			bool centerOnCurrentPosition);

		uint32_t UpdateVerticesWithFullScreenQuad(
			Size srcSize,
			Rect dstRect);

		bool IsFrameLatencyWaitableObjectSupported() const;

		bool IsAllowTearingFlagSupported() const;

		void ResizeBackbuffer();

		void SetVS(
			_In_ ID3D11VertexShader* vs);
		void SetPS(
			_In_ ID3D11PixelShader* ps);

		void SetBlendState(
			_In_ ID3D11BlendState* blendState);

		void SetPSShaderResourceViews(
			_In_ ID3D11ShaderResourceView* srvs[2]);

		void SetPrimitiveTopology(
			D3D11_PRIMITIVE_TOPOLOGY pt);

		struct Constants
		{
			float screenSize[2];
			float dummy[2];
		};

		static_assert(sizeof(Constants) == 4 * 4, "size of Constants");

		Size _gameSize;
		Rect _renderRect;
		Size _windowSize;
		Size _desktopSize;
		int32_t _desktopClientMaxHeight;

		uint32_t _vbWriteIndex;
		uint32_t _vbCapacity;

		Constants _constants;

		RenderContextSyncStrategy _syncStrategy;
		RenderContextSwapStrategy _swapStrategy;
		RenderContextBackbufferSizingStrategy _backbufferSizingStrategy;

		DWORD _swapChainCreateFlags;
		bool _dxgiAllowTearingFlagSupported;
		bool _frameLatencyWaitableObjectSupported;
		D3D_FEATURE_LEVEL _featureLevel;
		ComPtr<ID3D11Device> _device;
		ComPtr<ID3D11Device3> _device3;
		ComPtr<ID3D11DeviceContext> _deviceContext;
		ComPtr<ID3D11DeviceContext1> _deviceContext1;
		ComPtr<IDXGISwapChain1> _swapChain1;
		ComPtr<IDXGISwapChain2> _swapChain2;
		ComPtr<ID3D11RasterizerState> _rasterizerStateNoScissor;
		ComPtr<ID3D11RasterizerState> _rasterizerState;
		ComPtr<ID3D11InputLayout> _inputLayout;
		ComPtr<ID3D11Buffer> _vb;
		ComPtr<ID3D11Buffer> _cb;
		ComPtr<ID3D11VertexShader> _gameVS;
		ComPtr<ID3D11PixelShader> _gamePS;
		ComPtr<ID3D11PixelShader> _videoPS;
		ComPtr<ID3D11VertexShader> _displayVS;
		ComPtr<ID3D11PixelShader> _displayIntegerScalePS;
		ComPtr<ID3D11PixelShader> _displayNonintegerScalePS;
		ComPtr<ID3D11RenderTargetView> _backbufferRtv;
		ComPtr<ID3D11SamplerState> _samplerState[2];

		ComPtr<ID3D11Texture2D> _videoTexture;
		ComPtr<ID3D11ShaderResourceView> _videoTextureSrv;

		ComPtr<ID3D11BlendState> _blendStates[(int32_t)AlphaBlend::Count];

		ComPtr<ID3D11Texture1D> _gammaTexture;
		ComPtr<ID3D11ShaderResourceView> _gammaTextureSrv;

		ComPtr<ID3D11Texture1D> _paletteTexture;
		ComPtr<ID3D11ShaderResourceView> _paletteTextureSrv;

		ComPtr<ID3D11Texture2D> _gameTexture;
		ComPtr<ID3D11RenderTargetView> _gameTextureRtv;
		ComPtr<ID3D11ShaderResourceView> _gameTextureSrv;

		ComPtr<ITextureCache> _textureCaches[6];

		HWND _hWnd;
		Options& _options;

		struct
		{
			ID3D11VertexShader* _lastVS;
			ID3D11PixelShader* _lastPS;
			ID3D11BlendState* _lastBlendState;
			ID3D11ShaderResourceView* _psSrvs[2];
			D3D11_PRIMITIVE_TOPOLOGY _primitiveTopology;
			Constants _constants;
		} _shadowState;

		HANDLE _frameLatencyWaitableObject;
		ComPtr<ISimd> _simd;
	};
}