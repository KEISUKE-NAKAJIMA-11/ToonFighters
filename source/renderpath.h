#pragma once

#include "ShederTexture.h"
#include <memory>

//----------------------------------------------------
// 描画パスの管理を行う
//----------------------------------------------------

//----------------------------------------------------
// 今のところ実装している描画パイプラインは管理クラスがなく、
//シャドウマップやディファードレンダリング等の実装が少し手間なので描画パス管理クラスを
//この先のために設計する。
//----------------------------------------------------


class RenderingPath final
{
public:
	enum class PathType
	{
		ePreDepth,			//深度値のみを先に書き出し
		eShadow,			//シャドウマップの生成パス(グローバルディレクショナルライトのみ)
		eForward,			//３Ｄモデルの描画パス
		ePostProcessHDR,	//ポストエフェクトの描画処理(HDR)
		ePostProcessLDR,	//ポストエフェクトの描画処理(LDR)
		eFinal,				//最終描画パス(2Dの描画)
		eBackBuffer,		//バックバッファ

		eMax,
	};

private:
	//RenderTargets
	std::unique_ptr<TextureEX> m_rtForward;
	std::unique_ptr<TextureEX> m_rtPostProcessHDR;
	std::unique_ptr<TextureEX> m_rtPostProcessLDR;
	std::unique_ptr<TextureEX> m_rtFinal;

	//DepthBuffers
	std::unique_ptr<TextureEX> m_depth;
	std::unique_ptr<TextureEX> m_depthCopy;
	std::unique_ptr<TextureEX> m_shadowmap;

	//Resolution
	const UINT m_resolutionWidth = 1920u;
	const UINT m_resolutionHeight = 1080u;
	const UINT m_resolutionShadow = 512u;

	//BackBuffer
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_backBufferRTV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_backBufferDSV;

	bool m_isInitialize = false;

	//ビューポートの設定
	void SetViewPort(ID3D11DeviceContext* pContext, UINT width, UINT height);

public:
	//初期化
	void Create(ID3D11Device* pDevice);
	//解放
	void Destroy();

	//現在設定されているRTVとDSVをバックバッファとする
	void CaptureBackBufferRTV(ID3D11DeviceContext* pContext);

	//現在の深度バッファをtempバッファにコピー
	void CopyDepthBuffer(ID3D11DeviceContext* pContext);

	//パスの設定
	void BeginPath(ID3D11DeviceContext* pContext, PathType type);
	void EndPath(ID3D11DeviceContext* pContext);

	//描画したテクスチャのShaderResourceViewを取得
	//NOTE:該当するテクスチャがレンダーターゲットに設定されている間は使用できない
	ID3D11ShaderResourceView* GetShaderResource(PathType type);
	ID3D11ShaderResourceView* GetDepthCopy();
};