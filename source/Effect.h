#pragma once
#include <Effekseer.h>
#include <EffekseerRendererDX11.h>
#include <DirectXMath.h>
#include <map>
class Effect
{
public:

    Effekseer::Manager* effekseerManager = nullptr;
    EffekseerRenderer::Renderer* effekseerRenderer = nullptr;

    Effekseer::Effect* efc_smoke = nullptr;
    Effekseer::Effect* efc_smoke2 = nullptr;
    Effekseer::Effect* efc_smoke3 = nullptr;
    Effekseer::Effect* efc_guard = nullptr;


    void EffectInit(ID3D11Device* device);
    void EffectUpdate(float& elapsed_time);
    void EffectRender(DirectX::XMFLOAT4X4 view, DirectX::XMMATRIX projection);
    void EffectUninit();

    Effekseer::Handle EffectPlay(Effekseer::Effect* effect, Effekseer::Vector3D efcpos);//àÍâÒÇæÇØ
    Effekseer::Handle EffectPlay(Effekseer::Effect* effect, Effekseer::Vector3D efcpos, VECTOR3 scale);
    Effekseer::Handle EffectLoop(Effekseer::Effect* effect, Effekseer::Vector3D efcpos);//ÉãÅ[Év
};

extern Effect effect;

class EffectManager
{
public:
    Effect handcandle_effect;
    Effect Item_effect;
    Effect attackfire_effect;

    std::vector<Effect> efcList;
    void Add();
    void init(ID3D11Device* device);
    void update(float& elapsed_time);
    void draw(DirectX::XMFLOAT4X4 view, DirectX::XMMATRIX projection);
    void uninit();
    std::vector<Effect> getlist() { return efcList; }
};
