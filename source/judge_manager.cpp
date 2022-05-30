#include "judge_manager.h"
#include "view_seting_2d.h"
#include "2d_primitive.h"
#include "framework.h"
#include "obj2d_data.h"

extern bool ishitrect;

void judge_manager::update(OBJManager* object1, OBJManager* object2)
{
    auto obj1 = object1->getList().begin()->get();//1pの取得
    auto obj2 = object2->getList().begin()->get();//2pの取得

	

    distance1p = (obj2->position.x - obj1->position.x);
    distance2p = (obj1->position.x - obj2->position.x);

	hit1p = obj1->ishit;
	hit2p = obj2->ishit;
	
	if (distance1p<0)
	{
		distance1p = -1;
	}
	else
	{
		distance1p = 1;
	}
	

	if (distance2p < 0)
	{
		distance2p = -1;
	}
	else
	{
		distance2p = 1;
	}


	collision_update(object1, object2);

	_attack_manager->update();

}

void judge_manager::collision_update(OBJManager* object1, OBJManager* object2)//当たり判定の更新処理
{

	auto obj1 = object1->getList().begin()->get();
	auto obj2 = object2->getList().begin()->get();


	///くらい判定更新
	obj1->hit_rect[character_part::HEAD] =
	{
		obj1->head_position.x - obj1->head_size.x,
		obj1->head_position.y - obj1->head_size.y,
		obj1->head_position.x + obj1->head_size.x,
		obj1->head_position.y
	};


	obj1->hit_rect[character_part::BODY] =
	{
		obj1->body_position.x - obj1->body_size.x,
		obj1->body_position.y - obj1->body_size.y,
		obj1->body_position.x + obj1->body_size.x,
		obj1->body_position.y
	};


	obj1->hit_rect[character_part::REG] =
	{
		obj1->reg_position.x - obj1->reg_size.x,
		obj1->reg_position.y - obj1->reg_size.y,
		obj1->reg_position.x + obj1->reg_size.x,
		obj1->reg_position.y
	};


	obj1->extrude_rect =
	{
		obj1->screen_position.x - obj1->size.x,
		obj1->screen_position.y - obj1->size.y,
		obj1->screen_position.x + obj1->size.x,
		obj1->screen_position.y
	};

	obj1->extrude_rect_center =
	{
		obj1->screen_position.x - obj1->size.x/5.0f,
		obj1->screen_position.y - obj1->size.y,
		obj1->screen_position.x + obj1->size.x/5.0f,
		obj1->screen_position.y
	};


	obj1->guard_rect =
	{
		(obj1->screen_position.x - obj1->size.x*3),
		(obj1->screen_position.y - obj1->size.y*2),
		(obj1->screen_position.x + obj1->size.x*3),
		obj1->screen_position.y
	};



	obj1->areacheck_rect =
	{
		obj1->screen_position.x - obj1->checksize.x,
		obj1->screen_position.y - obj1->checksize.y,
		obj1->screen_position.x + obj1->checksize.x,
		obj1->screen_position.y
	};

	obj2->hit_rect[character_part::HEAD] =
	{
		obj2->head_position.x - obj2->head_size.x,
		obj2->head_position.y - obj2->head_size.y,
		obj2->head_position.x + obj2->head_size.x,
		obj2->head_position.y
	};


	obj2->hit_rect[character_part::BODY] =
	{
		obj2->body_position.x - obj2->body_size.x,
		obj2->body_position.y - obj2->body_size.y,
		obj2->body_position.x + obj2->body_size.x,
		obj2->body_position.y
	};


	obj2->hit_rect[character_part::REG] =
	{
		obj2->reg_position.x - obj2->reg_size.x,
		obj2->reg_position.y - obj2->reg_size.y,
		obj2->reg_position.x + obj2->reg_size.x,
		obj2->reg_position.y
	};



	obj2->extrude_rect =
	{
		obj2->screen_position.x - obj2->size.x,
		obj2->screen_position.y - obj2->size.y,
		obj2->screen_position.x + obj2->size.x,
		obj2->screen_position.y
	};

	obj2->extrude_rect_center =
	{
		obj2->screen_position.x - obj2->size.x/5.0f,
		obj2->screen_position.y - obj2->size.y,
		obj2->screen_position.x + obj2->size.x/5.0f,
		obj2->screen_position.y
	};

	obj2->areacheck_rect =
	{
		obj2->screen_position.x - obj2->checksize.x,
		obj2->screen_position.y - obj2->checksize.y,
		obj2->screen_position.x + obj2->checksize.x,
		obj2->screen_position.y
	};


	obj2->guard_rect =
	{
		(obj2->screen_position.x - obj2->size.x * 3),
		(obj2->screen_position.y - obj2->size.y * 2),
		(obj2->screen_position.x + obj2->size.x * 3),
		 obj2->screen_position.y
	};


	extrude_update(object1, object2);

	player_collision_update(object1,UI_Manager::Instance().combo[1]);

	player_collision_update(object2, UI_Manager::Instance().combo[0]);



}

