#include "character.h"
#include "collision.h"
#include "Stage.h"
#include "framework.h"

void Character::UpdateTransform()
{


}

bool Character::ApplyDamage(int damage, float invincibleTime, OBJ3D* obj)
{
	return false;
}

void Character::move(OBJ3D* obj)
{
	float delta_time = framework::Instance().elapased_time;


	if (!framework::Instance().dulation)
	{
		fighter_adr->move(obj);
		UpdateVerticalVelocity(delta_time, obj);
		UpdateVerticalMove(delta_time, obj);
		UpdateHorizontalMove(delta_time, obj);	
	}


	if (obj->hitstop_timer < obj->hitstop_limit)
	{
		obj->hitstop_timer++;
	}

	
}



void Character::UpdateVerticalVelocity(float elapsedFrame, OBJ3D* obj)
{
	const float GRAVITY_MAX = 700.0f;
	if (fighter_adr->state == CHARASTATE::STATE_DAMAGE|| fighter_adr->state == CHARASTATE::STATE_DOWN)
	{
		obj->velocity.y += (obj->gravity / 2) * elapsedFrame;
	}
	else
	{
		obj->velocity.y += obj->gravity * elapsedFrame;
	}


	if (obj->velocity.y <= -GRAVITY_MAX)
	{
		obj->velocity.y = -GRAVITY_MAX ;
	}



}

void Character::UpdateVerticalMove(float elapsedTime, OBJ3D* obj)
{
	float my = obj->velocity.y * elapsedTime;
	obj->slopeRate = 0.0f;

	// 落下中
	if (my < 0.0f)
	{
		// レイの開始位置は足元より少し上
		DirectX::XMFLOAT3 start = { obj->position.x,obj->position.y + obj->stepOffset , obj->position.z };
		// レイの終了位置は移動後の位置
		DirectX::XMFLOAT3 end = { obj->position.x,obj->position.y + my , obj->position.z };

		// レイキャストによる地面判定
		HitResult hit;
		if (Stage::Instance().RayCast(start, end, hit))
		{
			// 地面に接地している
			obj->position.y = hit.position.y;

			// 傾斜率の計算
			float normalLengthXZ = sqrtf(hit.normal.x * hit.normal.x + hit.normal.z * hit.normal.z);
			obj->slopeRate = 1.0f - (hit.normal.y / (normalLengthXZ + hit.normal.y));

			// 着地した
			if (!obj->isGround)
			{
		
			}
			obj->isGround = true;

		
			obj->velocity.y = 0.0f;
		}
		else
		{
			// 空中に浮いている
			obj->position.y += my;
			//obj->isGround = false;
		}
	}
	// 上昇中
	else if (my > 0.0f)
	{
		obj->position.y += my;
		obj->isGround = false;
	}
}



void Character::UpdateHorizontalMove(float elapsedTime, OBJ3D* obj)
{
	// 水平速力量計算
	float velocityLengthXZ = sqrtf(obj->velocity.x * obj->velocity.x + obj->velocity.z * obj->velocity.z);
	if (velocityLengthXZ > 0.0f)
	{
		// 水平移動値
		float mx = obj->velocity.x * elapsedTime;
		float mz = obj->velocity.z * elapsedTime;

		// レイとの開始位置と終点位置
		DirectX::XMFLOAT3 start = { obj->position.x , obj->position.y + obj->stepOffset , obj->position.z };
		DirectX::XMFLOAT3 end = { obj->position.x + mx , obj->position.y + obj->stepOffset , obj->position.z + mz };

		// レイキャストによる壁判定
		HitResult hit;
		HitResult wall;

		if (Stage::Instance().RayCast(start, end, hit))
		{
			// 壁までのベクトル
			DirectX::XMVECTOR Start = DirectX::XMLoadFloat3(&start);
			DirectX::XMVECTOR End = DirectX::XMLoadFloat3(&end);
			DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(End, Start);

			// 壁の法線
			DirectX::XMVECTOR Normal = DirectX::XMLoadFloat3(&hit.normal);

			//入射ベクトルを法線に射影
			DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(DirectX::XMVectorNegate(Vec), Normal);

			// 補正位置の計算
			DirectX::XMVECTOR CollectPosition = DirectX::XMVectorMultiplyAdd(Normal, Dot, End);
			DirectX::XMFLOAT3 collectPosition;
			DirectX::XMStoreFloat3(&collectPosition, CollectPosition);

			// 壁ずり方向へレイキャスト
			HitResult hit2;
			if (!Stage::Instance().RayCast(hit.position, collectPosition, hit2))
			{
				// 壁ずり方向で壁に当たらなかったら補正位置に移動
				obj->position.x = collectPosition.x;
				obj->position.z = collectPosition.z;
			}
			else
			{
				obj->position.x = hit2.position.x;
				obj->position.z = hit2.position.z;
			}
		}
	
		else if (Wall::Instance().RayCast(start, end, wall))
		{
			// 壁までのベクトル
			DirectX::XMVECTOR Start = DirectX::XMLoadFloat3(&start);
			DirectX::XMVECTOR End = DirectX::XMLoadFloat3(&end);
			DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(End, Start);

			// 壁の法線
			DirectX::XMVECTOR Normal = DirectX::XMLoadFloat3(&hit.normal);

			//入射ベクトルを法線に射影
			DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(DirectX::XMVectorNegate(Vec), Normal);

			// 補正位置の計算
			DirectX::XMVECTOR CollectPosition = DirectX::XMVectorMultiplyAdd(Normal, Dot, End);
			DirectX::XMFLOAT3 collectPosition;
			DirectX::XMStoreFloat3(&collectPosition, CollectPosition);

			// 壁ずり方向へレイキャスト
			HitResult hit2;
			if (!Wall::Instance().RayCast(hit.position, collectPosition, hit2))
			{
				// 壁ずり方向で壁に当たらなかったら補正位置に移動
				obj->position.x = collectPosition.x;
				obj->position.z = collectPosition.z;
			}
			else
			{
				obj->position.x = hit2.position.x;
				obj->position.z = hit2.position.z;
			}
		}

		else
		{
			// 移動
			obj->position.x += mx;
			obj->position.z += mz;
		}

	}
}

void Test::move(OBJ3D* obj)
{
	obj->scale = VECTOR3(0.2f,0.2f,0.2f);

	//ImGui::DragFloat("angley::%f", &obj->angle.y);
	//ImGui::DragFloat("posz::%f", &obj->position.z);
	//ImGui::DragFloat("posx::%f", &obj->position.x);
}



