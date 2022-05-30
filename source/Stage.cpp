#include "Stage.h"
#include "collision.h"
#include "framework.h"
static Stage* instance = nullptr;
static Wall* instancewall = nullptr;
// インスタンス取得
Stage& Stage::Instance()
{
	return *instance;
}
Stage::Stage(const char* _filename_temp)
{
	instance = this;

	ID3D11Device* device = framework::Instance().GetDevice();
	model_resource = std::make_shared<ModelResource>();
    model_resource->Init(device, _filename_temp, true);
	model = std::make_shared<Model>();
    model->Init(model_resource);
	model_renderer = std::make_shared<ModelRenderer>(device);

    angle.y = 53.4f;

}

Stage::~Stage()
{
	// ステージモデルを破棄

}

// 更新処理


void Stage::Render(ID3D11DeviceContext* dc,Shader* shader, DirectX::XMFLOAT4X4 view_projection, DirectX::XMFLOAT4X4 projection,DirectX::XMFLOAT4 light_direction, std::shared_ptr<Camera> camera, float elapsed_time, DirectX::XMFLOAT4X4 view, DirectX::XMFLOAT4 brightColor, DirectX::XMFLOAT4 darkColor)
{
    DirectX::XMMATRIX W;
    {
        DirectX::XMFLOAT3 scale(3, 3, 3);
        DirectX::XMFLOAT3 rotate(angle.x,angle.y, 0);
        DirectX::XMFLOAT3 translate(0, 0, 0);

        DirectX::XMMATRIX S, R, T;
        S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
        R = DirectX::XMMatrixRotationRollPitchYaw(rotate.x, rotate.y, rotate.z);	// ZXY回転
        T = DirectX::XMMatrixTranslation(translate.x, translate.y, translate.z);

        W = S * R * T;
    }

    DirectX::XMFLOAT4 emissionEdgeColor = { 0, 0, 0, 1 };
    DirectX::XMFLOAT4 emissionColor = { 0, 0, 0, 1 };
    //エッジのカラーを変更する
   // EdgeColorSet(invincible, emissionEdgeColor);

    model->CalculateLocalTransform();
    model->CalculateWorldTransform(W);
    model->UpdateAnimation(elapsed_time / 2);
    model_renderer->Begin(dc, shader, view_projection, W, projection,light_direction, camera,view, brightColor, darkColor);
    model_renderer->Draw(dc, model.get(), DirectX::XMFLOAT3(light_direction.x, light_direction.y, light_direction.z), color, emissionColor, emissionEdgeColor, 0.75f, 0.25f, 0.01f, FALSE, 0.0f, true, color);
    model_renderer->End(shader);
}

void Stage::Render(ID3D11DeviceContext* dc, DirectX::XMFLOAT4X4 view_projection, DirectX::XMFLOAT4X4 projection, DirectX::XMFLOAT4 light_direction, std::shared_ptr<Camera> camera, float elapsed_time, DirectX::XMFLOAT4X4 view)
{
    DirectX::XMMATRIX W;
    {
        DirectX::XMFLOAT3 scale(3, 3, 3);
        DirectX::XMFLOAT3 rotate(angle.x, angle.y, 0);
        DirectX::XMFLOAT3 translate(0, 0, 0);

        DirectX::XMMATRIX S, R, T;
        S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
        R = DirectX::XMMatrixRotationRollPitchYaw(rotate.x, rotate.y, rotate.z);	// ZXY回転
        T = DirectX::XMMatrixTranslation(translate.x, translate.y, translate.z);

        W = S * R * T;
    }

    DirectX::XMFLOAT4 emissionEdgeColor = { 0, 0, 0, 1 };
    DirectX::XMFLOAT4 emissionColor = { 0, 0, 0, 1 };
    //エッジのカラーを変更する
   // EdgeColorSet(invincible, emissionEdgeColor);

    model->CalculateLocalTransform();
    model->CalculateWorldTransform(W);
    model->UpdateAnimation(elapsed_time / 2);
    model_renderer->Begin(dc ,view_projection, W, projection, light_direction, camera, view);
    model_renderer->Draw(dc, model.get(), DirectX::XMFLOAT3(light_direction.x, light_direction.y, light_direction.z), color, emissionColor, emissionEdgeColor, 0.75f, 0.25f, 0.01f, FALSE, 0.0f, true, color);
    model_renderer->End(dc);
}