//押し戻し修正前
void judge_manager::player1_collision_update(OBJManager* object1, OBJManager* object2)
{
	auto obj1 = object1->getList().begin()->get();

	float delta_time = framework::Instance().elapased_time;

	

	for (auto obj = _attack_manager->attklist.begin(); obj != _attack_manager->attklist.end(); ++obj)
	{



		if (!obj->get()->trigger)continue;
		if (obj->get()->owner_tag == obj1->player_num)continue;
		if (obj1->is_invisible)continue;
		if (!obj1->is_guard)
		{
			if (Collision::rectHitCheck(obj->get()->_attack_range, obj1->hit_rect[character_part::HEAD]))
			{
				obj1->ishit = true;
				UI_Manager::Instance().Combo_TimerReset2p();

				UI_Manager::Instance().combo[1]++;

				obj->get()->hitcount--;
				obj1->velocity = obj->get()->vector;
				obj1->stanpoint += obj->get()->stanpoint;

				if (!obj1->isGround)
				{
					obj1->velocity.y += COLLECTIONAIR - (UI_Manager::Instance().combo[1] * obj->get()->attenuation_rate);

					if (COLLECTIONAIR <= 0)
					{
						COLLECTIONAIR = 0;
					}

				}
			

			/*	if (obj->get()->hitcount >= 0)
				{*/
					obj->get()->trigger = false;
					obj1->health -= obj->get()->damage;

					Effekseer::Vector3D pos = { obj1->position.x,obj1->position.y + 30,obj1->position.z };
					
					effect.EffectPlay(effect.efc_smoke, pos, VECTOR3(5.0f, 5.0f, 5.0f));
				//}

			}

			else if (Collision::rectHitCheck(obj1->hit_rect[character_part::BODY], obj->get()->_attack_range))
			{
				obj1->ishit = true;
				obj->get()->hitcount--;
				obj1->velocity = obj->get()->vector;
				
				UI_Manager::Instance().Combo_TimerReset2p();

				obj1->stanpoint += obj->get()->stanpoint;

				UI_Manager::Instance().combo[1]++;
				if (!obj1->isGround)
				{
					obj1->velocity.y += COLLECTIONAIR - (UI_Manager::Instance().combo[1] * obj->get()->attenuation_rate);

					if (COLLECTIONAIR <= 0)
					{
						COLLECTIONAIR = 0;
					}

				}
				//if (obj->get()->hitcount >= 0)
				{
					obj->get()->trigger = false;
					obj1->health -= obj->get()->damage;


					Effekseer::Vector3D pos = { obj1->position.x,obj1->position.y + 20,obj1->position.z };

					effect.EffectPlay(effect.efc_smoke2, pos, VECTOR3(5.0f, 5.0f, 5.0f));

				}

			}
			else if (Collision::rectHitCheck(obj->get()->_attack_range, obj1->hit_rect[character_part::REG]))
			{
				obj1->ishit = true;
				obj->get()->hitcount--;
				obj1->velocity = obj->get()->vector;

				obj1->stanpoint += obj->get()->stanpoint;
				UI_Manager::Instance().Combo_TimerReset2p();
				UI_Manager::Instance().combo[1]++;
			
				if (!obj1->isGround)
				{
					obj1->velocity.y += COLLECTIONAIR - (UI_Manager::Instance().combo[1] * obj->get()->attenuation_rate);

					if (COLLECTIONAIR <= 0)
					{
					  COLLECTIONAIR = 0;
					}
				}
				//if (obj->get()->hitcount >= 0)
				{
					obj->get()->trigger = false;
					obj1->health -= obj->get()->damage;
					Effekseer::Vector3D pos = { obj1->position.x,obj1->position.y + 20,obj1->position.z };

					effect.EffectPlay(effect.efc_smoke3, pos, VECTOR3(5.0f, 5.0f, 5.0f));

				}

			}

		}


		else 
		{
			if (Collision::rectHitCheck(obj->get()->_attack_range, obj1->hit_rect[character_part::HEAD]))
			{
			
				if (obj1->guard_elements & obj->get()->attack_types)
				{
					obj1->success_guard = true;
				}

				else 	obj1->success_guard = false;

			
			
				obj->get()->hitcount--;

				if (obj1->success_guard)
				{
					obj1->velocity = obj->get()->vector * attenuation_rate;
					obj1->velocity.y = 0.0f;


					Effekseer::Vector3D pos;
					if (get_1pdistance() >= 0)
					{
						pos = { obj1->position.x + 30,obj1->position.y,obj1->position.z };
					}

					else
					{
						pos = { obj1->position.x - 30,obj1->position.y,obj1->position.z };
					}

					if (obj1->stanpoint < MAX_STANPOINT)
					{
						obj1->stanpoint += obj->get()->stanpoint / 2.0f;
					}
				

					effect.EffectPlay(effect.efc_guard, pos, VECTOR3(5.0f, 5.0f, 5.0f));

					//if (obj->get()->hitcount >= 0)
					{
						obj->get()->trigger = false;

					}
				}

				else
				{
					obj1->velocity = obj->get()->vector;
					obj1->ishit = true;
					UI_Manager::Instance().Combo_TimerReset2p();

					obj1->stanpoint += obj->get()->stanpoint;

					UI_Manager::Instance().combo[1]++;
					if (!obj1->isGround)
					{
						obj1->velocity.y += COLLECTIONAIR - (UI_Manager::Instance().combo[1] * obj->get()->attenuation_rate);
						if (COLLECTIONAIR <= 0)
						{
							COLLECTIONAIR = 0;
						}
					}
					//if (obj->get()->hitcount >= 0)
					{
						obj->get()->trigger = false;

						obj1->health -= obj->get()->damage;

						Effekseer::Vector3D pos = { obj1->position.x,obj1->position.y,obj1->position.z };

						effect.EffectPlay(effect.efc_smoke, pos, VECTOR3(5.0f, 5.0f, 5.0f));
					}
				}

			}

			else if (Collision::rectHitCheck(obj1->hit_rect[character_part::BODY], obj->get()->_attack_range))
			{
				if (obj1->guard_elements & obj->get()->attack_types)
				{
					obj1->success_guard = true;
				}

				else
				{
					obj1->success_guard = false;
				}
				obj->get()->hitcount--;

				if (obj1->success_guard)
				{
					obj1->velocity = obj->get()->vector * attenuation_rate;
					obj1->velocity.y = 0.0f;

					Effekseer::Vector3D pos;
					if (get_1pdistance() >= 0)
					{
						pos = { obj1->position.x + 30,obj1->position.y,obj1->position.z };
					}

					else
					{
						pos = { obj1->position.x - 30,obj1->position.y,obj1->position.z };
					}

					effect.EffectPlay(effect.efc_guard, pos, VECTOR3(5.0f, 5.0f, 5.0f));


					if (obj1->stanpoint < MAX_STANPOINT)
					{
						obj1->stanpoint += obj->get()->stanpoint / 2.0f;
					}


					//if (obj->get()->hitcount >= 0)
					{
						obj->get()->trigger = false;


					}
				}

				else
				{
					obj1->velocity = obj->get()->vector;

					obj1->ishit = true;
					UI_Manager::Instance().Combo_TimerReset2p();
					UI_Manager::Instance().combo[1]++;
					obj1->stanpoint += obj->get()->stanpoint;
					if (!obj1->isGround)
					{
						obj1->velocity.y += COLLECTIONAIR - (UI_Manager::Instance().combo[1] * obj->get()->attenuation_rate);
						if (COLLECTIONAIR <= 0)
						{
							COLLECTIONAIR = 0;
						}
					}
					//if (obj->get()->hitcount >= 0)
					{
						obj->get()->trigger = false;

						obj1->health -= obj->get()->damage;

						Effekseer::Vector3D pos = { obj1->position.x,obj1->position.y + 20,obj1->position.z };

						effect.EffectPlay(effect.efc_smoke2, pos, VECTOR3(5.0f, 5.0f, 5.0f));
					}
				}


			}
			else if (Collision::rectHitCheck(obj->get()->_attack_range, obj1->hit_rect[character_part::REG]))
			{
				if (obj1->guard_elements & obj->get()->attack_types)
				{
					obj1->success_guard = true;
				}

				else 	obj1->success_guard = false;
				obj->get()->hitcount--;


				if (obj1->success_guard)
				{
					obj1->velocity = obj->get()->vector * attenuation_rate;
					obj1->velocity.y = 0.0f;

					Effekseer::Vector3D pos;
					if (get_1pdistance() >= 0)
					{
						pos = { obj1->position.x + 30,obj1->position.y,obj1->position.z };
					}

					else
					{
						pos = { obj1->position.x - 30,obj1->position.y,obj1->position.z };
					}

					effect.EffectPlay(effect.efc_guard, pos, VECTOR3(5.0f, 5.0f, 5.0f));

					if (obj1->stanpoint < MAX_STANPOINT)
					{
						obj1->stanpoint += obj->get()->stanpoint / 2.0f;
					}



					//if (obj->get()->hitcount >= 0)
					{
						obj->get()->trigger = false;


					}

				}


				else
				{
					obj1->velocity = obj->get()->vector;

					obj1->ishit = true;
					UI_Manager::Instance().Combo_TimerReset2p();

					UI_Manager::Instance().combo[1]++;


					obj1->stanpoint += obj->get()->stanpoint;

					if (!obj1->isGround)
					{
						obj1->velocity.y += COLLECTIONAIR - (UI_Manager::Instance().combo[1] * obj->get()->attenuation_rate);

						if (COLLECTIONAIR <= 0)
						{
							COLLECTIONAIR = 0;
						}
					}

					//if (obj->get()->hitcount >= 0)
					{
						obj->get()->trigger = false;

						obj1->health -= obj->get()->damage;

						Effekseer::Vector3D pos = { obj1->position.x,obj1->position.y + 20,obj1->position.z };

						effect.EffectPlay(effect.efc_smoke3, pos, VECTOR3(5.0f, 5.0f, 5.0f));
					}
				}


			}
		}
	}



}

