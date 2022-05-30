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
	//		コマンド確認関数
	//=========================================
	//・入力の履歴を最新のものからさかのぼっていき、
	//  過去にコマンドが入力されているか動かを判定する
	//---------------------------------------------------------------

	trigger = false;
	state = Command::NOCOMMAND;

	//Listの何処を見たかを保存する
	numI = 0;

	//コマンド判定タイマーが一定の値以内ならコマンド判定を取るようにする(コマンド受付猶予)
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

		
	

	//引数で最後に入力されたボタンをもらっている(攻撃用ボタンのX、Y、B、RBボタンなど)
	lastinput = last;

	//Listの中身を後ろから見ていく
	for (int i = MAX_BUFFER - 1; i > -1; i--)
	{
		if (history[i] != -1)
		{
			//最後に入力されたものと違うものであれば
			{

				if (history[i] != lastinput)
				{
					 if (history[i] == scastI(STICKDIRECTION::STICK_RIGHT))
					{
						//入力されたのが右だった
						state = Command::RHURF;
						numI = i;
						//Reset();
						break;
					}
					else if (history[i] == scastI(STICKDIRECTION::STICK_LEFT))
					{
						//入力されたのが左だった
						state = Command::LHURF;
						numI = i;

						break;
					}
					else
					{
						//コマンドは無かった
						return Command::NOCOMMAND;
					}
				}
			}
			
		}
	}

	if (numI < 2)
	{
		//残っているのが2以下ならコマンドは絶対に成立しないのでここで区切る
		return Command::NOCOMMAND;
	}

	//Listの中身を判定した場所から見ていく
	//※コマンドは正しく入力されていればListの中で「↓」「↓→」「→」と並んでいるはずなので
	//一つずつ後ろを参照していく
	switch (state)
	{
	case Command::RHURF:
		//最後に入力されたのは右だった

		if (time - timer[numI] < LIMIT_FRAME)
		{

			if (timer[numI] - timer[numI - 1] < LIMIT_FRAME)
			{

				if (history[numI - 1] == scastI((STICKDIRECTION::STICK_RIGHT_DOWN)))
				{


					if (timer[numI - 1] - timer[numI - 2] > LIMIT_FRAME) return Command::NOCOMMAND;

					//その前に入力されたのが右下だった
					if (history[numI - 2] == scastI(STICKDIRECTION::STICK_DOWN))
					{

						//更にその前に入力されたのが下だった

						if (numI - 3 < 0|| numI - 2 < 0)return Command::NOCOMMAND;

						if (timer[numI - 2] - timer[numI - 3] > (LIMIT_FRAME+40))
						{
							if (distance > 0)
							{
								//プレイヤーは右を向いている
								trigger = true;

								interbal = 0;

								return Command::RHURF;
							}
							if (distance < 0)
							{
								//プレイヤーは左を向いている
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
									//プレイヤーは右を向いている
									trigger = true;

									interbal = 0;

									return Command::RFUJIN;
								}
								if (distance < 0)
								{
									//プレイヤーは左を向いている
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
					// その前に入力されたのが右だった
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
					//コマンドはない
					return Command::NOCOMMAND;
				}
			}
		}

		return Command::NOCOMMAND;

		break;

	case Command::LHURF:
		//最後に入力されたのは左だった
		if (time - timer[numI] < LIMIT_FRAME)
		{

			if (timer[numI] - timer[numI - 1] < LIMIT_FRAME)
			{

				if (history[numI - 1] == scastI(STICKDIRECTION::STICK_LEFT_DOWN))
				{
					//その前に入力されたのが左下だった
					if (numI - 1 < 0 || numI - 2 < 0)return Command::NOCOMMAND;
					if (timer[numI - 1] - timer[numI - 2] > LIMIT_FRAME)return Command::NOCOMMAND;

					if (history[numI - 2] == scastI(STICKDIRECTION::STICK_DOWN))
					{
						if (timer[numI - 2] - timer[numI - 3] > (LIMIT_FRAME+40))
						{
							//更にその前に入力されたのが下だった
							if (distance > 0)
							{
								//プレイヤーは右を向いている
								trigger = true;
								interbal = 0;
								return Command::LHURF;
							}
							if (distance < 0)
							{
								//プレイヤーは左を向いている
								trigger = true;
								interbal = 0;
								return Command::RHURF;
							}
						}
						else if (history[numI - 3] == scastI(STICKDIRECTION::STICK_LEFT_DOWN))
						{
							//更にその前に入力されたのが下だった
							if (numI - 3 < 0 || numI - 4 < 0)return Command::NOCOMMAND;
							if (timer[numI - 3] - timer[numI - 4] > LIMIT_FRAME+40)return Command::NOCOMMAND;
							if (history[numI - 4] == scastI(STICKDIRECTION::STICK_LEFT))
							{
								if (distance > 0)
								{
									//プレイヤーは右を向いている
									trigger = true;
									interbal = 0;
									return Command::LFUJIN;
								}
								if (distance < 0)
								{
									//プレイヤーは左を向いている
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
					// その前に入力されたのが右だった

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
					//コマンドはない


					return Command::NOCOMMAND;
				}
			}
		}

		return Command::NOCOMMAND;

		break;




	default:
		break;




	//case Command::RFUJIN:
	//	//最後に入力されたのは右だった

	//	if (timer[numI] - timer[numI - 1] < LIMIT_FRAME)
	//	{

	//		if (history[numI - 1] == scastI((STICKDIRECTION::STICK_DOWN)))
	//		{

	//			//その前に入力されたのが右下だった
	//			if (timer[numI - 1] - timer[numI - 2] > LIMIT_FRAME)return Command::NOCOMMAND;

	//			if (history[numI - 2] == scastI(STICKDIRECTION::STICK_RIGHT_DOWN))
	//			{
	//				if (timer[numI - 2] - timer[numI - 3] > LIMIT_FRAME)return Command::NOCOMMAND;

	//				if (history[numI - 3] == scastI(STICKDIRECTION::STICK_RIGHT))
	//				{
	//					//更にその前に入力されたのが下だった
	//					if (distance > 0)
	//					{
	//						//プレイヤーは右を向いている
	//						trigger = true;
	//						interbal = 0;
	//						return Command::RFUJIN;
	//					}
	//					if (distance < 0)
	//					{
	//						//プレイヤーは左を向いている
	//						trigger = true;
	//						interbal = 0;
	//						return Command::LFUJIN;
	//					}
	//				}
	//			}

	//			else
	//			{
	//				//コマンドはない
	//				return Command::NOCOMMAND;
	//			}
	//		}
	//	}
	//	break;
	//case Command::LFUJIN:
	//	//最後に入力されたのはニュートラルだった

	//	if (timer[numI] - timer[numI - 1] < LIMIT_FRAME)
	//	{

	//		if (history[numI - 1] == scastI(STICKDIRECTION::STICK_DOWN))
	//		{
	//			//その前に入力されたのが左下だった

	//			if (timer[numI - 1] - timer[numI - 2] > LIMIT_FRAME)return Command::NOCOMMAND;

	//			if (history[numI - 2] == scastI(STICKDIRECTION::STICK_LEFT_DOWN))
	//			{
	//				if (timer[numI - 2] - timer[numI - 3] > LIMIT_FRAME)return Command::NOCOMMAND;

	//				if (history[numI - 3] == scastI(STICKDIRECTION::STICK_LEFT))
	//				{
	//					//更にその前に入力されたのが下だった
	//					if (distance < 0)
	//					{
	//						//プレイヤーは右を向いている
	//						trigger = true;
	//						interbal = 0;
	//						return Command::RFUJIN;
	//					}
	//					if (distance > 0)
	//					{
	//						//プレイヤーは左を向いている
	//						trigger = true;
	//						interbal = 0;
	//						return Command::LFUJIN;
	//					}
	//				}
	//			}
	//		}
	//		else
	//		{
	//			//コマンドはない
	//			return Command::NOCOMMAND;
	//		}
	//		break;
	//	}
	//

	}
	trigger = true;
	//コマンドはない
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
