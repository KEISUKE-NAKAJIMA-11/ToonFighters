#pragma once
#include "assain.h"
#include <vector>
#include <list>
#include<array>
#include<memory>
//#include"Common.h"

constexpr float TIMER_LIMIT = 1.5f;				//コマンドタイマーがこの数以下ならコマンド判定を開始する
constexpr float timer_max_command = 6.0f;		//この数以下ならコマンドタイマーを増やす


constexpr int LIMIT_FRAME = 20;
constexpr int INTERVAL = 60;
constexpr int MAX_BUFFER = 20;
#define		scastI				static_cast<int>



enum STICKDIRECTION : int
{
	STICK_N,
	STICK_UP,
	STICK_RIGHT_UP,
	STICK_RIGHT,
	STICK_RIGHT_DOWN,
	STICK_DOWN,
	STICK_LEFT_DOWN,
	STICK_LEFT,
	STICK_LEFT_UP,
};

enum BUTTONNUM : int
{
	BUTTON_NO,
	BUTTON_A,
	BUTTON_B,
	BUTTON_X,
	BUTTON_Y,
	BTN_LEFT_THUMB,
	BTN_RIGHT_THUMB,
	BTN_LEFT_SHOULDER,
	BTN_RIGHT_SHOULDER,
	BTN_LEFT_TRIGGER,
	BTN_RIGHT_TRIGGER,
};






//コマンド判定用列挙
enum class Command : int
{
	NOCOMMAND = 0,	//コマンドは無い
	RHURF,			//前方向コマンド236(右向きの場合)
	LHURF,			//後方向コマンド214(右向きの場合)
	RFUJIN,			//風神（昇竜）コマンド6N23(右向きの場合)
	LFUJIN,			//逆風神（昇竜）コマンド4N21(右向きの場合)
	RSTEP,
	LSTEP,

	END,
};


struct COMMAND_DATA
{
	char	trg;					// command key
	int	interval;				    // interval
};


// Command Analysis Class
class ComParse
{
private:
	COMMAND_DATA* pStart;//skills started.
	COMMAND_DATA* pCurrent;//skills current.
	int				interval;
public:
	void init(COMMAND_DATA* pData);
	bool operator()(char trg);
};





class CommandList
{
private:
	int						lastinput;		//引数でもらう最終入力
	int						command_start;	//なにこれ

	int old = 0;//1フレーム前の入力	
	int oldt = 0;//1フレーム前の入力	


	Command					state;			//現在判定しているコマンド
	int						numI;			//コマンドはリストの何処から判定するかを保存する変数
public:
	bool trigger = false;					//コマンドを判定するかどうかのトリガー
	float command_timer = 0.0f;				//コマンド猶予時間

	std::array<int, MAX_BUFFER >		commandbuf = { 0 };	//コマンド入力解析バッファ
	int interbal = 0;
	std::list <int>		list = { 0 };	//コマンド入力解析リスト
	std::list <int>		timer_list = { 0 };	//コマンド入力解析リスト

	std::array<int, MAX_BUFFER >		history = { 0 };	//listのデータを引数で受け取る
	std::array<int, MAX_BUFFER >		historytimer = { 0 };	//listのデータを引数で受け取る

	std::array<int, MAX_BUFFER >		timer = { 0 };	//タイマー
	void checklist(int time);

	 Command CommandCheack(int last, int* hist,int* histtimer, float distans,int timer);
	void Reset();
};


//Command log Class
class Stick_log
{
private:



public:


	int old = 0;//1フレーム前の入力	
	int oldt = 0;//1フレーム前の入力	

	Stick_log() {}
	~Stick_log() {}
	int old_stick = 0;

	void update(int hist, CommandList* clist,int frame);

	std::list<int> sticklog; //入力スタック
	std::list<int> timerlog; //入力スタック
};



class Button_log
{
private:



public:




	Button_log() {}
	~Button_log() {}


	void update(int hist, CommandList* clist);

	std::list<int> buttonlog; //入力スタック

};