void judge_manager::player2_collision_update(OBJManager* object1, OBJManager* object2)
{

	auto obj2 = object2->getList().begin()->get();

	float delta_time = framework::Instance().elapased_time;



	for (auto obj = _attack_manager->attklist.begin(); obj != _attack_manager->attklist.end(); ++obj)
	{
		
		
		if (!obj->get()->trigger)continue;
		if (obj->get()->owner_tag == obj2->player_num)continue;
		if (obj2->is_invisible)continue;
		if (!obj2->is_guard)
		{

			if (Collision::rectHitCheck(obj->get()->_attack_range, obj2->hit_rect[character_part::HEAD]))
			{
				obj->get()->hitcount--;
				obj2->ishit = true;
				UI_Manager::Instance().Combo_TimerReset1p();
				UI_Manager::Instance().combo[0]++;
				obj2->velocity = obj->get()->vector;
				

				obj2->stanpoint += obj->get()->stanpoint;

				if (!obj2->isGround)
				{
					obj2->velocity.y += COLLECTIONAIR - (UI_Manager::Instance().combo[0] * obj->get()->attenuation_rate);

					if (COLLECTIONAIR <= 0)
					{
						COLLECTIONAIR = 0;
					}
				}

			/*	if (!obj2->inthehitrect)*/
				{
					obj->get()->trigger = false;
					obj2->health = obj2->health - obj->get()->damage;
					Effekseer::Vector3D pos = { obj2->head.x,obj2->head.y + 20,obj2->head.z };

					effect.EffectPlay(effect.efc_smoke, pos, VECTOR3(5.0f, 5.0f, 5.0f));
				}
			}

			else if (Collision::rectHitCheck(obj2->hit_rect[character_part::BODY], obj->get()->_attack_range))
			{
				obj->get()->hitcount--;
				obj2->ishit = true;
				UI_Manager::Instance().Combo_TimerReset1p();
				UI_Manager::Instance().combo[0]++;
				obj2->velocity = obj->get()->vector;

				obj2->stanpoint += obj->get()->stanpoint;

				if (!obj2->isGround)
				{
					obj2->velocity.y += COLLECTIONAIR - (UI_Manager::Instance().combo[0] * obj->get()->attenuation_rate);

					if (COLLECTIONAIR <= 0)
					{
						COLLECTIONAIR = 0;
					}
				}
				//if (obj->get()->hitcount >= 0)
				{
					obj->get()->trigger = false;
					obj2->health = obj2->health - obj->get()->damage;

					//Effekseer::Vector3D pos = { obj2->body.x,obj2->body.y,obj2->body.z };
					Effekseer::Vector3D pos = { obj2->body.x,obj2->body.y+20,obj2->body.z };

					effect.EffectPlay(effect.efc_smoke2, pos, VECTOR3(5.0f, 5.0f, 5.0f));
				}
			}
			else if (Collision::rectHitCheck(obj->get()->_attack_range, obj2->hit_rect[character_part::REG]))
			{
				obj->get()->hitcount--;
				obj2->ishit = true;


				UI_Manager::Instance().Combo_TimerReset1p();
				UI_Manager::Instance().combo[0]++;
				obj2->velocity = obj->get()->vector;


				obj2->stanpoint += obj->get()->stanpoint;

				if (!obj2->isGround)
				{
					obj2->velocity.y += COLLECTIONAIR - (UI_Manager::Instance().combo[0] * obj->get()->attenuation_rate);

					if (COLLECTIONAIR <= 0)
					{
						COLLECTIONAIR = 0;
					}
				}
				//if (obj->get()->hitcount >= 0)
				{
					obj->get()->trigger = false;
					obj2->health = obj2->health - obj->get()->damage;
					Effekseer::Vector3D pos = { obj2->reg.x,obj2->reg.y + 20,obj2->reg.z };

					effect.EffectPlay(effect.efc_smoke3, pos, VECTOR3(5.0f, 5.0f, 5.0f));

				}

			}

		}

		else
		{
			if (Collision::rectHitCheck(obj->get()->_attack_range, obj2->hit_rect[character_part::HEAD]))
			{
				if (obj2->guard_elements & obj->get()->attack_types)
				{
					obj2->success_guard = true;
				}

				else 	obj2->success_guard = false;

				obj->get()->hitcount--;


				if (obj2->success_guard)
				{

					obj2->velocity = obj->get()->vector * attenuation_rate;
					obj2->velocity.y = 0.0f;

					Effekseer::Vector3D pos;
					if (get_2pdistance() >= 0)
					{
						pos = { obj2->position.x + 30,obj2->position.y,obj2->position.z };
					}

					else
					{
						pos = { obj2->position.x - 30,obj2->position.y,obj2->position.z };
					}

					effect.EffectPlay(effect.efc_guard, pos, VECTOR3(5.0f, 5.0f, 5.0f));

					if (obj2->stanpoint < MAX_STANPOINT)
					{
						obj2->stanpoint += obj->get()->stanpoint / 2.0f;
					}


					//if (obj->get()->hitcount >= 0)
					{
						obj->get()->trigger = false;


					}

				}


				else
				{
					obj2->velocity = obj->get()->vector;

					obj2->ishit = true;
					UI_Manager::Instance().Combo_TimerReset1p();
					UI_Manager::Instance().combo[0]++;


					obj2->stanpoint += obj->get()->stanpoint;

					if (!obj2->isGround)
					{
						obj2->velocity.y += COLLECTIONAIR - (UI_Manager::Instance().combo[0] * obj->get()->attenuation_rate);

						if (COLLECTIONAIR <= 0)
						{
							COLLECTIONAIR = 0;
						}
					}

					//if (obj->get()->hitcount >= 0)
					{
						obj->get()->trigger = false;
						obj2->health = obj2->health - obj->get()->damage;
						Effekseer::Vector3D pos = { obj2->position.x,obj2->position.y + 20,obj2->position.z };

						effect.EffectPlay(effect.efc_smoke, pos, VECTOR3(5.0f, 5.0f, 5.0f));

					}
				}
			}

			else if (Collision::rectHitCheck(obj2->hit_rect[character_part::BODY], obj->get()->_attack_range))
			{
				if (obj2->guard_elements & obj->get()->attack_types)
				{
					obj2->success_guard = true;
				}
				else 	obj2->success_guard = false;

				obj->get()->hitcount--;


				if (obj2->success_guard)
				{
					//obj2->ishit = true;
					obj2->velocity = obj->get()->vector * attenuation_rate;
					obj2->velocity.y = 0.0f;

					Effekseer::Vector3D pos;
					if (get_2pdistance() >= 0)
					{
						pos = { obj2->position.x + 30,obj2->position.y,obj2->position.z };
					}

					else
					{
						pos = { obj2->position.x - 30,obj2->position.y,obj2->position.z };
					}

					effect.EffectPlay(effect.efc_guard, pos, VECTOR3(5.0f, 5.0f, 5.0f));


					if (obj2->stanpoint < MAX_STANPOINT)
					{
						obj2->stanpoint += obj->get()->stanpoint / 2.0f;
					}


					///if (obj->get()->hitcount >= 0)
					{
						obj->get()->trigger = false;


					}

				}


				else
				{
					obj2->velocity = obj->get()->vector;
					obj2->ishit = true;
					UI_Manager::Instance().Combo_TimerReset1p();
					UI_Manager::Instance().combo[0]++;



					obj2->stanpoint += obj->get()->stanpoint;


					if (!obj2->isGround)
					{
						obj2->velocity.y += COLLECTIONAIR - (UI_Manager::Instance().combo[0] * obj->get()->attenuation_rate);

						if (COLLECTIONAIR <= 0)
						{
							COLLECTIONAIR = 0;
						}
					}

					//if (obj->get()->hitcount >= 0)
					{
						obj->get()->trigger = false;

						obj2->health = obj2->health - obj->get()->damage;
						Effekseer::Vector3D pos = { obj2->position.x,obj2->position.y + 20,obj2->position.z };

						effect.EffectPlay(effect.efc_smoke2, pos, VECTOR3(5.0f, 5.0f, 5.0f));
					}
				}
			}
			else if (Collision::rectHitCheck(obj->get()->_attack_range, obj2->hit_rect[character_part::REG]))
			{
				if (obj2->guard_elements & obj->get()->attack_types)
				{
					obj2->success_guard = true;
				}
			else 	obj2->success_guard = false;

				obj->get()->hitcount--;


				if (obj2->success_guard)
				{

					obj2->velocity = obj->get()->vector * attenuation_rate;
					obj2->velocity.y = 0.0f;

					Effekseer::Vector3D pos;
					if (get_2pdistance() >= 0)
					{
						pos = { obj2->position.x + 30,obj2->position.y,obj2->position.z };
					}

					else
					{
						pos = { obj2->position.x - 30,obj2->position.y,obj2->position.z };
					}

					effect.EffectPlay(effect.efc_guard, pos, VECTOR3(5.0f, 5.0f, 5.0f));



					if (obj2->stanpoint < MAX_STANPOINT)
					{
						obj2->stanpoint += obj->get()->stanpoint / 2.0f;
					}
					

					//if (obj->get()->hitcount >= 0)
					{
						obj->get()->trigger = false;


					}

				}


				else
				{
					obj2->velocity = obj->get()->vector;

					if (!obj2->isGround)
					{
						obj2->velocity.y += COLLECTIONAIR - (UI_Manager::Instance().combo[0] * obj->get()->attenuation_rate);

						if (COLLECTIONAIR <= 0)
						{
							COLLECTIONAIR = 0;
						}
					}

					obj2->ishit = true;
					UI_Manager::Instance().Combo_TimerReset1p();


					obj2->stanpoint += obj->get()->stanpoint;


					UI_Manager::Instance().combo[0]++;

					//if (obj->get()->hitcount >= 0)
					{
						obj->get()->trigger = false;

						obj2->health = obj2->health - obj->get()->damage;
						Effekseer::Vector3D pos = { obj2->position.x,obj2->position.y + 20,obj2->position.z };

						effect.EffectPlay(effect.efc_smoke3, pos, VECTOR3(5.0f, 5.0f, 5.0f));
					}
				}

			}

		}
	}

	

}

