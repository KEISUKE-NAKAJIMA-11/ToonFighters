#include "renderpath.h"
#include <assert.h>

//ビューポートの設定
void RenderingPath::SetViewPort(ID3D11DeviceContext* pContext, UINT width, UINT height)
{
	D3D11_VIEWPORT vp = {};
	vp.Width = static_cast<float>(width);
	vp.Height = static_cast<float>(height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	pContext->RSSetViewports(1u, &vp);
}


//作成
void RenderingPath::Create(ID3D11Device* pDevice)
{
	//すでに初期化している場合は何もしない
	if (m_isInitialize) { return; }

	//RenderTargets
	m_rtForward = std::make_unique<TextureEX>();
	m_rtForward->Create(m_resolutionWidth, m_resolutionHeight, DXGI_FORMAT_R11G11B10_FLOAT);
	m_rtPostProcessHDR = std::make_unique<TextureEX>();
	m_rtPostProcessHDR->Create(m_resolutionWidth, m_resolutionHeight, DXGI_FORMAT_R11G11B10_FLOAT);
	m_rtPostProcessLDR = std::make_unique<TextureEX>();
	m_rtPostProcessLDR->Create(m_resolutionWidth, m_resolutionHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_rtFinal = std::make_unique<TextureEX>();
	m_rtFinal->Create(m_resolutionWidth, m_resolutionHeight, DXGI_FORMAT_R8G8B8A8_UNORM);

	//DepthBuffers
	m_depth = std::make_unique<TextureEX>();
	m_depth->CreateDepth(m_resolutionWidth, m_resolutionHeight, TextureEX::DepthFormat::eD24S8);
	m_depthCopy = std::make_unique<TextureEX>();
	m_depthCopy->CreateDepth(m_resolutionWidth, m_resolutionHeight, TextureEX::DepthFormat::eD24S8);
	m_shadowmap = std::make_unique<TextureEX>();
	m_shadowmap->CreateDepth(m_resolutionShadow, m_resolutionShadow, TextureEX::DepthFormat::eD32);

	//初期化完了
	m_isInitialize = true;
}
//解放
void RenderingPath::Destroy()
{
	//そもそも初期化していない場合は何もしない
	if (!m_isInitialize) { return; }

	//RenderTargets
	m_rtForward.reset();
	m_rtPostProcessHDR.reset();
	m_rtPostProcessLDR.reset();
	m_rtFinal.reset();

	//DepthBuffers
	m_depth.reset();
	m_shadowmap.reset();

	//BackBuffers
	if (m_backBufferRTV.Get() != nullptr) { m_backBufferRTV.Reset(); }
	if (m_backBufferDSV.Get() != nullptr) { m_backBufferDSV.Reset(); }
}


//現在設定されているRTVとDSVをバックバッファとする
void RenderingPath::CaptureBackBufferRTV(ID3D11DeviceContext* pContext)
{
	pContext->OMGetRenderTargets(1u, m_backBufferRTV.ReleaseAndGetAddressOf(), m_backBufferDSV.ReleaseAndGetAddressOf());
}

//現在の深度バッファをtempバッファにコピー
void RenderingPath::CopyDepthBuffer(ID3D11DeviceContext* pContext)
{
	ID3D11Resource* depthRes = nullptr;
	ID3D11Resource* depthCopyRes = nullptr;
	m_depth->GetDepthStencilView()->GetResource(&depthRes);
	m_depthCopy->GetDepthStencilView()->GetResource(&depthCopyRes);
	pContext->CopyResource(depthCopyRes, depthRes);
	depthRes->Release();
	depthCopyRes->Release();
}

//パスの開始
void RenderingPath::BeginPath(ID3D11DeviceContext* pContext, PathType type)
{
	const FLOAT black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	//レンダーターゲットの設定
	switch (type)
	{
		//深度値を先に書き出し
	case RenderingPath::PathType::ePreDepth:
	{
		pContext->OMSetRenderTargets(0, nullptr, m_depth->GetDepthStencilView());
	}
	break;

	//シャドウマップ生成
	case RenderingPath::PathType::eShadow:
	{
		pContext->ClearDepthStencilView(m_shadowmap->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
		pContext->OMSetRenderTargets(0, nullptr, m_shadowmap->GetDepthStencilView());
	}
	break;

	//3Dシーン描画(HDR)
	case RenderingPath::PathType::eForward:
	{
		ID3D11RenderTargetView* rtvs[] = {
			m_rtForward->GetRenderTargetView()
		};
		pContext->ClearRenderTargetView(m_rtForward->GetRenderTargetView(), black);
		pContext->ClearDepthStencilView(m_depth->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		pContext->OMSetRenderTargets(ARRAYSIZE(rtvs), rtvs, m_depth->GetDepthStencilView());
	}
	break;

	//ポストエフェクト描画(HDR)
	case RenderingPath::PathType::ePostProcessHDR:
	{
		ID3D11RenderTargetView* rtvs[] = {
			m_rtPostProcessHDR->GetRenderTargetView()
		};
		pContext->OMSetRenderTargets(ARRAYSIZE(rtvs), rtvs, m_depth->GetDepthStencilView());
	}
	break;

	//ポストエフェクト描画(LDR)
	case RenderingPath::PathType::ePostProcessLDR:
	{
		ID3D11RenderTargetView* rtvs[] = {
			m_rtPostProcessLDR->GetRenderTargetView()
		};
		pContext->OMSetRenderTargets(ARRAYSIZE(rtvs), rtvs, m_depth->GetDepthStencilView());
	}
	break;

	//最終描画 + 2Dシーン描画(LDR)
	case RenderingPath::PathType::eFinal:
	{
		ID3D11RenderTargetView* rtvs[] = {
			m_rtFinal->GetRenderTargetView()
		};
		pContext->OMSetRenderTargets(ARRAYSIZE(rtvs), rtvs, m_depth->GetDepthStencilView());
	}
	break;

	//バックバッファ
	case RenderingPath::PathType::eBackBuffer:
	{
		ID3D11RenderTargetView* rtvs[] = {
			m_backBufferRTV.Get()
		};
		pContext->OMSetRenderTargets(ARRAYSIZE(rtvs), rtvs, m_backBufferDSV.Get());
	}
	break;


	//エラー
	case RenderingPath::PathType::eMax:
	default:
		assert(!"invalid type used!!");
		return;
		break;
	}


	//ビューポートの設定
	if (type == PathType::eShadow)
	{
		SetViewPort(pContext, m_resolutionShadow, m_resolutionShadow);
	}
	else
	{
		SetViewPort(pContext, m_resolutionWidth, m_resolutionHeight);
	}
}

//パスの終了
void RenderingPath::EndPath(ID3D11DeviceContext* pContext)
{
	ID3D11RenderTargetView* rtvs[] = {
		nullptr,
	};
	pContext->OMSetRenderTargets(ARRAYSIZE(rtvs), rtvs, nullptr);
}


//描画したテクスチャのShaderResourceViewを取得
//NOTE:該当するテクスチャがレンダーターゲットに設定されている間は使用できない
ID3D11ShaderResourceView* RenderingPath::GetShaderResource(PathType type)
{
	switch (type)
	{
	case RenderingPath::PathType::ePreDepth:
		return m_depth->GetDepthShaderResource();
		break;
	case RenderingPath::PathType::eShadow:
		return m_shadowmap->GetDepthShaderResource();
		break;
	case RenderingPath::PathType::eForward:
		return m_rtForward->GetShaderResource();
		break;
	case RenderingPath::PathType::ePostProcessHDR:
		return m_rtPostProcessHDR->GetShaderResource();
		break;
	case RenderingPath::PathType::ePostProcessLDR:
		return m_rtPostProcessLDR->GetShaderResource();
		break;
	case RenderingPath::PathType::eFinal:
		return m_rtFinal->GetShaderResource();
		break;


		//エラー
	case RenderingPath::PathType::eMax:
	default:
		assert(!"invalid type used!!");
		return nullptr;
		break;
	}
}
ID3D11ShaderResourceView* RenderingPath::GetDepthCopy() { return m_depthCopy->GetDepthShaderResource(); }
