#pragma once

#include "OBJ3D.h"

#include "fighter_base.h"
#include "util.h"

struct Character_para {
    //GameLib::AnimeData* aidl[6];
    //GameLib::AnimeData* woke[6];


    float accel;
    float max_speed;
    VECTOR3 last_unitvec;

    float spend_water;
    float recovery_water;
};

//Characterクラス
//重力やレイキャストなどを行う汎用クラス
//いつか作るキャラ選択の時などはfighter_adr変数で指定されたキャラのアドレスを保持しておく役割もある


class Character :public MoveAlg  {
private:
	std::unique_ptr<FighterBase> fighter_adr;

public:
    Character(std::shared_ptr<GamePad> pad,int characternum = 0)
	{
		//キャラクター増やせば追加
		switch (characternum)
		{
		case 0:
			fighter_adr = std::make_unique<FighterBase>(pad);
			break;
		}
	
	}
    virtual ~Character() {}

	// 行列更新処理
	void UpdateTransform();

	// 位置取得
	const VECTOR3& GetPosition(OBJ3D* obj) const { return obj->position; }

	// 位置設定
	void SetPosition(const VECTOR3& position, OBJ3D* obj) { obj->position = position; }

	// 回転取得
	const VECTOR3& GetAngle(OBJ3D* obj)const { return obj->angle; }

	// 回転設定
	void SetAngle(const VECTOR3& angle, OBJ3D* obj) { obj->angle = angle; }

	// スケール取得
	const VECTOR3& GetScale(OBJ3D* obj)const { return obj->scale; }

	// スケール設定
	void SetScale(const VECTOR3& scale, OBJ3D* obj) { obj->scale = scale; }

	// ダメージを与える
	bool ApplyDamage(int damage, float invincibleTime, OBJ3D* obj);

	// 地面に接地しているか
	bool IsGround(OBJ3D* obj)const { return obj->isGround; }

	// 健康状態を取得
	int GetHealth(OBJ3D* obj) const { return obj->health; };

	// 最大健康状態を取得
	int GetMaxHealth(OBJ3D* obj) const { return obj->maxHealth; }

	void SetHealth(const int health, OBJ3D* obj) { obj->health = health; }

	void move(OBJ3D* obj)override;

protected:
    Character_para act_para;

	

	void UpdateVerticalVelocity(float elapsedFrame, OBJ3D* obj);

	void UpdateVerticalMove(float elapsedTime, OBJ3D* obj);

	void UpdateHorizontalMove(float elapsedTime, OBJ3D* obj);

	// ダメージを受けた時に呼ばれる
	virtual void OnDamaged() {}
 
};


class Test :public MoveAlg
{
	void move(OBJ3D* obj)override;
};