void judge_manager::player_collision_update(OBJManager* object1, int combo)
{
	auto objct = object1->getList().begin()->get();

	float delta_time = framework::Instance().elapased_time;



	for (auto obj = _attack_manager->attklist.begin(); obj != _attack_manager->attklist.end(); ++obj)
	{


		if (!obj->get()->trigger)continue;
		if (obj->get()->owner_tag == objct->player_num)continue;
		if (objct->is_invisible)continue;
		if (!objct->is_guard)
		{

			if (Collision::rectHitCheck(obj->get()->_attack_range, objct->hit_rect[character_part::HEAD]))
			{
				obj->get()->hitcount--;
				objct->ishit = true;
				if (objct->tag == "PLAYER1")
				{
				
					UI_Manager::Instance().Combo_TimerReset2p();
				}
				else if (objct->tag == "PLAYER2")
				{

					UI_Manager::Instance().Combo_TimerReset1p();
				}

				combo++;

			

				objct->velocity = obj->get()->vector;


				objct->stanpoint += obj->get()->stanpoint;

				framework::Instance().setdulation(obj->get()->hitstop);


				if (!objct->isGround)
				{
					objct->velocity.y += COLLECTIONAIR - (combo * obj->get()->attenuation_rate);

					if (COLLECTIONAIR <= 0)
					{
						COLLECTIONAIR = 0;
					}
				}

				/*	if (!objct->inthehitrect)*/
				{
					if (obj->get()->hitcount <= 0)
					{
						obj->get()->trigger = false;
					}

					Audio::GetSound("hit")->Play();

					objct->health = objct->health - obj->get()->damage * (combo * 0.25f);
					Effekseer::Vector3D pos = { objct->head.x,objct->head.y + 20,objct->head.z };

					effect.EffectPlay(effect.efc_smoke, pos, VECTOR3(5.0f, 5.0f, 5.0f));
				}
			}

			else if (Collision::rectHitCheck(objct->hit_rect[character_part::BODY], obj->get()->_attack_range))
			{
				obj->get()->hitcount--;
				objct->ishit = true;
				if (objct->tag == "PLAYER1")
				{

					UI_Manager::Instance().Combo_TimerReset2p();
				}
				else if (objct->tag == "PLAYER2")
				{

					UI_Manager::Instance().Combo_TimerReset1p();
				}

			    combo++;
				

				objct->velocity = obj->get()->vector;

				objct->stanpoint += obj->get()->stanpoint;

				framework::Instance().setdulation(obj->get()->hitstop);

				if (!objct->isGround)
				{
					objct->velocity.y += COLLECTIONAIR - (combo * obj->get()->attenuation_rate);

					if (COLLECTIONAIR <= 0)
					{
						COLLECTIONAIR = 0;
					}
				}
				//if (obj->get()->hitcount >= 0)
				{
					if (obj->get()->hitcount <= 0)
					{
						obj->get()->trigger = false;
					}
					objct->health = objct->health - obj->get()->damage * (combo * 0.25f);

					Audio::GetSound("hit")->Play();

					//Effekseer::Vector3D pos = { objct->body.x,objct->body.y,objct->body.z };
					Effekseer::Vector3D pos = { objct->body.x,objct->body.y + 20,objct->body.z };

					effect.EffectPlay(effect.efc_smoke2, pos, VECTOR3(5.0f, 5.0f, 5.0f));
				}
			}
			else if (Collision::rectHitCheck(obj->get()->_attack_range, objct->hit_rect[character_part::REG]))
			{
				obj->get()->hitcount--;
				objct->ishit = true;


				if (objct->tag == "PLAYER1")
				{

					UI_Manager::Instance().Combo_TimerReset2p();
				}
				else if (objct->tag == "PLAYER2")
				{

					UI_Manager::Instance().Combo_TimerReset1p();
				}
				combo++;
				objct->velocity = obj->get()->vector;


				

				objct->stanpoint += obj->get()->stanpoint;


				framework::Instance().setdulation(obj->get()->hitstop);

				if (!objct->isGround)
				{
					objct->velocity.y += COLLECTIONAIR - (combo * obj->get()->attenuation_rate);

					if (COLLECTIONAIR <= 0)
					{
						COLLECTIONAIR = 0;
					}
				}
				//if (obj->get()->hitcount >= 0)
				{
					if (obj->get()->hitcount <= 0)
					{
						obj->get()->trigger = false;
					}

					Audio::GetSound("hit")->Play();
					objct->health = objct->health - obj->get()->damage * (combo * 0.25f);
					Effekseer::Vector3D pos = { objct->reg.x,objct->reg.y + 20,objct->reg.z };

					effect.EffectPlay(effect.efc_smoke3, pos, VECTOR3(5.0f, 5.0f, 5.0f));

				}

			}

		}

		else
		{
		

			if (Collision::rectHitCheck(obj->get()->_attack_range, objct->guard_rect))
			{
				objct->guard_redy = true;
			}

			else
			{
				objct->guard_redy = false;
			}


			if (Collision::rectHitCheck(obj->get()->_attack_range, objct->hit_rect[character_part::HEAD]))
			{
				if (objct->guard_elements & obj->get()->attack_types)
				{
					objct->success_guard = true;
				}

				else 	objct->success_guard = false;

				obj->get()->hitcount--;


				if (objct->success_guard)
				{

					objct->velocity = obj->get()->vector * attenuation_rate;
					objct->velocity.y = 0.0f;


					framework::Instance().setdulation(obj->get()->hitstop/2);


					Audio::GetSound("guard")->Play();

					Effekseer::Vector3D pos;
					if (get_2pdistance() >= 0)
					{
						pos = { objct->position.x + 30,objct->position.y,objct->position.z };
					}

					else
					{
						pos = { objct->position.x - 30,objct->position.y,objct->position.z };
					}

					effect.EffectPlay(effect.efc_guard, pos, VECTOR3(5.0f, 5.0f, 5.0f));

					if (objct->stanpoint < MAX_STANPOINT)
					{
						objct->stanpoint += obj->get()->stanpoint / 2.0f;
					}


					if (obj->get()->hitcount <= 0)
					{
						obj->get()->trigger = false;
					}

				}


				else
				{
					objct->velocity = obj->get()->vector;

					objct->ishit = true;
					if (objct->tag == "PLAYER1")
					{

						UI_Manager::Instance().Combo_TimerReset2p();
					}
					else if (objct->tag == "PLAYER2")
					{

						UI_Manager::Instance().Combo_TimerReset1p();
					}
				     combo++;

					

					objct->stanpoint += obj->get()->stanpoint;

					framework::Instance().setdulation(obj->get()->hitstop);

					if (!objct->isGround)
					{
						objct->velocity.y += COLLECTIONAIR - (UI_Manager::Instance().combo[0] * obj->get()->attenuation_rate);

						if (COLLECTIONAIR <= 0)
						{
							COLLECTIONAIR = 0;
						}
					}

					//if (obj->get()->hitcount >= 0)
					{
						if (obj->get()->hitcount <= 0)
						{
							obj->get()->trigger = false;
						}
						objct->health = objct->health - obj->get()->damage * (combo * 0.25f);
						Audio::GetSound("hit")->Play();

						Effekseer::Vector3D pos = { objct->position.x,objct->position.y + 20,objct->position.z };

						effect.EffectPlay(effect.efc_smoke, pos, VECTOR3(5.0f, 5.0f, 5.0f));

					}
				}
			}

			else if (Collision::rectHitCheck(objct->hit_rect[character_part::BODY], obj->get()->_attack_range))
			{
				if (objct->guard_elements & obj->get()->attack_types)
				{
					objct->success_guard = true;
				}
				else 	objct->success_guard = false;

				obj->get()->hitcount--;


				if (objct->success_guard)
				{
					//objct->ishit = true;
					objct->velocity = obj->get()->vector * attenuation_rate;
					objct->velocity.y = 0.0f;


					Audio::GetSound("guard")->Play();

					framework::Instance().setdulation(obj->get()->hitstop/2);

					Effekseer::Vector3D pos;
					if (get_2pdistance() >= 0)
					{
						pos = { objct->position.x + 30,objct->position.y,objct->position.z };
					}

					else
					{
						pos = { objct->position.x - 30,objct->position.y,objct->position.z };
					}

					effect.EffectPlay(effect.efc_guard, pos, VECTOR3(5.0f, 5.0f, 5.0f));


					if (objct->stanpoint < MAX_STANPOINT)
					{
						objct->stanpoint += obj->get()->stanpoint / 2.0f;
					}


					///if (obj->get()->hitcount >= 0)
					{
						if (obj->get()->hitcount <= 0)
						{
							obj->get()->trigger = false;
						}


					}

				}


				else
				{
					objct->velocity = obj->get()->vector;
					objct->ishit = true;
					if (objct->tag == "PLAYER1")
					{

						UI_Manager::Instance().Combo_TimerReset2p();
					}
					else if (objct->tag == "PLAYER2")
					{

						UI_Manager::Instance().Combo_TimerReset1p();
					}
					combo++;

				

					objct->stanpoint += obj->get()->stanpoint;

					framework::Instance().setdulation(obj->get()->hitstop);


					if (!objct->isGround)
					{
						objct->velocity.y += COLLECTIONAIR - (UI_Manager::Instance().combo[0] * obj->get()->attenuation_rate);

						if (COLLECTIONAIR <= 0)
						{
							COLLECTIONAIR = 0;
						}
					}

					//if (obj->get()->hitcount >= 0)
					{
						if (obj->get()->hitcount <= 0)
						{
							obj->get()->trigger = false;
						}

						Audio::GetSound("hit")->Play();

						objct->health = objct->health - obj->get()->damage * (combo * 0.25f);
						Effekseer::Vector3D pos = { objct->position.x,objct->position.y + 20,objct->position.z };

						effect.EffectPlay(effect.efc_smoke2, pos, VECTOR3(5.0f, 5.0f, 5.0f));
					}
				}
			}
			else if (Collision::rectHitCheck(obj->get()->_attack_range, objct->hit_rect[character_part::REG]))
			{
				if (objct->guard_elements & obj->get()->attack_types)
				{
					objct->success_guard = true;
				}
				else 	objct->success_guard = false;

				obj->get()->hitcount--;


				if (objct->success_guard)
				{

					objct->velocity = obj->get()->vector * attenuation_rate;
					objct->velocity.y = 0.0f;


					Audio::GetSound("guard")->Play();

					framework::Instance().setdulation(obj->get()->hitstop/2.0f);

					Effekseer::Vector3D pos;
					if (get_2pdistance() >= 0)
					{
						pos = { objct->position.x + 30,objct->position.y,objct->position.z };
					}

					else
					{
						pos = { objct->position.x - 30,objct->position.y,objct->position.z };
					}

					effect.EffectPlay(effect.efc_guard, pos, VECTOR3(5.0f, 5.0f, 5.0f));



					if (objct->stanpoint < MAX_STANPOINT)
					{
						objct->stanpoint += obj->get()->stanpoint / 2.0f;
					}


					//if (obj->get()->hitcount >= 0)
					{
						if (obj->get()->hitcount <= 0)
						{
							obj->get()->trigger = false;
						}


					}

				}


				else
				{
					objct->velocity = obj->get()->vector;

					if (!objct->isGround)
					{
						objct->velocity.y += COLLECTIONAIR - (UI_Manager::Instance().combo[0] * obj->get()->attenuation_rate);

						if (COLLECTIONAIR <= 0)
						{
							COLLECTIONAIR = 0;
						}
					}

					objct->ishit = true;
					if (objct->tag == "PLAYER1")
					{

						UI_Manager::Instance().Combo_TimerReset2p();
					}
					else if (objct->tag == "PLAYER2")
					{

						UI_Manager::Instance().Combo_TimerReset1p();
					}


					objct->stanpoint += obj->get()->stanpoint;


					

					combo++;


					framework::Instance().setdulation(obj->get()->hitstop);

					//if (obj->get()->hitcount >= 0)
					{
						if (obj->get()->hitcount <= 0)
						{
							obj->get()->trigger = false;
						}

						objct->health = objct->health - obj->get()->damage * (combo * 0.25f);

						Audio::GetSound("hit")->Play();

						Effekseer::Vector3D pos = { objct->position.x,objct->position.y + 20,objct->position.z };

						effect.EffectPlay(effect.efc_smoke3, pos, VECTOR3(5.0f, 5.0f, 5.0f));
					}
				}

			}

		}
	}


	if (objct->tag == "PLAYER1")
	{

		UI_Manager::Instance().combo[1] = combo;
	}
	else if (objct->tag == "PLAYER2")
	{

		UI_Manager::Instance().combo[0] = combo;
	}

}

