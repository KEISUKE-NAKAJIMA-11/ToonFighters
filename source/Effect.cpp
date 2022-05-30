#include "framework.h"

Effect effect;

void Effect::EffectInit(ID3D11Device* device)
{
	 // Ganrate Effekseer Renderer
	effekseerRenderer = EffekseerRendererDX11::Renderer::Create(device, framework::Instance().GetDeviceContext(), 10000);
	
	// Ganrate Effekseer Manager
	effekseerManager = Effekseer::Manager::Create(10000);
	
	// Effekseer Renderer setting
	effekseerManager->SetSpriteRenderer(effekseerRenderer->CreateSpriteRenderer());
	effekseerManager->SetRibbonRenderer(effekseerRenderer->CreateRibbonRenderer());
	effekseerManager->SetRingRenderer(effekseerRenderer->CreateRingRenderer());
	effekseerManager->SetTrackRenderer(effekseerRenderer->CreateTrackRenderer());
	effekseerManager->SetModelRenderer(effekseerRenderer->CreateModelRenderer());
	// EffekseerLoderSetting
	effekseerManager->SetTextureLoader(effekseerRenderer->CreateTextureLoader());
	effekseerManager->SetModelLoader(effekseerRenderer->CreateModelLoader());
	effekseerManager->SetMaterialLoader(effekseerRenderer->CreateMaterialLoader());
	
	// Effekseer Conversion left hand coordinate system
	effekseerManager->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);
	
	// Effekseer Resource Read
	
	// Effekseer filepass conversion UTF-16 codes
	const char* smoke = u8"Data/Effect/ToonHit1.efkefc";
	char16_t utf16_smoke[256];
	Effekseer::ConvertUtf8ToUtf16(utf16_smoke, 256, smoke);

	const char* smoke2 = u8"Data/Effect/ToonHit2.efkefc";
	char16_t utf16_smoke2[256];
	Effekseer::ConvertUtf8ToUtf16(utf16_smoke2, 256, smoke2);

	const char* smoke3 = u8"Data/Effect/ToonHit3.efkefc";
	char16_t utf16_smoke3[256];
	Effekseer::ConvertUtf8ToUtf16(utf16_smoke3, 256, smoke3);

		
	const char* guard = u8"Data/Effect/guard.efkefc";
	char16_t utf16_guard[256];
	Effekseer::ConvertUtf8ToUtf16(utf16_guard, 256, guard);


	// EffekseerEffectRead
	efc_smoke = Effekseer::Effect::Create(effekseerManager, (EFK_CHAR*)utf16_smoke);
	efc_smoke2 = Effekseer::Effect::Create(effekseerManager, (EFK_CHAR*)utf16_smoke2);
	efc_smoke3 = Effekseer::Effect::Create(effekseerManager, (EFK_CHAR*)utf16_smoke3);

	efc_guard = Effekseer::Effect::Create(effekseerManager, (EFK_CHAR*)utf16_guard);
	
}

void Effect::EffectUpdate(float& elapsed_time)
{

	effekseerManager->Update(elapsed_time * 60.0f);

#ifdef USE_IMGUI
	if (framework::Instance().useimgui == false) return;
	ImGui::Begin("effec");
	ImGui::Text("effect_totalinstance:%d", effekseerManager->GetTotalInstanceCount());
	ImGui::Text("effect_restinstance:%d", effekseerManager->GetRestInstancesCount());
	ImGui::End();
#endif

	
}

Effekseer::Handle tempHandle = -1;
Effekseer::Handle Handle = -1;

Effekseer::Handle Effect::EffectLoop(Effekseer::Effect* effect, Effekseer::Vector3D efcpos)
{
	static int tenpframe = 0;
	if (tenpframe <= 0)
	{
		Handle = effekseerManager->Play(effect, efcpos);
	}

	tenpframe++;
	if (tenpframe == 30)
	{
		effekseerManager->StopEffect(tempHandle);
		tempHandle = effekseerManager->Play(effect, efcpos);

	}

	if (tenpframe >= 60)
	{
		tenpframe = 0;
		effekseerManager->StopEffect(Handle);
	}

	effekseerManager->SetLocation(Handle, efcpos);
	if (tempHandle) {
		effekseerManager->SetLocation(tempHandle, efcpos);
	}
	return Handle;
}

Effekseer::Handle Effect::EffectPlay(Effekseer::Effect* effect, Effekseer::Vector3D efcpos)
{
	auto Handle = effekseerManager->Play(effect, efcpos);
	return Handle;
}


Effekseer::Handle Effect::EffectPlay(Effekseer::Effect* effect, Effekseer::Vector3D efcpos, VECTOR3 scale)
{
	auto Handle = effekseerManager->Play(effect, efcpos);
	effekseerManager->SetScale(Handle, scale.x, scale.y, scale.z);
	return Handle;
}

void Effect::EffectRender(DirectX::XMFLOAT4X4 view, DirectX::XMMATRIX projection)
{
	effekseerRenderer->SetCameraMatrix(*reinterpret_cast<const Effekseer::Matrix44*>(&view));
	effekseerRenderer->SetProjectionMatrix(*reinterpret_cast<const Effekseer::Matrix44*>(&projection));
	// Effekseer Rendering Begin
	effekseerRenderer->BeginRendering();

	//  Effekseer Rendering
	// マネージャー単位で描画するので描画順を制御する場合はマネージャーを複数個作成し、
	// Draw()関数を実行する順序で制御できそう
	effekseerManager->Draw();

	// Effekseer Rendering End
	effekseerRenderer->EndRendering();

}

void Effect::EffectUninit()
{
	if (efc_smoke != nullptr)
	{
		efc_smoke->Release();
		efc_smoke = nullptr;
	}

	if (efc_smoke2 != nullptr)
	{
		efc_smoke2->Release();
		efc_smoke2 = nullptr;
	}

	if (efc_smoke3 != nullptr)
	{
		efc_smoke3->Release();
		efc_smoke3 = nullptr;
	}


	if (efc_guard != nullptr)
	{
		efc_guard->Release();
		efc_guard = nullptr;
	}

	if (effekseerManager != nullptr)
	{
		effekseerManager->Destroy();
		effekseerManager = nullptr;
	}
	if (effekseerRenderer != nullptr)
	{
		effekseerRenderer->Destroy();
		effekseerRenderer = nullptr;
	}

}

void EffectManager::Add()
{
	
	efcList.push_back(handcandle_effect);

	efcList.push_back(Item_effect);

	efcList.push_back(attackfire_effect);


}
void EffectManager::init(ID3D11Device* device)
{
	//Add();
	//for (auto &it : efcList)
	//{
	//	it.EffectInit(device);
	//}
}
void EffectManager::update(float& elapsed_time)
{
	//for (auto &it : efcList)
	//{
	//	it.EffectUpdate(elapsed_time);
	//}

}
void EffectManager::draw(DirectX::XMFLOAT4X4 view, DirectX::XMMATRIX projection)
{
	//for (auto &it : efcList)
	//{
	//	it.EffectRender(view, projection);
	//}

}

void EffectManager::uninit()
{
	for (auto &it : efcList)
	{
		it.EffectUninit();
	}

}
