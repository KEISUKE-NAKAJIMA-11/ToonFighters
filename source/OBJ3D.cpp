#include "OBJ3D.h"
#include "2d_primitive.h"
#include "framework.h"
#include "judge_manager.h"


OBJ3D::OBJ3D()
{
    //Model = std::make_unique<Model>(model_resource);
    //Model = new Model(model_resource);
    //Model->GetNodes()
}
void OBJ3D::Clear()
{
    MoveAlg* movealg = nullptr;

    DirectX::XMFLOAT3 position = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 speed = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 scale = { 1.0f,1.0f,1.0f };
    DirectX::XMFLOAT3 angle = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT4 color = { 0.0f,0.0f,0.0f,0.0f };

    int state = 0;
    int timer = 0;
    bool exist = true;

}
void OBJ3D::Update()
{
    movealg->move(this);
}
void OBJ3D::Draw()
{
    //OBJManagerでやる
}

void OBJManager::update(DirectX::XMFLOAT4X4 view, DirectX::XMFLOAT4X4 projection)
{
    for (auto& it : objList)
    {

        //スクリーン座標更新
        judge_manager::Instance().WorldToScreen(&it->screen_position, it->position, view, projection);
#ifndef  BURUBON
        // 当たり判定作成用のパーツ毎位置更新
  /*      DirectX::XMFLOAT3 head = it->model->GetBonePosition("J_L_HairFront_01");
        DirectX::XMFLOAT3 body   = it->model->GetBonePosition("Character1_Spine");
        DirectX::XMFLOAT3 reg    = it->model->GetBonePosition("Character1_LeftToeBase");*/

 
        DirectX::XMFLOAT3 rootpos = it->model->GetBonePosition("J_Mune_root_00");
        it->root = VECTOR3(rootpos.x, rootpos.y, rootpos.z);
        it->head = DirectX::XMFLOAT3(it->position.x, it->position.y + 120, it->position.z);
        it->body = DirectX::XMFLOAT3(it->position.x, it->position.y + 90, it->position.z);
        it->reg = it->position;

        it->view = view;
        this->view = view;

        it->projection = projection;

        judge_manager::Instance().WorldToScreen(&it->head_position, VECTOR3(it->head.x, it->head.y, it->head.z), view, projection);
        judge_manager::Instance().WorldToScreen(&it->body_position, VECTOR3(it->body.x, it->body.y, it->body.z), view, projection);
        judge_manager::Instance().WorldToScreen(&it->reg_position, VECTOR3(it->reg.x, it->reg.y, it->reg.z), view, projection);







#endif //  BURUBON

        it->Update();
    }
    objList.remove_if([](std::shared_ptr<OBJ3D> obj)
    {

        return obj->exist == false; 
    }
    );

}