void judge_manager::extrude_update(OBJManager* object1, OBJManager* object2)
{
	auto obj1 = object1->getList().begin()->get();
	auto obj2 = object2->getList().begin()->get();

	float delta_time = framework::Instance().elapased_time;

	if (Collision::rectHitCheck(obj1->extrude_rect, obj2->extrude_rect))
	{

		if (!obj1->iswall)
		{
			if (!obj1->is_invisible)
			{
				if (distance1p < 0)
				{
					if (!Collision::rectHitCheck(obj1->extrude_rect_center, obj2->extrude_rect_center))
					{
						obj1->position.x += (correction_velocity)*delta_time;//左から
					}

					else
					{
						if (obj1->velocity.y == 0)
						{
							obj1->position.x -= correction_velocity * delta_time;//右から
						}
					}
				}


				else if (distance1p > 0)
				{
					//if (!Collision::rectHitCheck(obj1->extrude_rect_center, obj2->extrude_rect_center))
					{
						obj1->position.x -= correction_velocity * delta_time;//右から
					}
				}




				else if (distance1p < 0 && !obj1->isGround)
				{
					obj1->position.x += correction_velocity_y * delta_time;//左から
				}

				else if (distance1p > 0 && !obj1->isGround)
				{
					obj1->position.x -= correction_velocity_y * delta_time;;//右から
				}

				if (obj1->velocity.x == 0)
				{

					obj2->velocity.x = 0;

				}
			}
		}




		if (!obj2->iswall)
		{
			if (!obj1->is_invisible)
			{
				if (distance2p < 0)
				{
					//if (!Collision::rectHitCheck(obj1->extrude_rect_center, obj2->extrude_rect_center))
					{
						obj2->position.x += correction_velocity * delta_time;//左から
					}
				}

				else if (distance2p > 0)
				{
					if (!Collision::rectHitCheck(obj1->extrude_rect_center, obj2->extrude_rect_center))
					{
						obj2->position.x -= correction_velocity * delta_time;;//右から
					}

					else
					{
						if (obj2->velocity.y == 0)
						{
							obj2->position.x += correction_velocity * delta_time;;//右から
						}
					}
				}

				else if (distance2p < 0 && !obj2->isGround)
				{
					obj2->position.x += correction_velocity * delta_time;//左から
				}

				else if (distance2p > 0 && !obj2->isGround)
				{
					obj2->position.x -= correction_velocity * delta_time;//右から
				}

				if (obj2->velocity.x == 0)
				{

					obj1->velocity.x = 0;

				}
			}
		}

	}




}


