#include "ui_hpgauge.h"
#include <wchar.h>

extern bool demo;

void UI_Manager::update(OBJManager* object1, OBJManager* object2,Camera* camera)
{
	auto obj1 = object1->getList().begin()->get();
	auto obj2 = object2->getList().begin()->get();
	hitpoint_1p = (float)obj1->health;
	hitpoint_2p = (float)obj2->health;



	if (!koflag)
	{
		hpgauge1p = (float)((hitpoint_1p / obj1->maxHealth) * 1600);

		hpgauge2p = (float)((hitpoint_2p / obj2->maxHealth) * 1600);


		stanpoint[0] = (obj1->stanpoint / 500.0f) * 1600;
		stanpoint[1] = (obj2->stanpoint / 500.0f) * 1600;


	}

	
	

	switch (state)
	{

	case 0:

		timer++;
		if (timer >= (60 * 2))
		{
			fade -= 0.125;
			if (fade <= 0.0)
			{
				state++;
				timer = 0;

	
			
			}
		}

		break;


	case 1:

		timer++;
		if (timer >= (60 * 5))
		{
			{
				state++;
				timer = 0;
				roundstart = true;
			}
		}

		break;


	case 2: 




		if (demo)
		{
			if (hitpoint_1p <= 0)
			{
				hpgauge1p = 1.5f;
				//koflag = true;
			}
			if (hitpoint_2p <= 0)
			{
				hpgauge2p = 1.5f;
				//koflag = true;
			}
		}

		else
		{

			if (hitpoint_1p <= 0 && hitpoint_2p <= 0)
			{
				hpgauge1p = 0.1f;
				hpgauge2p = 0.1f;

				koflag = true;

				Audio::GetSound("ko")->Play();

				framework::Instance().setdulation(60.0f * 3);

			}

			 else if (hitpoint_1p <= 0)
			{
				hpgauge1p = 0.1f;
				koflag = true;

				Audio::GetSound("ko")->Play();

				framework::Instance().setdulation(60.0f * 3);

				wincountp1--;
			
			}
			else if (hitpoint_2p <= 0)
			{
				hpgauge2p = 0.1f;
				koflag = true;

				Audio::GetSound("ko")->Play();

				framework::Instance().setdulation(60.0f * 3);


				wincountp2--;
				
			}

			if (koflag)
			{
				state++;
			}


			if (combo[0] > 0)
			{
				combo1Ptimer++;
			}

			if (combo[1] > 0)
			{
				combo2Ptimer++;
			}

		


			if (combo1Ptimer >= (60 * 2))
			{
				combo[0] = 0;
				combo1Ptimer = 120;
			}

			if (combo2Ptimer >= (60 * 2))
			{
				combo[1] = 0;
				combo2Ptimer = 120;
			}

		}

		break;

	case 3:

		if (size>=1.0f)
		{
			timer++;
		}
	

		if (timer >= (60 * 2))
		{
			if (wincountp1 <= 0 || wincountp2 <= 0)
			{
				alpha = 0.0f;
				break;
			}
		}

		if (timer >= (60 * 5))
		{
			
				fade += 0.125;
			if (fade > 4.0)
			{
				koflag = false;
				obj1->health = obj1->maxHealth;
				obj2->health = obj2->maxHealth;
				obj1->position = VECTOR3(-50, 1.0f, 0.0f);
				obj2->position = VECTOR3(70, 1.0f, 0.0f);

				
				size = 0.0f;
				state = 0;
				timer = 0;
				fade = 5.0;
				combo[0] = 0;
				combo[1] = 0;
				combo1Ptimer = 0;
				combo2Ptimer = 0;
				roundstart = false;

				camera->pos = DirectX::XMFLOAT3(.0f, 130.0f, -530.f);

				round++;

			}
		}
		break;
	}



	if (wincountp1<=0)
	{
		win2p = true;
		
	}

    if (wincountp2 <= 0)
	{
		win1p = true;
	}

	if (win1p && win2p)
	{
		win1p = false;
		win2p = false;
	}

	
		
	
}