void OBJManager::draw(ID3D11DeviceContext* immediate_context,  DirectX::XMFLOAT4X4 view_projection, DirectX::XMFLOAT4X4 projection, DirectX::XMFLOAT4 light_direction, std::shared_ptr<Camera> camera, float elapsed_time)
{
    for (auto& it : objList)
    {
        DirectX::XMMATRIX W;
        {
            DirectX::XMFLOAT3 scale(it->scale.x, it->scale.y, it->scale.z);
            DirectX::XMFLOAT3 rotate(it->angle.x, it->angle.y, it->angle.z);
            DirectX::XMFLOAT3 translate(it->position.x, it->position.y, it->position.z);

            DirectX::XMMATRIX S, R, T;
            S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
            R = DirectX::XMMatrixRotationRollPitchYaw(rotate.x, rotate.y, rotate.z);	// ZXY回転
            T = DirectX::XMMatrixTranslation(translate.x, translate.y, translate.z);

            W = S * R * T;
        }



        DirectX::XMMATRIX WB;
        {
            DirectX::XMFLOAT3 scale(0.1f, 0.1f, 0.1f);
            DirectX::XMFLOAT3 rotate(0.0f, 0.0f, 0.0f);
            DirectX::XMFLOAT3 translate(0.0f, 0.0f, 0.0f);

            DirectX::XMMATRIX S, R, T;
            S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
            R = DirectX::XMMatrixRotationRollPitchYaw(rotate.x, rotate.y, rotate.z);	// ZXY回転
            T = DirectX::XMMatrixTranslation(translate.x, translate.y, translate.z);

            WB = S * R * T;
        }
      

        it->model->CalculateLocalTransform();
        it->model->CalculateWorldTransform(W);

        if (it->scale.x < 0)
        {
            it->model_renderer->culling = 1;
        }
        else if (it->scale.x > 0)
        {
            it->model_renderer->culling = 0;
        }


        if (it->hitstop_timer < it->hitstop_limit)
        {
            elapsed_time = 0.1f;
         
        }


        it->model->UpdateAnimation(elapsed_time);


        //エッジのカラーを変更する
       // EdgeColorSet(invincible, emissionEdgeColor);



        it->model_renderer->Begin(immediate_context, view_projection,W,projection, light_direction,camera,view);
        if (it->tag == "PLAYER1")
        {
            it->model_renderer->Draw(immediate_context, it->model.get(),DirectX::XMFLOAT3(light_direction.x, light_direction.y, light_direction.z),it->color, it->emissionColor, it->emissionEdgeColor,0.75f, 0.25f, 0.01f, FALSE, 0.0f, true, it->color);
        }

        else if (it->tag == "PLAYER2")
        {
            it->model_renderer->Draw(immediate_context, it->model.get(), DirectX::XMFLOAT3(light_direction.x, light_direction.y, light_direction.z), it->color, it->emissionColor, it->emissionEdgeColor, 0.75f, 0.25f, 0.01f, FALSE, 0.1f, true, it->color);
        }
      
        it->model_renderer->End(immediate_context);

        //// 衝突判定用デバッグ球を描画

        //for (int i = 0; i < it->model->bonepos.size(); i++)
        //{
        //    it->debugRenderer->DrawSphere(it->model->bonepos.at(i), 1.0f, DirectX::XMFLOAT4(0.5, 0, 0, 0));
        //}
    
        //it->debugRenderer->Render(immediate_context,view_projection);


    }

}