void judge_manager::drawhit(OBJManager* object1, OBJManager* object2)
{
     auto obj1 = object1->getList().begin()->get();
     auto obj2 = object2->getList().begin()->get();


	 if (ishitrect)
	 {

		 primitive_batch::begin();
		 primitive_batch::rect(VECTOR2(obj1->hit_rect[character_part::HEAD].left, obj1->hit_rect[character_part::HEAD].top), VECTOR2(obj1->hit_rect[character_part::HEAD].right - obj1->hit_rect[character_part::HEAD].left, obj1->hit_rect[character_part::HEAD].bottom - obj1->hit_rect[character_part::HEAD].top), VECTOR2(0, 0), 0.0f, VECTOR4(0, 1, 0, 0.3f));
		 primitive_batch::rect(VECTOR2(obj1->hit_rect[character_part::BODY].left, obj1->hit_rect[character_part::BODY].top), VECTOR2(obj1->hit_rect[character_part::BODY].right - obj1->hit_rect[character_part::BODY].left, obj1->hit_rect[character_part::BODY].bottom - obj1->hit_rect[character_part::BODY].top), VECTOR2(0, 0), 0.0f, VECTOR4(0, 1, 0, 0.3f));
		 primitive_batch::rect(VECTOR2(obj1->hit_rect[character_part::REG].left, obj1->hit_rect[character_part::REG].top), VECTOR2(obj1->hit_rect[character_part::REG].right - obj1->hit_rect[character_part::REG].left, obj1->hit_rect[character_part::REG].bottom - obj1->hit_rect[character_part::REG].top), VECTOR2(0, 0), 0.0f, VECTOR4(0, 1, 0, 0.3f));
		 primitive_batch::rect(VECTOR2(obj1->extrude_rect.left, obj1->extrude_rect.top), VECTOR2(obj1->extrude_rect.right - obj1->extrude_rect.left, obj1->extrude_rect.bottom - obj1->extrude_rect.top), VECTOR2(0, 0), 0.0f, VECTOR4(0.5f, 0.5f, 1, 0.3f));
		 //primitive_batch::rect(VECTOR2(obj1->extrude_rect_center.left, obj1->extrude_rect_center.top), VECTOR2(obj1->extrude_rect_center.right - obj1->extrude_rect_center.left, obj1->extrude_rect_center.bottom - obj1->extrude_rect_center.top), VECTOR2(0, 0), 0.0f, VECTOR4(0.5f, 0.5f, 1, 0.3f));
		 primitive_batch::rect(VECTOR2(obj1->areacheck_rect.left, obj1->areacheck_rect.top), VECTOR2(obj1->areacheck_rect.right - obj1->areacheck_rect.left, obj1->areacheck_rect.bottom - obj1->areacheck_rect.top), VECTOR2(0, 0), 0.0f, VECTOR4(1.5f, 1.5f, 1.5f, 0.3f));
		 primitive_batch::rect(VECTOR2(obj1->guard_rect.left, obj1->guard_rect.top), VECTOR2(obj1->guard_rect.right - obj1->guard_rect.left, obj1->guard_rect.bottom - obj1->guard_rect.top), VECTOR2(0, 0), 0.0f, VECTOR4(1.5f, 1.5f, 1.5f, 0.3f));


		 primitive_batch::rect(VECTOR2(obj2->hit_rect[character_part::HEAD].left, obj2->hit_rect[character_part::HEAD].top), VECTOR2(obj2->hit_rect[character_part::HEAD].right - obj2->hit_rect[character_part::HEAD].left, obj2->hit_rect[character_part::HEAD].bottom - obj2->hit_rect[character_part::HEAD].top), VECTOR2(0, 0), 0.0f, VECTOR4(0, 1, 0, 0.3f));
		 primitive_batch::rect(VECTOR2(obj2->hit_rect[character_part::BODY].left, obj2->hit_rect[character_part::BODY].top), VECTOR2(obj2->hit_rect[character_part::BODY].right - obj2->hit_rect[character_part::BODY].left, obj2->hit_rect[character_part::BODY].bottom - obj2->hit_rect[character_part::BODY].top), VECTOR2(0, 0), 0.0f, VECTOR4(0, 1, 0, 0.3f));
		 primitive_batch::rect(VECTOR2(obj2->hit_rect[character_part::REG].left, obj2->hit_rect[character_part::REG].top), VECTOR2(obj2->hit_rect[character_part::REG].right - obj2->hit_rect[character_part::REG].left, obj2->hit_rect[character_part::REG].bottom - obj2->hit_rect[character_part::REG].top), VECTOR2(0, 0), 0.0f, VECTOR4(0, 1, 0, 0.3f));
		 primitive_batch::rect(VECTOR2(obj2->extrude_rect.left, obj2->extrude_rect.top), VECTOR2(obj2->extrude_rect.right - obj2->extrude_rect.left, obj2->extrude_rect.bottom - obj2->extrude_rect.top), VECTOR2(0, 0), 0.0f, VECTOR4(0.5f, 0.5f, 1, 0.3f));
		 primitive_batch::rect(VECTOR2(obj2->areacheck_rect.left, obj2->areacheck_rect.top), VECTOR2(obj2->areacheck_rect.right - obj2->areacheck_rect.left, obj2->areacheck_rect.bottom - obj2->areacheck_rect.top), VECTOR2(0, 0), 0.0f, VECTOR4(1.5f, 1.5f, 1.5f, 0.3f));
		 primitive_batch::rect(VECTOR2(obj2->guard_rect.left, obj2->guard_rect.top), VECTOR2(obj2->guard_rect.right - obj2->guard_rect.left, obj2->guard_rect.bottom - obj2->guard_rect.top), VECTOR2(0, 0), 0.0f, VECTOR4(1.5f, 1.5f, 1.5f, 0.3f));

		 //primitive_batch::rect(VECTOR2(obj2->extrude_rect_center.left, obj2->extrude_rect_center.top), VECTOR2(obj2->extrude_rect_center.right - obj2->extrude_rect_center.left, obj2->extrude_rect_center.bottom - obj2->extrude_rect_center.top), VECTOR2(0, 0), 0.0f, VECTOR4(0.5f, 0.5f, 1, 0.3f));


		 for (auto obj = _attack_manager->attklist.begin(); obj != _attack_manager->attklist.end(); ++obj)
		 {
			 if (obj->get()->attack_life_time >= 0 && obj->get()->attack_standby_frame <= 0 && obj->get()->trigger)
			 {
				 primitive_batch::rect(VECTOR2(obj->get()->_attack_range.left, obj->get()->_attack_range.top), VECTOR2(obj->get()->_attack_range.right - obj->get()->_attack_range.left, obj->get()->_attack_range.bottom - obj->get()->_attack_range.top), VECTOR2(0, 0), 0.0f, VECTOR4(1, 0, 0, 0.3f));
			 }
		 }

		 primitive_batch::end();
	 }
}







