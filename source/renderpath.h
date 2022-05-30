#pragma once

#include "ShederTexture.h"
#include <memory>

//----------------------------------------------------
// �`��p�X�̊Ǘ����s��
//----------------------------------------------------

//----------------------------------------------------
// ���̂Ƃ���������Ă���`��p�C�v���C���͊Ǘ��N���X���Ȃ��A
//�V���h�E�}�b�v��f�B�t�@�[�h�����_�����O���̎�����������ԂȂ̂ŕ`��p�X�Ǘ��N���X��
//���̐�̂��߂ɐ݌v����B
//----------------------------------------------------


class RenderingPath final
{
public:
	enum class PathType
	{
		ePreDepth,			//�[�x�l�݂̂��ɏ����o��
		eShadow,			//�V���h�E�}�b�v�̐����p�X(�O���[�o���f�B���N�V���i�����C�g�̂�)
		eForward,			//�R�c���f���̕`��p�X
		ePostProcessHDR,	//�|�X�g�G�t�F�N�g�̕`�揈��(HDR)
		ePostProcessLDR,	//�|�X�g�G�t�F�N�g�̕`�揈��(LDR)
		eFinal,				//�ŏI�`��p�X(2D�̕`��)
		eBackBuffer,		//�o�b�N�o�b�t�@

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

	//�r���[�|�[�g�̐ݒ�
	void SetViewPort(ID3D11DeviceContext* pContext, UINT width, UINT height);

public:
	//������
	void Create(ID3D11Device* pDevice);
	//���
	void Destroy();

	//���ݐݒ肳��Ă���RTV��DSV���o�b�N�o�b�t�@�Ƃ���
	void CaptureBackBufferRTV(ID3D11DeviceContext* pContext);

	//���݂̐[�x�o�b�t�@��temp�o�b�t�@�ɃR�s�[
	void CopyDepthBuffer(ID3D11DeviceContext* pContext);

	//�p�X�̐ݒ�
	void BeginPath(ID3D11DeviceContext* pContext, PathType type);
	void EndPath(ID3D11DeviceContext* pContext);

	//�`�悵���e�N�X�`����ShaderResourceView���擾
	//NOTE:�Y������e�N�X�`���������_�[�^�[�Q�b�g�ɐݒ肳��Ă���Ԃ͎g�p�ł��Ȃ�
	ID3D11ShaderResourceView* GetShaderResource(PathType type);
	ID3D11ShaderResourceView* GetDepthCopy();
};