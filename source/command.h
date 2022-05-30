#pragma once
#include "assain.h"
#include <vector>
#include <list>
#include<array>
#include<memory>
//#include"Common.h"

constexpr float TIMER_LIMIT = 1.5f;				//�R�}���h�^�C�}�[�����̐��ȉ��Ȃ�R�}���h������J�n����
constexpr float timer_max_command = 6.0f;		//���̐��ȉ��Ȃ�R�}���h�^�C�}�[�𑝂₷


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






//�R�}���h����p��
enum class Command : int
{
	NOCOMMAND = 0,	//�R�}���h�͖���
	RHURF,			//�O�����R�}���h236(�E�����̏ꍇ)
	LHURF,			//������R�}���h214(�E�����̏ꍇ)
	RFUJIN,			//���_�i�����j�R�}���h6N23(�E�����̏ꍇ)
	LFUJIN,			//�t���_�i�����j�R�}���h4N21(�E�����̏ꍇ)
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
	int						lastinput;		//�����ł��炤�ŏI����
	int						command_start;	//�Ȃɂ���

	int old = 0;//1�t���[���O�̓���	
	int oldt = 0;//1�t���[���O�̓���	


	Command					state;			//���ݔ��肵�Ă���R�}���h
	int						numI;			//�R�}���h�̓��X�g�̉������画�肷�邩��ۑ�����ϐ�
public:
	bool trigger = false;					//�R�}���h�𔻒肷�邩�ǂ����̃g���K�[
	float command_timer = 0.0f;				//�R�}���h�P�\����

	std::array<int, MAX_BUFFER >		commandbuf = { 0 };	//�R�}���h���͉�̓o�b�t�@
	int interbal = 0;
	std::list <int>		list = { 0 };	//�R�}���h���͉�̓��X�g
	std::list <int>		timer_list = { 0 };	//�R�}���h���͉�̓��X�g

	std::array<int, MAX_BUFFER >		history = { 0 };	//list�̃f�[�^�������Ŏ󂯎��
	std::array<int, MAX_BUFFER >		historytimer = { 0 };	//list�̃f�[�^�������Ŏ󂯎��

	std::array<int, MAX_BUFFER >		timer = { 0 };	//�^�C�}�[
	void checklist(int time);

	 Command CommandCheack(int last, int* hist,int* histtimer, float distans,int timer);
	void Reset();
};


//Command log Class
class Stick_log
{
private:



public:


	int old = 0;//1�t���[���O�̓���	
	int oldt = 0;//1�t���[���O�̓���	

	Stick_log() {}
	~Stick_log() {}
	int old_stick = 0;

	void update(int hist, CommandList* clist,int frame);

	std::list<int> sticklog; //���̓X�^�b�N
	std::list<int> timerlog; //���̓X�^�b�N
};



class Button_log
{
private:



public:




	Button_log() {}
	~Button_log() {}


	void update(int hist, CommandList* clist);

	std::list<int> buttonlog; //���̓X�^�b�N

};



