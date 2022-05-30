#pragma once
#pragma once

#include "Actor.h"

// �R���|�[�l���g
class Component
{
public:
	Component() {}
	virtual ~Component() {}

	// ���O�擾
	virtual const char* GetName() const = 0;

	// �J�n����
	virtual void Start() {}

	// �X�V����
	virtual void Update(float elapsedTime) {}

	// GUI�`��
	virtual void OnGUI() {}

	// �A�N�^�[�ݒ�
	void SetActor(std::shared_ptr<Actor> actor) { this->actor = actor; }

	// �A�N�^�[�擾
	std::shared_ptr<Actor> GetActor() { return actor.lock(); }

private:
	std::weak_ptr<Actor>	actor;

};