// レイキャスト
bool Stage::RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit)
{
	return Collision::IntersectRayVsModel(start, end, model.get(),hit);
}






// インスタンス取得
Wall& Wall::Instance()
{
    return *instancewall;
}
Wall::Wall(const char* _filename_temp)
{
    instancewall = this;

    ID3D11Device* device = framework::Instance().GetDevice();
    model_resource = std::make_shared<ModelResource>();
    model_resource->Init(device, _filename_temp, true);
    model = std::make_shared<Model>();
    model->Init(model_resource);
    model2 = std::make_shared<Model>();
    model2->Init(model_resource);
    model_renderer = std::make_shared<ModelRenderer>(device);

    angle.y = 0.0f;

}

Wall::~Wall()
{
    // ステージモデルを破棄

}

// 更新処理


void Wall::Render(ID3D11DeviceContext* dc, DirectX::XMFLOAT4X4 view_projection, DirectX::XMFLOAT4X4 projection, DirectX::XMFLOAT4 light_direction, std::shared_ptr<Camera> camera, float elapsed_time, DirectX::XMFLOAT4X4 view)
{
    DirectX::XMMATRIX W;
    {
        DirectX::XMFLOAT3 scale(0.1f, 0.1f, 0.1f);
        DirectX::XMFLOAT3 rotate(angle.x, angle.y, 0);
        DirectX::XMFLOAT3 translate(-200, 0, 0);

        DirectX::XMMATRIX S, R, T;
        S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
        R = DirectX::XMMatrixRotationRollPitchYaw(rotate.x, rotate.y, rotate.z);	// ZXY回転
        T = DirectX::XMMatrixTranslation(translate.x, translate.y, translate.z);

        W = S * R * T;
    }

    DirectX::XMMATRIX W2;
    {
        DirectX::XMFLOAT3 scale(0.1f, 0.1f, 0.1f);
        DirectX::XMFLOAT3 rotate(angle.x, angle.y, 0);
        DirectX::XMFLOAT3 translate(300, 0, 0);

        DirectX::XMMATRIX S, R, T;
        S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
        R = DirectX::XMMatrixRotationRollPitchYaw(rotate.x, rotate.y, rotate.z);	// ZXY回転
        T = DirectX::XMMatrixTranslation(translate.x, translate.y, translate.z);

        W = S * R * T;
    }

    model->CalculateLocalTransform();
    model->CalculateWorldTransform(W);
    model->UpdateAnimation(elapsed_time / 2);

    DirectX::XMFLOAT4 emissionEdgeColor = { 0, 0, 0, 1 };
    DirectX::XMFLOAT4 emissionColor = { 0, 0, 0, 1 };
    //エッジのカラーを変更する
   // EdgeColorSet(invincible, emissionEdgeColor);


    model2->CalculateLocalTransform();
    model2->CalculateWorldTransform(W2);
    model2->UpdateAnimation(elapsed_time / 2);

    model_renderer->Begin(dc, view_projection, W, projection, light_direction, camera,view);
    model_renderer->Draw(dc, model.get(), DirectX::XMFLOAT3(light_direction.x, light_direction.y, light_direction.z), color, emissionColor, emissionEdgeColor, 0.75f, 0.25f, 0.01f, FALSE, 0.0f, true, color);
    model_renderer->Draw(dc, model2.get(), DirectX::XMFLOAT3(light_direction.x, light_direction.y, light_direction.z), color, emissionColor, emissionEdgeColor, 0.75f, 0.25f, 0.01f, FALSE, 0.0f, true, color);
    model_renderer->End(dc);
}

// レイキャスト
bool Wall::RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit)
{
    return Collision::IntersectRayVsModel(start, end, model.get(), hit);
}