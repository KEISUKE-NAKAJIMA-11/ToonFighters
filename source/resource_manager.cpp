#include "resource_manager.h"

void resourceManager::s_resource_shader_resource_views::Release(bool _force)
{
	//インデックスは0以上のはずなので0は例外なので終了
	if (i_ref_num == 0) return;

	//インデックスをインクリメントして0になるとき(0は1文前で処理しているので1の時のみ)
	if (--i_ref_num <= 0) _force = true;

	//forceがtrueの場合はshaderResourceViewを解放する
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
	//リファレンスのインデックスが0にはならない仕様なので例外
	if (i_ref_num == 0) return;

	//リファレンスのインデックスが1か0以下の時解放処理を実行する
	if (--i_ref_num <= 0) _force = true;

	//fourceがtrueの時解放処理を行う
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
	//リファレンスのインデックスは0にならない仕様なので例外
	if (i_ref_num == 0) return;

	//インデックスが0またはマイナスの値の場合は解放処理を行う
	if (--i_ref_num <= 0) _force = true;

	//解放処理
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
	//リファレンスのインデックスは0にならない仕様なので例外
	if (i_ref_num == 0) return;

	//インデックスが0またはマイナスの値の場合は解放処理を行う
	if (--i_ref_num <= 0) _force = true;

	//解放処理
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
	//リファレンスのインデックスは0にならない仕様なので例外
	if (i_ref_num == 0) return;

	//インデックスが0またはマイナスの値の場合は解放処理を行う
	if (--i_ref_num <= 0) _force = true;

	//解放処理
	if (_force)
	{
		if (geometry_shader) geometry_shader->Release();
		geometry_shader = nullptr;
		i_ref_num = 0;
		path[0] = '\0';
	}
}
