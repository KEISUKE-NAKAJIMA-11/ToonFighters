#pragma once
#pragma once
#include<wrl.h>
#include <d3d11.h>

//-----------------------------------------------------------------
//			�e�N�X�`���N���X
//-----------------------------------------------------------------
//�E�䂭�䂭�̓e�N�X�`���S�Ă��Ǘ�����N���X�ɂȂ�\��
//�E�V�F�[�_�[���\�[�X�����łȂ������_�[�^�[�Q�b�g�ɂ��ݒ�ł���
//-----------------------------------------------------------------

class TextureEX
{
public:
	enum class DepthFormat
	{
		eD32,
		eD24S8,
	};

protected:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	ShaderResouceView = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	DepthShaderResouceView = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>		DepthStencilView = nullptr;

	//�e�N�X�`�����
	D3D11_TEXTURE2D_DESC	texture2d_desc;

public:
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		RenderTargetView = nullptr;

	TextureEX();
	TextureEX(const wchar_t* filename);
	~TextureEX() {}

	bool Load(const wchar_t* filename);
	void Set(UINT slot, BOOL flg = TRUE);

	UINT GetWidth() const { return texture2d_desc.Width; };//�摜�̑S�̉��T�C�Y��Ԃ�
	UINT GetHeight() const { return texture2d_desc.Height; };//�摜�̑S�̏c�T�C�Y��Ԃ�
	bool Create(u_int width, u_int height, DXGI_FORMAT format);

	//�[�x�X�e���V���o�b�t�@�쐬
	bool CreateDepth(u_int width, u_int height, DepthFormat format);

	ID3D11RenderTargetView* GetRenderTargetView() { return RenderTargetView.Get(); };
	ID3D11DepthStencilView* GetDepthStencilView() { return DepthStencilView.Get(); };
	ID3D11ShaderResourceView* GetShaderResource() { return ShaderResouceView.Get(); };
	ID3D11ShaderResourceView* GetDepthShaderResource() { return DepthShaderResouceView.Get(); };

};
