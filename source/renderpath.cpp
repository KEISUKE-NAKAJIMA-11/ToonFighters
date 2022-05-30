#include "renderpath.h"
#include <assert.h>

//�r���[�|�[�g�̐ݒ�
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


//�쐬
void RenderingPath::Create(ID3D11Device* pDevice)
{
	//���łɏ��������Ă���ꍇ�͉������Ȃ�
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

	//����������
	m_isInitialize = true;
}
//���
void RenderingPath::Destroy()
{
	//�����������������Ă��Ȃ��ꍇ�͉������Ȃ�
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


//���ݐݒ肳��Ă���RTV��DSV���o�b�N�o�b�t�@�Ƃ���
void RenderingPath::CaptureBackBufferRTV(ID3D11DeviceContext* pContext)
{
	pContext->OMGetRenderTargets(1u, m_backBufferRTV.ReleaseAndGetAddressOf(), m_backBufferDSV.ReleaseAndGetAddressOf());
}

//���݂̐[�x�o�b�t�@��temp�o�b�t�@�ɃR�s�[
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

//�p�X�̊J�n
void RenderingPath::BeginPath(ID3D11DeviceContext* pContext, PathType type)
{
	const FLOAT black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	//�����_�[�^�[�Q�b�g�̐ݒ�
	switch (type)
	{
		//�[�x�l���ɏ����o��
	case RenderingPath::PathType::ePreDepth:
	{
		pContext->OMSetRenderTargets(0, nullptr, m_depth->GetDepthStencilView());
	}
	break;

	//�V���h�E�}�b�v����
	case RenderingPath::PathType::eShadow:
	{
		pContext->ClearDepthStencilView(m_shadowmap->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
		pContext->OMSetRenderTargets(0, nullptr, m_shadowmap->GetDepthStencilView());
	}
	break;

	//3D�V�[���`��(HDR)
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

	//�|�X�g�G�t�F�N�g�`��(HDR)
	case RenderingPath::PathType::ePostProcessHDR:
	{
		ID3D11RenderTargetView* rtvs[] = {
			m_rtPostProcessHDR->GetRenderTargetView()
		};
		pContext->OMSetRenderTargets(ARRAYSIZE(rtvs), rtvs, m_depth->GetDepthStencilView());
	}
	break;

	//�|�X�g�G�t�F�N�g�`��(LDR)
	case RenderingPath::PathType::ePostProcessLDR:
	{
		ID3D11RenderTargetView* rtvs[] = {
			m_rtPostProcessLDR->GetRenderTargetView()
		};
		pContext->OMSetRenderTargets(ARRAYSIZE(rtvs), rtvs, m_depth->GetDepthStencilView());
	}
	break;

	//�ŏI�`�� + 2D�V�[���`��(LDR)
	case RenderingPath::PathType::eFinal:
	{
		ID3D11RenderTargetView* rtvs[] = {
			m_rtFinal->GetRenderTargetView()
		};
		pContext->OMSetRenderTargets(ARRAYSIZE(rtvs), rtvs, m_depth->GetDepthStencilView());
	}
	break;

	//�o�b�N�o�b�t�@
	case RenderingPath::PathType::eBackBuffer:
	{
		ID3D11RenderTargetView* rtvs[] = {
			m_backBufferRTV.Get()
		};
		pContext->OMSetRenderTargets(ARRAYSIZE(rtvs), rtvs, m_backBufferDSV.Get());
	}
	break;


	//�G���[
	case RenderingPath::PathType::eMax:
	default:
		assert(!"invalid type used!!");
		return;
		break;
	}


	//�r���[�|�[�g�̐ݒ�
	if (type == PathType::eShadow)
	{
		SetViewPort(pContext, m_resolutionShadow, m_resolutionShadow);
	}
	else
	{
		SetViewPort(pContext, m_resolutionWidth, m_resolutionHeight);
	}
}

//�p�X�̏I��
void RenderingPath::EndPath(ID3D11DeviceContext* pContext)
{
	ID3D11RenderTargetView* rtvs[] = {
		nullptr,
	};
	pContext->OMSetRenderTargets(ARRAYSIZE(rtvs), rtvs, nullptr);
}


//�`�悵���e�N�X�`����ShaderResourceView���擾
//NOTE:�Y������e�N�X�`���������_�[�^�[�Q�b�g�ɐݒ肳��Ă���Ԃ͎g�p�ł��Ȃ�
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


		//�G���[
	case RenderingPath::PathType::eMax:
	default:
		assert(!"invalid type used!!");
		return nullptr;
		break;
	}
}
ID3D11ShaderResourceView* RenderingPath::GetDepthCopy() { return m_depthCopy->GetDepthShaderResource(); }
