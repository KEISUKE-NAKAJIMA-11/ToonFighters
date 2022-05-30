#include "command.h"




void Stick_log::update(int hist, CommandList* clist,int timer )
{
	if (hist != old && timer > oldt)
	{
		sticklog.emplace_back(hist);

		timerlog.emplace_back(timer);

		old = hist;
		oldt = timer;
	}

		if (sticklog.size() > 20)
		{
			sticklog.pop_front();
			timerlog.pop_front();
		}

		if (hist == STICKDIRECTION::STICK_DOWN || hist == STICKDIRECTION::STICK_LEFT || hist == STICKDIRECTION::STICK_RIGHT)
		{
			clist->Reset();

		}

	
		old_stick = hist;

	//if (clist->trigger)
	{
		if (clist->command_timer < timer_max_command)
		{
			clist->command_timer += 0.1f;
		}

	}


}



void CommandList::checklist(int time)
{
	time = time - INTERVAL;
	for (int i = numI; i > -1; i--)
	{
		if (i == numI)continue;

		if (history[i] != history[numI])
		{
			if (timer[i] >= (time))	break;

			else
			{
				numI = i;
				break;
			}
		}
	}
}

Command CommandList::CommandCheack(int last, int* hist, int* histtimer, float distance, int time)
{
	//--------------------------------------------------------------
	//=========================================
	//		�R�}���h�m�F�֐�
	//=========================================
	//�E���̗͂������ŐV�̂��̂��炳���̂ڂ��Ă����A
	//  �ߋ��ɃR�}���h�����͂���Ă��邩�����𔻒肷��
	//---------------------------------------------------------------

	trigger = false;
	state = Command::NOCOMMAND;

	//List�̉�������������ۑ�����
	numI = 0;

	//�R�}���h����^�C�}�[�����̒l�ȓ��Ȃ�R�}���h��������悤�ɂ���(�R�}���h��t�P�\)
	if (command_timer > TIMER_LIMIT || command_timer <= 0.0f)
	{
		trigger = true;
		return Command::NOCOMMAND;
	}




	//command_timer = 0;
	for (int i = 0; i < MAX_BUFFER; i++)
	{
		history[i] = hist[i];
		timer[i] = histtimer[i];
	}

		
	

	//�����ōŌ�ɓ��͂��ꂽ�{�^����������Ă���(�U���p�{�^����X�AY�AB�ARB�{�^���Ȃ�)
	lastinput = last;

	//List�̒��g����납�猩�Ă���
	for (int i = MAX_BUFFER - 1; i > -1; i--)
	{
		if (history[i] != -1)
		{
			//�Ō�ɓ��͂��ꂽ���̂ƈႤ���̂ł����
			{

				if (history[i] != lastinput)
				{
					 if (history[i] == scastI(STICKDIRECTION::STICK_RIGHT))
					{
						//���͂��ꂽ�̂��E������
						state = Command::RHURF;
						numI = i;
						//Reset();
						break;
					}
					else if (history[i] == scastI(STICKDIRECTION::STICK_LEFT))
					{
						//���͂��ꂽ�̂���������
						state = Command::LHURF;
						numI = i;

						break;
					}
					else
					{
						//�R�}���h�͖�������
						return Command::NOCOMMAND;
					}
				}
			}
			
		}
	}

	if (numI < 2)
	{
		//�c���Ă���̂�2�ȉ��Ȃ�R�}���h�͐�΂ɐ������Ȃ��̂ł����ŋ�؂�
		return Command::NOCOMMAND;
	}

	//List�̒��g�𔻒肵���ꏊ���猩�Ă���
	//���R�}���h�͐��������͂���Ă����List�̒��Łu���v�u�����v�u���v�ƕ���ł���͂��Ȃ̂�
	//��������Q�Ƃ��Ă���
	switch (state)
	{
	case Command::RHURF:
		//�Ō�ɓ��͂��ꂽ�͉̂E������

		if (time - timer[numI] < LIMIT_FRAME)
		{

			if (timer[numI] - timer[numI - 1] < LIMIT_FRAME)
			{

				if (history[numI - 1] == scastI((STICKDIRECTION::STICK_RIGHT_DOWN)))
				{


					if (timer[numI - 1] - timer[numI - 2] > LIMIT_FRAME) return Command::NOCOMMAND;

					//���̑O�ɓ��͂��ꂽ�̂��E��������
					if (history[numI - 2] == scastI(STICKDIRECTION::STICK_DOWN))
					{

						//�X�ɂ��̑O�ɓ��͂��ꂽ�̂���������

						if (numI - 3 < 0|| numI - 2 < 0)return Command::NOCOMMAND;

						if (timer[numI - 2] - timer[numI - 3] > (LIMIT_FRAME+40))
						{
							if (distance > 0)
							{
								//�v���C���[�͉E�������Ă���
								trigger = true;

								interbal = 0;

								return Command::RHURF;
							}
							if (distance < 0)
							{
								//�v���C���[�͍��������Ă���
								trigger = true;

								interbal = 0;

								return Command::LHURF;
							}
						}

						else if (history[numI - 3] == scastI(STICKDIRECTION::STICK_RIGHT_DOWN))
						{

							if (numI - 3 < 0 || numI - 4 < 0) return Command::NOCOMMAND;

							if (timer[numI - 3] - timer[numI - 4] > (LIMIT_FRAME+40)) return Command::NOCOMMAND;
							if (history[numI - 4] == scastI(STICKDIRECTION::STICK_RIGHT))
							{
								if (distance > 0)
								{
									//�v���C���[�͉E�������Ă���
									trigger = true;

									interbal = 0;

									return Command::RFUJIN;
								}
								if (distance < 0)
								{
									//�v���C���[�͍��������Ă���
									trigger = true;

									interbal = 0;

									return Command::LFUJIN;
								}
							}
						}
					}


				}

				else if (history[numI - 1] == scastI((STICKDIRECTION::STICK_N)))
				{
					// ���̑O�ɓ��͂��ꂽ�̂��E������
					if (numI - 1 < 0 || numI - 2 < 0)return Command::NOCOMMAND;
					if (timer[numI - 1] - timer[numI - 2] > LIMIT_FRAME )return Command::NOCOMMAND;
					// checklist(time);

					if (history[numI - 2] == scastI(STICKDIRECTION::STICK_RIGHT))
					{
						{
							trigger = true;
							interbal = 0;
							return Command::RSTEP;
						}


					}
				}
				else
				{
					//�R�}���h�͂Ȃ�
					return Command::NOCOMMAND;
				}
			}
		}

		return Command::NOCOMMAND;

		break;

	case Command::LHURF:
		//�Ō�ɓ��͂��ꂽ�͍̂�������
		if (time - timer[numI] < LIMIT_FRAME)
		{

			if (timer[numI] - timer[numI - 1] < LIMIT_FRAME)
			{

				if (history[numI - 1] == scastI(STICKDIRECTION::STICK_LEFT_DOWN))
				{
					//���̑O�ɓ��͂��ꂽ�̂�����������
					if (numI - 1 < 0 || numI - 2 < 0)return Command::NOCOMMAND;
					if (timer[numI - 1] - timer[numI - 2] > LIMIT_FRAME)return Command::NOCOMMAND;

					if (history[numI - 2] == scastI(STICKDIRECTION::STICK_DOWN))
					{
						if (timer[numI - 2] - timer[numI - 3] > (LIMIT_FRAME+40))
						{
							//�X�ɂ��̑O�ɓ��͂��ꂽ�̂���������
							if (distance > 0)
							{
								//�v���C���[�͉E�������Ă���
								trigger = true;
								interbal = 0;
								return Command::LHURF;
							}
							if (distance < 0)
							{
								//�v���C���[�͍��������Ă���
								trigger = true;
								interbal = 0;
								return Command::RHURF;
							}
						}
						else if (history[numI - 3] == scastI(STICKDIRECTION::STICK_LEFT_DOWN))
						{
							//�X�ɂ��̑O�ɓ��͂��ꂽ�̂���������
							if (numI - 3 < 0 || numI - 4 < 0)return Command::NOCOMMAND;
							if (timer[numI - 3] - timer[numI - 4] > LIMIT_FRAME+40)return Command::NOCOMMAND;
							if (history[numI - 4] == scastI(STICKDIRECTION::STICK_LEFT))
							{
								if (distance > 0)
								{
									//�v���C���[�͉E�������Ă���
									trigger = true;
									interbal = 0;
									return Command::LFUJIN;
								}
								if (distance < 0)
								{
									//�v���C���[�͍��������Ă���
									trigger = true;
									interbal = 0;
									return Command::RFUJIN;
								}
							}
						}
					}
				}

				else if (history[numI - 1] == scastI((STICKDIRECTION::STICK_N)))
				{
					// ���̑O�ɓ��͂��ꂽ�̂��E������

					if (numI - 1 < 0 || numI - 2 < 0)return Command::NOCOMMAND;
					if (timer[numI - 1] - timer[numI - 2] > LIMIT_FRAME)return Command::NOCOMMAND;

					if (history[numI - 2] == scastI(STICKDIRECTION::STICK_LEFT))
					{
						trigger = true;
						interbal = 0;
						return Command::LSTEP;

					}
				}

				else
				{
					//�R�}���h�͂Ȃ�


					return Command::NOCOMMAND;
				}
			}
		}

		return Command::NOCOMMAND;

		break;




	default:
		break;




	//case Command::RFUJIN:
	//	//�Ō�ɓ��͂��ꂽ�͉̂E������

	//	if (timer[numI] - timer[numI - 1] < LIMIT_FRAME)
	//	{

	//		if (history[numI - 1] == scastI((STICKDIRECTION::STICK_DOWN)))
	//		{

	//			//���̑O�ɓ��͂��ꂽ�̂��E��������
	//			if (timer[numI - 1] - timer[numI - 2] > LIMIT_FRAME)return Command::NOCOMMAND;

	//			if (history[numI - 2] == scastI(STICKDIRECTION::STICK_RIGHT_DOWN))
	//			{
	//				if (timer[numI - 2] - timer[numI - 3] > LIMIT_FRAME)return Command::NOCOMMAND;

	//				if (history[numI - 3] == scastI(STICKDIRECTION::STICK_RIGHT))
	//				{
	//					//�X�ɂ��̑O�ɓ��͂��ꂽ�̂���������
	//					if (distance > 0)
	//					{
	//						//�v���C���[�͉E�������Ă���
	//						trigger = true;
	//						interbal = 0;
	//						return Command::RFUJIN;
	//					}
	//					if (distance < 0)
	//					{
	//						//�v���C���[�͍��������Ă���
	//						trigger = true;
	//						interbal = 0;
	//						return Command::LFUJIN;
	//					}
	//				}
	//			}

	//			else
	//			{
	//				//�R�}���h�͂Ȃ�
	//				return Command::NOCOMMAND;
	//			}
	//		}
	//	}
	//	break;
	//case Command::LFUJIN:
	//	//�Ō�ɓ��͂��ꂽ�̂̓j���[�g����������

	//	if (timer[numI] - timer[numI - 1] < LIMIT_FRAME)
	//	{

	//		if (history[numI - 1] == scastI(STICKDIRECTION::STICK_DOWN))
	//		{
	//			//���̑O�ɓ��͂��ꂽ�̂�����������

	//			if (timer[numI - 1] - timer[numI - 2] > LIMIT_FRAME)return Command::NOCOMMAND;

	//			if (history[numI - 2] == scastI(STICKDIRECTION::STICK_LEFT_DOWN))
	//			{
	//				if (timer[numI - 2] - timer[numI - 3] > LIMIT_FRAME)return Command::NOCOMMAND;

	//				if (history[numI - 3] == scastI(STICKDIRECTION::STICK_LEFT))
	//				{
	//					//�X�ɂ��̑O�ɓ��͂��ꂽ�̂���������
	//					if (distance < 0)
	//					{
	//						//�v���C���[�͉E�������Ă���
	//						trigger = true;
	//						interbal = 0;
	//						return Command::RFUJIN;
	//					}
	//					if (distance > 0)
	//					{
	//						//�v���C���[�͍��������Ă���
	//						trigger = true;
	//						interbal = 0;
	//						return Command::LFUJIN;
	//					}
	//				}
	//			}
	//		}
	//		else
	//		{
	//			//�R�}���h�͂Ȃ�
	//			return Command::NOCOMMAND;
	//		}
	//		break;
	//	}
	//

	}
	trigger = true;
	//�R�}���h�͂Ȃ�
	return Command::NOCOMMAND;
	
}


void CommandList::Reset()
{
	command_timer = 0.0f;
}


// Command Analysis Class
void ComParse::init(COMMAND_DATA* pData)
{
	pStart = pCurrent = pData;
	interval = 0;
}


bool ComParse::operator()(char trg)
{
	interval--;
	if (interval <= 0)
	{
		pCurrent = pStart;
		interval = 0;
	}

	if ((short)trg)
	{
		if ((short)trg == (short)pCurrent->trg)
		{
			interval = pCurrent->interval;
			if (interval == 0) return true;
			pCurrent++;
		}
	}
	return false;
}

void Button_log::update(int hist, CommandList* clist)
{
	buttonlog.push_back(hist);
	if (buttonlog.size() > 300)
	{
		buttonlog.pop_front();
	}

	if (hist != STICKDIRECTION::STICK_N)
	{
		clist->Reset();

	}
}
