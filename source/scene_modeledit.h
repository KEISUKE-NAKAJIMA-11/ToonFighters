#pragma once
#include <future>

#include "font.h"
#include "assain.h"
#include "scene.h"
#include "scene.h"
#include "tonemap.h"
#include "blender.h"
#include "framework.h"
#include <Windows.h>
#include "Effect.h"
#include "shadertoy_test.h"
#include "cameracontroller.h"

#include "line_renderer.h"

class Scene_model_editor : public scene
{
private:

	DirectX::XMFLOAT3 eyes, forcuss, ups;

	std::shared_ptr <Camera>			camera_t;
	std::shared_ptr <MapCamera>				mapcamera_t;


	int timer;
	std::unique_ptr<Font>				font;
	//std::unique_ptr<GamePad>			gamePad;

	std::unique_ptr<ToneMap> tonemap_effect;
	std::unique_ptr<framebuffer>	framebuffers[4];
	std::unique_ptr<Bloom>			bloom_effect;

	std::unique_ptr<rasterizer>			Rasterizer;
	std::unique_ptr<fullscreen_quad>	Fullscreen_quad;
	std::unique_ptr<Sadertoy_test> shadertoy;

	/////////////////////////////////////////

	CameraController				camera;
	std::shared_ptr<ModelResource>	modelResource;
	std::unique_ptr<Model>			model;
	std::unique_ptr<ModelRenderer>			modelrenderer;
	std::unique_ptr<LineRenderer>			linerenderer;

	Model::s_node* selectionNode = nullptr;
	int								selectionAnimationIndex = -1;
	bool							hiddenHierarchy = false;
	bool							hiddenProperty = false;
	bool							hiddenAnimation = false;
	bool							hiddenTimeline = false;

	bool							animationLoop = false;
	bool							animationPause = false;
	bool active = true;



	


	DirectX::XMFLOAT4 lightDirection = DirectX::XMFLOAT4(0, -1, -0.60f, 0);
	DirectX::XMMATRIX W;
public:
	bool initialize(ID3D11Device* device, unsigned int screen_width, unsigned int screen_height, const char* nextscene);

	const char* update(ID3D11Device* device, float& elapsed_time/*Elapsed seconds from last frame*/);
	std::future<void> future;

	void render(ID3D11DeviceContext* immediate_context, float elapsed_time, blender* Blender);

	void uninitialize();



	// �M�Y���`��
	void draw_guizmo(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection, Model::s_node* node);

	// �O���b�h�`��
	void drawgrid(ID3D11DeviceContext* context, int subdivisions, float scale);

	// GUI�`��
	void draw_gui();

	// ���j���[GUI�`��
	void draw_menuGUI();

	// �m�[�hGUI�`��
	void draw_node_gui(Model::s_node* node);

	// �q�G�����L�[GUI�`��
	void draw_hierarchy_gui();

	// �v���p�e�BGUI�`��
	void draw_propertygui();

	// �A�j���[�V����GUI�`��
	void draw_animation_gui();

	// �^�C�����C��GUI�`��
	void draw_timeline_gui();

	// ���f���t�@�C�����J��
	void open_model_file();

	// ���f���t�@�C����ۑ�
	void save_model_file();

	// �A�j���[�V�����t�@�C�����J��
	void open_animation_file();

	// ���f������
	void import_model(const char* filename);

	// �A�j���[�V��������
	void import_animation(const char* filename);

	// ���f���o��
	void export_moodel(std::string filename);





	// �I���A�j���[�V�������擾
	ModelResource::s_animation* GetSelectionAnimation();



	Scene_model_editor* getscene() { return this; }
};