void OBJManager::draw(ID3D11DeviceContext* immediate_context, Shader* shader, DirectX::XMFLOAT4X4 view_projection, DirectX::XMFLOAT4X4 projection, DirectX::XMFLOAT4 light_direction, std::shared_ptr<Camera> camera, float elapsed_time,DirectX::XMFLOAT4 brightColor, DirectX::XMFLOAT4 darkColor,float edgeThershood,float toonThereshood,float shadowbias)
{
    for (auto& it : objList)
    {
        DirectX::XMMATRIX W;
        {
            DirectX::XMFLOAT3 scale(it->scale.x, it->scale.y, it->scale.z);
            DirectX::XMFLOAT3 rotate(it->angle.x, it->angle.y, it->angle.z);
            DirectX::XMFLOAT3 translate(it->position.x, it->position.y, it->position.z);

            DirectX::XMMATRIX S, R, T;
            S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
            R = DirectX::XMMatrixRotationRollPitchYaw(rotate.x, rotate.y, rotate.z);	// ZXY回転
            T = DirectX::XMMatrixTranslation(translate.x, translate.y, translate.z);

            W = S * R * T;
        }



        DirectX::XMMATRIX WB;
        {
            DirectX::XMFLOAT3 scale(0.1f, 0.1f, 0.1f);
            DirectX::XMFLOAT3 rotate(0.0f, 0.0f, 0.0f);
            DirectX::XMFLOAT3 translate(0.0f, 0.0f, 0.0f);

            DirectX::XMMATRIX S, R, T;
            S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
            R = DirectX::XMMatrixRotationRollPitchYaw(rotate.x, rotate.y, rotate.z);	// ZXY回転
            T = DirectX::XMMatrixTranslation(translate.x, translate.y, translate.z);

            WB = S * R * T;
        }


        it->model->CalculateLocalTransform();
        it->model->CalculateWorldTransform(W);

        if (it->scale.x < 0)
        {
            it->model_renderer->culling = 1;
        }
        else if (it->scale.x > 0)
        {
            it->model_renderer->culling = 0;
        }


        if (it->hitstop_timer < it->hitstop_limit)
        {
            elapsed_time = 0.1f;

        }


        it->model->UpdateAnimation(elapsed_time);


        DirectX::XMFLOAT4 emissionEdgeColor = { 0, 0, 0, 1 };
        DirectX::XMFLOAT4 emissionColor = { 0, 0, 0, 1 };


        //エッジのカラーを変更する
       // EdgeColorSet(invincible, emissionEdgeColor);


        it->model_renderer->Begin(immediate_context,shader, view_projection, W, projection, light_direction, camera, view, brightColor,darkColor);
        if (it->tag == "PLAYER1")
        {
            it->model_renderer->Draw(immediate_context, it->model.get(), DirectX::XMFLOAT3(light_direction.x, light_direction.y, light_direction.z), it->color, emissionColor, emissionEdgeColor, 0.75f, 0.25f, 0.01f, FALSE, 0.0f, true, it->color);
        }

        else if (it->tag == "PLAYER2")
        {
            it->model_renderer->Draw(immediate_context, it->model.get(), DirectX::XMFLOAT3(light_direction.x, light_direction.y, light_direction.z), it->color,
                emissionColor, emissionEdgeColor, edgeThershood, toonThereshood, shadowbias, FALSE, 0.1f, true, it->color
            
            
            );
        }



        it->model_renderer->End(shader);

        //// 衝突判定用デバッグ球を描画

        //for (int i = 0; i < it->model->bonepos.size(); i++)
        //{
        //    it->debugRenderer->DrawSphere(it->model->bonepos.at(i), 1.0f, DirectX::XMFLOAT4(0.5, 0, 0, 0));
        //}

        //it->debugRenderer->Render(immediate_context,view_projection);


    }


}


void OBJManager::uninit()
{
    objList.clear();
    objects.clear();
}

std::shared_ptr<OBJ3D> OBJManager::Add(ID3D11Device* device, MoveAlg* movealg, VECTOR3 pos, const char* filename, std::string tag, bool has_fbm = true, bool has_blade = false)
{
    //OBJ3D* obj = new OBJ3D();
    std::shared_ptr<OBJ3D> obj = std::make_unique<OBJ3D>();
    obj->Clear();
    obj->movealg = movealg;
    obj->exist = true;
    obj->position = pos;
    obj->scale = { 1.0f,1.0f,1.0f };
    obj->filename_temp = filename;
    for (auto &it : objList)
    {
        if (filename == it->filename_temp) 
        {
            obj->model = std::make_shared<Model>(*it->model);
            obj->model_renderer = it->model_renderer;
        }
    }
    if (!obj->model)
    {

        obj->model_resource = std::make_shared<ModelResource>();
        obj->model_resource->Init(device,filename,true,has_fbm);
        obj->model = std::make_shared<Model>();
        obj->model->Init(obj->model_resource);
        obj->model_renderer = std::make_shared<ModelRenderer>(device);

        if (has_blade)
        {
            obj->blade_model_resource = std::make_shared<ModelResource>();
            obj->blade_model_resource->Init(device, "./Data/FBX/helmet/helmet2.fbx", true, false);
            obj->blade_model = std::make_shared<Model>();
            obj->blade_model->Init(obj->blade_model_resource);
        }

    }


    obj->debugRenderer = std::make_unique<DebugRenderer>(device);

    obj->tag = tag;

  

    objList.emplace_back(obj);

   // objects.insert(std::make_pair(tag, obj));

    return *objList.rbegin();

}




