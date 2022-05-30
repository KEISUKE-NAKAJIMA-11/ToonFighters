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

	// ������
	if (my < 0.0f)
	{
		// ���C�̊J�n�ʒu�͑�����菭����
		DirectX::XMFLOAT3 start = { obj->position.x,obj->position.y + obj->stepOffset , obj->position.z };
		// ���C�̏I���ʒu�͈ړ���̈ʒu
		DirectX::XMFLOAT3 end = { obj->position.x,obj->position.y + my , obj->position.z };

		// ���C�L���X�g�ɂ��n�ʔ���
		HitResult hit;
		if (Stage::Instance().RayCast(start, end, hit))
		{
			// �n�ʂɐڒn���Ă���
			obj->position.y = hit.position.y;

			// �X�Η��̌v�Z
			float normalLengthXZ = sqrtf(hit.normal.x * hit.normal.x + hit.normal.z * hit.normal.z);
			obj->slopeRate = 1.0f - (hit.normal.y / (normalLengthXZ + hit.normal.y));

			// ���n����
			if (!obj->isGround)
			{
		
			}
			obj->isGround = true;

		
			obj->velocity.y = 0.0f;
		}
		else
		{
			// �󒆂ɕ����Ă���
			obj->position.y += my;
			//obj->isGround = false;
		}
	}
	// �㏸��
	else if (my > 0.0f)
	{
		obj->position.y += my;
		obj->isGround = false;
	}
}



void Character::UpdateHorizontalMove(float elapsedTime, OBJ3D* obj)
{
	// �������͗ʌv�Z
	float velocityLengthXZ = sqrtf(obj->velocity.x * obj->velocity.x + obj->velocity.z * obj->velocity.z);
	if (velocityLengthXZ > 0.0f)
	{
		// �����ړ��l
		float mx = obj->velocity.x * elapsedTime;
		float mz = obj->velocity.z * elapsedTime;

		// ���C�Ƃ̊J�n�ʒu�ƏI�_�ʒu
		DirectX::XMFLOAT3 start = { obj->position.x , obj->position.y + obj->stepOffset , obj->position.z };
		DirectX::XMFLOAT3 end = { obj->position.x + mx , obj->position.y + obj->stepOffset , obj->position.z + mz };

		// ���C�L���X�g�ɂ��ǔ���
		HitResult hit;
		HitResult wall;

		if (Stage::Instance().RayCast(start, end, hit))
		{
			// �ǂ܂ł̃x�N�g��
			DirectX::XMVECTOR Start = DirectX::XMLoadFloat3(&start);
			DirectX::XMVECTOR End = DirectX::XMLoadFloat3(&end);
			DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(End, Start);

			// �ǂ̖@��
			DirectX::XMVECTOR Normal = DirectX::XMLoadFloat3(&hit.normal);

			//���˃x�N�g����@���Ɏˉe
			DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(DirectX::XMVectorNegate(Vec), Normal);

			// �␳�ʒu�̌v�Z
			DirectX::XMVECTOR CollectPosition = DirectX::XMVectorMultiplyAdd(Normal, Dot, End);
			DirectX::XMFLOAT3 collectPosition;
			DirectX::XMStoreFloat3(&collectPosition, CollectPosition);

			// �ǂ�������փ��C�L���X�g
			HitResult hit2;
			if (!Stage::Instance().RayCast(hit.position, collectPosition, hit2))
			{
				// �ǂ�������ŕǂɓ�����Ȃ�������␳�ʒu�Ɉړ�
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
			// �ǂ܂ł̃x�N�g��
			DirectX::XMVECTOR Start = DirectX::XMLoadFloat3(&start);
			DirectX::XMVECTOR End = DirectX::XMLoadFloat3(&end);
			DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(End, Start);

			// �ǂ̖@��
			DirectX::XMVECTOR Normal = DirectX::XMLoadFloat3(&hit.normal);

			//���˃x�N�g����@���Ɏˉe
			DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(DirectX::XMVectorNegate(Vec), Normal);

			// �␳�ʒu�̌v�Z
			DirectX::XMVECTOR CollectPosition = DirectX::XMVectorMultiplyAdd(Normal, Dot, End);
			DirectX::XMFLOAT3 collectPosition;
			DirectX::XMStoreFloat3(&collectPosition, CollectPosition);

			// �ǂ�������փ��C�L���X�g
			HitResult hit2;
			if (!Wall::Instance().RayCast(hit.position, collectPosition, hit2))
			{
				// �ǂ�������ŕǂɓ�����Ȃ�������␳�ʒu�Ɉړ�
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
			// �ړ�
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



