#include "resource_manager.h"

void resourceManager::s_resource_shader_resource_views::Release(bool _force)
{
	//�C���f�b�N�X��0�ȏ�̂͂��Ȃ̂�0�͗�O�Ȃ̂ŏI��
	if (i_ref_num == 0) return;

	//�C���f�b�N�X���C���N�������g����0�ɂȂ�Ƃ�(0��1���O�ŏ������Ă���̂�1�̎��̂�)
	if (--i_ref_num <= 0) _force = true;

	//force��true�̏ꍇ��shaderResourceView���������
	if (_force)
	{
		if (shader_resource_view)
		{
			shader_resource_view->Release();
		}
		shader_resource_view = nullptr;
		i_ref_num = 0;
		path[0] = '\0';
	}
}

void resourceManager::s_resource_vertex_shaders::Release(bool _force)
{
	//���t�@�����X�̃C���f�b�N�X��0�ɂ͂Ȃ�Ȃ��d�l�Ȃ̂ŗ�O
	if (i_ref_num == 0) return;

	//���t�@�����X�̃C���f�b�N�X��1��0�ȉ��̎�������������s����
	if (--i_ref_num <= 0) _force = true;

	//fource��true�̎�����������s��
	if (_force)
	{
		if (layout) layout->Release();

		if (vertex_shader) vertex_shader->Release();

		layout = nullptr;
		vertex_shader = nullptr;

		i_ref_num = 0;
		path[0] = '\0';
	}
}

void resourceManager::s_resource_pixel_shaders::Release(bool _force)

{
	//���t�@�����X�̃C���f�b�N�X��0�ɂȂ�Ȃ��d�l�Ȃ̂ŗ�O
	if (i_ref_num == 0) return;

	//�C���f�b�N�X��0�܂��̓}�C�i�X�̒l�̏ꍇ�͉���������s��
	if (--i_ref_num <= 0) _force = true;

	//�������
	if (_force)
	{
		if (pixel_shader) pixel_shader->Release();
		pixel_shader = nullptr;
		i_ref_num = 0;
		path[0] = '\0';
	}
}

void resourceManager::s_resource_compute_shaders::Release(bool _force)

{
	//���t�@�����X�̃C���f�b�N�X��0�ɂȂ�Ȃ��d�l�Ȃ̂ŗ�O
	if (i_ref_num == 0) return;

	//�C���f�b�N�X��0�܂��̓}�C�i�X�̒l�̏ꍇ�͉���������s��
	if (--i_ref_num <= 0) _force = true;

	//�������
	if (_force)
	{
		if (compute_shader) compute_shader->Release();
		compute_shader = nullptr;
		i_ref_num = 0;
		path[0] = '\0';
	}
}

void resourceManager::s_resource_geometry_shaders::Release(bool _force)

{
	//���t�@�����X�̃C���f�b�N�X��0�ɂȂ�Ȃ��d�l�Ȃ̂ŗ�O
	if (i_ref_num == 0) return;

	//�C���f�b�N�X��0�܂��̓}�C�i�X�̒l�̏ꍇ�͉���������s��
	if (--i_ref_num <= 0) _force = true;

	//�������
	if (_force)
	{
		if (geometry_shader) geometry_shader->Release();
		geometry_shader = nullptr;
		i_ref_num = 0;
		path[0] = '\0';
	}
}
