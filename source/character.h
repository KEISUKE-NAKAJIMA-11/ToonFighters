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

//Character�N���X
//�d�͂⃌�C�L���X�g�Ȃǂ��s���ėp�N���X
//�������L�����I���̎��Ȃǂ�fighter_adr�ϐ��Ŏw�肳�ꂽ�L�����̃A�h���X��ێ����Ă�������������


class Character :public MoveAlg  {
private:
	std::unique_ptr<FighterBase> fighter_adr;

public:
    Character(std::shared_ptr<GamePad> pad,int characternum = 0)
	{
		//�L�����N�^�[���₹�Βǉ�
		switch (characternum)
		{
		case 0:
			fighter_adr = std::make_unique<FighterBase>(pad);
			break;
		}
	
	}
    virtual ~Character() {}

	// �s��X�V����
	void UpdateTransform();

	// �ʒu�擾
	const VECTOR3& GetPosition(OBJ3D* obj) const { return obj->position; }

	// �ʒu�ݒ�
	void SetPosition(const VECTOR3& position, OBJ3D* obj) { obj->position = position; }

	// ��]�擾
	const VECTOR3& GetAngle(OBJ3D* obj)const { return obj->angle; }

	// ��]�ݒ�
	void SetAngle(const VECTOR3& angle, OBJ3D* obj) { obj->angle = angle; }

	// �X�P�[���擾
	const VECTOR3& GetScale(OBJ3D* obj)const { return obj->scale; }

	// �X�P�[���ݒ�
	void SetScale(const VECTOR3& scale, OBJ3D* obj) { obj->scale = scale; }

	// �_���[�W��^����
	bool ApplyDamage(int damage, float invincibleTime, OBJ3D* obj);

	// �n�ʂɐڒn���Ă��邩
	bool IsGround(OBJ3D* obj)const { return obj->isGround; }

	// ���N��Ԃ��擾
	int GetHealth(OBJ3D* obj) const { return obj->health; };

	// �ő匒�N��Ԃ��擾
	int GetMaxHealth(OBJ3D* obj) const { return obj->maxHealth; }

	void SetHealth(const int health, OBJ3D* obj) { obj->health = health; }

	void move(OBJ3D* obj)override;

protected:
    Character_para act_para;

	

	void UpdateVerticalVelocity(float elapsedFrame, OBJ3D* obj);

	void UpdateVerticalMove(float elapsedTime, OBJ3D* obj);

	void UpdateHorizontalMove(float elapsedTime, OBJ3D* obj);

	// �_���[�W���󂯂����ɌĂ΂��
	virtual void OnDamaged() {}
 
};


class Test :public MoveAlg
{
	void move(OBJ3D* obj)override;
};