void judge_manager::WorldToScreen(VECTOR3* screenPosition, const VECTOR3& worldPosition, DirectX::XMFLOAT4X4 view, DirectX::XMFLOAT4X4 projection)
{

	// ビューポート
	float viewportX = 0.0f;
	float viewportY = 0.0f;
	float viewportW = static_cast<float>(pSystem.SCREEN_WIDTH);
	float viewportH = static_cast<float>(pSystem.SCREEN_HEIGHT);
	float viewportMinZ = 0.0f;
	float viewportMaxZ = 1.0f;
	// ビュー行列
	DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&view);
	// プロジェクション行列
	DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&projection);
	// ワールド行列
	DirectX::XMMATRIX W = DirectX::XMMatrixIdentity();//移動成分はいらないので単位行列を入れておく。

#if 1
	// ワールド座標からスクリーン座標へ変換（ライブラリを使うとこんなに便利）
	DirectX::XMVECTOR SPos = DirectX::XMVector3Project(DirectX::XMLoadFloat3(&worldPosition),
		viewportX, viewportY, viewportW, viewportH, viewportMinZ, viewportMaxZ,
		P, V, W);
	DirectX::XMStoreFloat3(screenPosition, SPos);
#else// ワールド座標からNDC座標へ変換（ライブラリを使わずに変換してみよう）
	DirectX::XMMATRIX WVP = W * V * P;
	DirectX::XMVECTOR NDCPosition = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&worldPosition), WVP);
	DirectX::XMFLOAT3 ndcPosition;
	DirectX::XMStoreFloat3(&ndcPosition, NDCPosition);
	// NDC座標からスクリーン座標へ変換
	screenPosition->x = ((ndcPosition.x + 1.0f) * 0.5f) * viewportW;

	screenPosition->y = viewportH - ((ndcPosition.y + 1.0f) * 0.5f) * viewportH;

	screenPosition->z = viewportMinZ + ndcPosition.z * (viewportMaxZ - viewportMinZ);