void UI_Manager::render(Font* font)
{
	texture::begin(TEXNO::HP_GAUGE);
	
	////1p側のHP
	texture::draw(TEXNO::HP_GAUGE,
		40, 15, 0.35f, 0.5f,
		0, 0, hpgauge1p, 112,
		0, 0, 0,
		1, 1, 1, 1);

	//2p側のHP
	texture::draw(TEXNO::HP_GAUGE,
		framework::Instance().SCREEN_WIDTH - 45, 15, -0.35f, 0.5f,
		0, 0, hpgauge2p, 112,
		0, 0, 0,
		1, 1, 1, 1);

	

	texture::end(TEXNO::HP_GAUGE);


	texture::begin(TEXNO::GAUGE_COVER);
	texture::draw(TEXNO::GAUGE_COVER,
		0, 0, 0.4f, 0.5f,
		0, 0, 1600, 163,
		0, 0, 0,
		1, 1, 1, 1);

	texture::draw(TEXNO::GAUGE_COVER,
		1900, 0, -0.4f, 0.5f,
		0, 0, 1600, 163,
		0, 0, 0,
		1, 1, 1, 1);

	texture::end(TEXNO::GAUGE_COVER);

	texture::begin(TEXNO::STAN);

	//1p側のスタンゲージ
	texture::draw(TEXNO::STAN,
		40, 80, 0.35f, 0.5f,
		0, 0, stanpoint[0], 50,
		0, 0, 0,
		1, 1, 1, 1);

	//2p側のスタンゲージ
	texture::draw(TEXNO::STAN,
		framework::Instance().SCREEN_WIDTH - 45, 80, -0.35f, 0.5f,
		0, 0, stanpoint[1], 50,
		0, 0, 0,
		1, 1, 1, 1);


	texture::end(TEXNO::STAN);

	wchar_t buf1[100];
	_itow_s(combo[0], buf1, 10);
	wchar_t buf2[100];
	_itow_s(combo[1], buf2, 10);
	DirectX::XMFLOAT4 color1 = DirectX::XMFLOAT4(1.5f, 0.5f, 0.5f, 1.0f);
	DirectX::XMFLOAT4 color2 = DirectX::XMFLOAT4(1.5f, 0.5f, 0.5f, 1.0f);
	if (combo[0] >= 5)
	{
		color1 = DirectX::XMFLOAT4(0.5f, 0.5f, 1.5f, 1.0f);
	}
	 if (combo[0] >= 10 )
	{
		color1 = DirectX::XMFLOAT4(0.5f, 1.5f, 1.5f, 1.0f);
	}

	if (combo[1] >= 5)
	{
		color2 = DirectX::XMFLOAT4(0.5f, 0.5f, 1.5f, 1.0f);
	}
	 if (combo[1] >= 10)
	{
		color2 = DirectX::XMFLOAT4(0.5f, 1.5f, 0.5f, 1.0f);
	}

	if (combo[0] > 0)
	{
		font->Begin(pSystem.devicecontext.Get());

		font->Draw(150, 100, L"COMBO", color1, DirectX::XMFLOAT2(2.0f, 2.0f));

		font->Draw(50, 100, buf1, color1, DirectX::XMFLOAT2(2.0f, 2.0f));
		
		font->End(pSystem.devicecontext.Get());
	}
	
	if (combo[1] > 0)
	{
		font->Begin(pSystem.devicecontext.Get());

		font->Draw(1700, 100, L"COMBO", color2, DirectX::XMFLOAT2(2.0f, 2.0f));

		font->Draw(1650, 100, buf2, color2, DirectX::XMFLOAT2(2.0f, 2.0f));


		font->End(pSystem.devicecontext.Get());
	}


	if (koflag)
	{
		size += 0.1f;

		if (size>=1.0f)
		{
			size = 1.0f;


		

		}


		texture::begin(TEXNO::KO);
		texture::draw(TEXNO::KO,
			0, 0, size, size,
			0, 0, 1920, 1080,
			0, 0, 0,
			1, 1, 1, alpha);

		texture::end(TEXNO::KO);

	}


	if (wincountp2<=1)
	{
		primitive::rect(500, 100, 64, 100,
			0, 0, 0, 1.0f, 1.0f, 0, 1.0f);
	}

	if (wincountp2 <= 0)
	{
		primitive::rect(400, 100, 64, 100,
			0, 0, 0, 1.0f, 1.0f, 0.0f, 1.0f);
	}


	if (wincountp1 <= 1)
	{
		primitive::rect(1300, 100, 64, 100,
			0, 0, 0, 1.0f, 1.0f, 0, 1.0f);
	}

	if (wincountp1 <= 0)
	{
		primitive::rect(1400, 100, 64, 100,
			0, 0, 0, 1.0f, 1.0f, 0.0f, 1.0f);
	}


	if (state == 1)
	{
		wchar_t buf1[100];
		_itow_s(round, buf1, 10);

		DirectX::XMFLOAT4 color2 = DirectX::XMFLOAT4(1.5f, 1.5f, 0.5f, 1.0f);


		font->Begin(pSystem.devicecontext.Get());

		font->Draw(800, 250, L"ROUND", color2, DirectX::XMFLOAT2(2.0f, 2.0f));
		font->Draw(1000, 250, buf1, color2, DirectX::XMFLOAT2(2.0f, 2.0f));

		font->End(pSystem.devicecontext.Get());

		if (timer >= (60 * 3))
		{
			font->Begin(pSystem.devicecontext.Get());

			font->Draw(800, 350, L"FIGHT!!", color2, DirectX::XMFLOAT2(2.0f, 2.0f));
	

			font->End(pSystem.devicecontext.Get());
		}


	}


	if (fade >= 0.0f)
	{
		primitive::rect(0, 0, framework::Instance().SCREEN_WIDTH, framework::Instance().SCREEN_HEIGHT,
			0, 0, 0, 0, 0, 0, fade);
	}

}
