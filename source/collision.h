#pragma once

#include	<DirectXMath.h>
#include "OBJ3D.h"

/*******************************************************************************
	衝突判定用クラス
*******************************************************************************/

// レイキャストヒット結果
struct HitResult
{
	DirectX::XMFLOAT3	position = { 0.0f,0.0f,0.0f };
	DirectX::XMFLOAT3	normal = { 0.0f,0.0f,0.0f };
	float				distance = 0.0f;
	int					materialIndex = -1;
};


class Collision
{
public:
    inline static bool	HitSphere(const DirectX::XMFLOAT3& p1, float r1, const DirectX::XMFLOAT3& p2, float r2);

	// レイとモデルの交差判定
	static bool IntersectRayVsModel(
		const DirectX::XMFLOAT3& start,
		const DirectX::XMFLOAT3& end,
		const Model* model,
		HitResult& result
	);

   static int IntersectRayVsModelX(const DirectX::XMVECTOR& Start, const DirectX::XMVECTOR& Dir, float neart, const ModelResource::s_mesh& mesh, DirectX::XMVECTOR& hitPosition, DirectX::XMVECTOR& hitNormal);

   static int IntersectRayVsModelY(const DirectX::XMVECTOR& Start, const DirectX::XMVECTOR& Dir, float neart, const ModelResource::s_mesh& mesh, DirectX::XMVECTOR& hitPosition, DirectX::XMVECTOR& hitNormal);
 
   static int IntersectRayVsModelZ(const DirectX::XMVECTOR& Start, const DirectX::XMVECTOR& Dir, float neart, const ModelResource::s_mesh& mesh, DirectX::XMVECTOR& hitPosition, DirectX::XMVECTOR& hitNormal);

   static int IntersectRayVsModel(const DirectX::XMVECTOR& Start, const DirectX::XMVECTOR& Dir, float neart, const ModelResource::s_mesh& mesh, DirectX::XMVECTOR& hitPosition, DirectX::XMVECTOR& hitNormal);

   

 //  矩形と矩形のあたり判定

 inline  static bool rectHitCheck(const fRECT& rect1, const fRECT& rect2)
    {
        // 条件を満たさなければすぐリターンさせる（早期リターン）
        if (rect1.left > rect2.right)  return false;
        if (rect1.right < rect2.left)   return false;
        if (rect1.top > rect2.bottom) return false;
        if (rect1.bottom < rect2.top)    return false;

        return true;    // あたっている
    }


};










class attack_range
{
public:

    fRECT _attack_range = {};                          // 攻撃判定用　敵のくらい判定とコリジョンマネージャーで判定する

    float a_left;
    float a_top;
    float a_right;
    float a_bottom;

    float left;
    float right;

    float attenuation_rate = 1.0f;
    int attack_life_time = 0;                         //攻撃持続フレーム
    int owner_tag = 0;                                    //攻撃をしたキャラの名前
    int attack_standby_frame = 0;
    unsigned int  attack_types;

    bool air = false;

    int hitcount;
    bool trigger = true;//当たり判定トリガー
    std::weak_ptr<OBJ3D> owner;
    int damage = 0;

    float stanpoint = 0.0f;
    
    float hitstop = 0;

    bool isPursuit = true;

    VECTOR2 vector;
    attack_range(){};
    ~attack_range(){};



};

class atack_manager
{
    
public:


    std::list<std::shared_ptr<attack_range>> attklist;

    void init() 
    {
        attklist.clear();
    }

    void update();

    std::shared_ptr<attack_range> add_attack_range(OBJ3D* owner, attack_range* atkinfo);
 

    //std::shared_ptr<attack_range> add_attack_range(OBJ3D* owner,float left, float top, float right, float bottom,
    //    int attack_life_time, int _owner_tag, unsigned int _attack_types,
    //    int hitcount, VECTOR2 vector/*吹っ飛びベクトル*/, int stundbyframe/*発生フレーム*/, int damage = 0);

    //std::shared_ptr<attack_range> add_attack_range(OBJ3D* owner, attack_range atkinfo);

    //std::shared_ptr<attack_range> add_attack_range(OBJ3D* owner, attack_range* atkinfo);
   
    //void draw();//デバッグ

   
   
};