#endif



}

void judge_manager::ScreenToWorld(VECTOR3 screenPosition, VECTOR3& worldPosition, DirectX::XMFLOAT4X4 view, DirectX::XMFLOAT4X4 projection)
{

	// ビューポート
	float viewportX = 0.0f;
	float viewportY = 0.0f;
	float viewportW = static_cast<float>(pSystem.SCREEN_WIDTH);
	float viewportH = static_cast<float>(pSystem.SCREEN_HEIGHT);
	float viewportMinZ = 0.0f;
	float viewportMaxZ = 1.0f;
	// ビュー行列

	DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&view);
	// プロジェクション行列
	DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&projection);
	// ワールド行列
	DirectX::XMMATRIX W = DirectX::XMMatrixIdentity();//移動成分はいらないので単位行列を入れておく。

#if 0// スクリーン座標からワールド座標へ変換
	DirectX::XMVECTOR WPos = DirectX::XMVector3Unproject(DirectX::XMLoadFloat3(&screenPosition),
		viewportX, viewportY, viewportW, viewportH, viewportMinZ, viewportMaxZ, P, V, W);
	DirectX::XMStoreFloat3(worldPosition, WPos);
#else// スクリーン座標からNDC座標へ変換
	DirectX::XMFLOAT3 a = {};
	a.x = screenPosition.x / viewportW;
	a.y = screenPosition.y / viewportH;
	a.z = screenPosition.z / (viewportMaxZ - viewportMinZ) - viewportMinZ;

	DirectX::XMVECTOR VEC = DirectX::XMVectorSet(
		a.x * 2.0f - 1.0f,
		(2.0f - (a.y * 2.0f)) - 1.0f,
		a.z,
		1.0f);
	// NDC座標からワールド座標へ変換
	DirectX::XMMATRIX WVP = W * V * P;
	DirectX::XMMATRIX IWVP = DirectX::XMMatrixInverse(nullptr, WVP);

	DirectX::XMVECTOR WPos = DirectX::XMVector3TransformCoord(VEC, IWVP);
	DirectX::XMFLOAT3 wpos;
	DirectX::XMStoreFloat3(&wpos, WPos);

	worldPosition = VECTOR3(wpos.x, wpos.y, wpos.z);

#endif


}






//----------------------------------------------------------------------
//  右方向補正処理
//----------------------------------------------------------------------
void judge_manager::extrude_Right(OBJ3D* obj)
{
	float x = obj->screen_position.x + obj->size.x;
	x -= fmodf(x, static_cast<float>(correction_velocity));
	obj->screen_position.x = x - obj->size.x - ADJUST_X;
	VECTOR3 POS;

	ScreenToWorld(obj->screen_position, POS, obj->view, obj->projection);

	obj->position = POS;


	obj->velocity.x = 0.0f;
	//obj->hoseirect();
}

//----------------------------------------------------------------------
//  左方向補正処理
//----------------------------------------------------------------------
void judge_manager::extrude_Left(OBJ3D* obj)
{
	float x = obj->position.x - obj->size.x;
	x += 32.5f - fmodf(x, static_cast<float>(correction_velocity));
	obj->screen_position.x = x + obj->size.x - ADJUST_X;
		VECTOR3 POS;

	ScreenToWorld(obj->screen_position, POS, obj->view, obj->projection);

	obj->position = POS;

	obj->velocity.x = 0.0f;
	//obj->hoseirect();
}