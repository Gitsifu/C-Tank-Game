#include<iostream>
#include<stdlib.h>
#include<tchar.h>
#include<Windows.h>
#include<time.h>
#include<conio.h>
using namespace std;

HANDLE Mutex = CreateMutex(NULL, FALSE, NULL);//�������

int GameOver = 0;
int level = 0;
int map[23][23];
//̹�����࣬NormalΪ���̹��
#define Normal 0
#define Red 1
#define Blue 2
#define Green 3
//����ĺ궨��
#define Up 0
#define Down 1
#define Left 2
#define Right 3
//��ͼ��ǵĺ궨��
#define Empty 0
#define Player 1
#define PlayerBullet 2
#define EnemyBullet 3
#define Enemy 4

int Kill;
int KillRed;
int KillGreen;
int EnemyExist;

void SetPos(int i, int j)//�趨���λ��
{
	COORD pos = { i, j };
	HANDLE Out = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(Out, pos);
}

void HideCurSor(void)//���ع��
{
	CONSOLE_CURSOR_INFO info = { 1, 0 };
	HANDLE Out = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorInfo(Out, &info);
}

int sharp[4][12] =
{
	{ 0, 1, 1, 0, 1, 1, 1, 2, 2, 0, 2, 2 },
	{ 0, 0, 0, 2, 1, 0, 1, 1, 1, 2, 2, 1 },
	{ 0, 1, 0, 2, 1, 0, 1, 1, 2, 1, 2, 2 },
	{ 0, 0, 0, 1, 1, 1, 1, 2, 2, 0, 2, 1 },
};//��������������̹�˸����������״��Ϣ

DWORD WINAPI Bulletfly(LPVOID lpParameter);//�ӵ���������
void Updata();//���½�����Ϣ��������

class Tank//̹����
{
private:
	int Direction;//����
	int hotpoint[2];//���
	int Speed;//�ٶ�
	int FirePower;//����
public:
	Tank(int dir, int hot1, int hot2, int typ, int spe, int firepow)//���캯��
	{
		Direction = dir;
		hotpoint[0] = hot1;
		hotpoint[1] = hot2;
		Type = typ;
		Speed = spe;
		FirePower = firepow;
	}
	int Type;//̹�˵����ࣨ����궨�壩
	int ID;//̹����MAP�еı�ǣ�����궨�壩
	int FireEnable;//�Ƿ���Կ���
	int Life;//����ֵ
	void Running();//���к���
	int Judge(int x, int y, int ID);//�ж��Ƿ���Ի���̹��
	void DrawTank();//�ػ�̹��
	void Redraw();//����̹��
	int GetSpeed()//��ȡ�ٶ�
	{
		return Speed;
	}
	int GetFire()//��ȡ����
	{
		return FirePower;
	}
	int GetDirection()//��ȡ����
	{
		return Direction;
	}
	int GetHotX()//��ȡ�������
	{
		return hotpoint[0];
	}
	int GetHotY()
	{
		return hotpoint[1];
	}
	void IncreaseFire()//����+
	{
		FirePower++;
	}
	void IncreaseSpeed()//�ٶ�+
	{
		Speed++;
	}
	void ChangeDirection(int newD)//�ı䷽��
	{
		Direction = newD;
	}
	void ChangePos(int x, int y)//�ı���
	{
		hotpoint[0] = x;
		hotpoint[1] = y;
	}
};

Tank player(Right, 0, 0, Normal, 1, 1);//���
Tank enemy(Left, 20, 0, Red, 1, 1);//����

void Tank::DrawTank()//����̹��
{
	int i;
	int nx, ny;
	if (Type == Red)
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
	else if (Type == Blue)
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_BLUE);
	else if (Type == Green)
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
	else if (Type == Normal)
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	for (i = 0; i<6; i++)
	{
		nx = hotpoint[0] + sharp[Direction][i * 2];
		ny = hotpoint[1] + sharp[Direction][i * 2 + 1];
		SetPos((ny + 1) * 2, nx + 1);//����sharp��������ڵ�x,y������״
		map[nx][ny] = ID;
		cout << "��";
	}
}

void Tank::Redraw()//����̹�ˣ�ԭ��ͬ��
{
	int i;
	int nx, ny;
	for (i = 0; i<6; i++)
	{
		nx = hotpoint[0] + sharp[Direction][i * 2];
		ny = hotpoint[1] + sharp[Direction][i * 2 + 1];
		map[nx][ny] = Empty;
		SetPos((ny + 1) * 2, nx + 1);
		cout << " ";
	}
}

int Tank::Judge(int x, int y, int dir)//�жϵ�ǰ�Ƿ���Ի���̹��
{
	int i;
	int nx, ny;
	for (i = 0; i<6; i++)
	{
		nx = x + sharp[dir][i * 2];
		ny = y + sharp[dir][i * 2 + 1];
		if (nx<0 || nx >= 23 || ny<0 || ny >= 23 || map[nx][ny] != Empty)//���ܻ��ƣ�����1
			return 1;
	}
	return 0;
}


void Tank::Running()//̹�����к���
{
	int newD;
	//̹�˵�����
	while (1)
	{
		if (Life == 0)
		{
			EnemyExist = 0;//���˲�����
			return;
		}
		if (GameOver == 1)
			return;
		if (FireEnable == 1 && GameOver == 0)//������Կ���
		{
			WaitForSingleObject(Mutex, INFINITE);//�߳�ӵ�л������
			FireEnable = 0;//��Ϊ���ɿ���
			HANDLE bullet = CreateThread(NULL, 0, Bulletfly, &ID, 0, NULL);//�����ӵ��߳�
			CloseHandle(bullet);
			ReleaseMutex(Mutex);//�ͷŻ������
			Sleep(100);
		}
		WaitForSingleObject(Mutex, INFINITE);//�߳�ӵ�л������
		srand((int)time(0));
		newD = rand() % 4;

		if (newD == Up)//������µķ������»���̹��
		{
			Redraw();
			if (Judge(hotpoint[0] - 1, hotpoint[1], newD) == 0)
			{
				hotpoint[0]--;
				Direction = newD;
			}
			else
			{
				if (Judge(hotpoint[0], hotpoint[1], newD) == 0)
					Direction = newD;
			}
		}
		else if (newD == Down)
		{
			Redraw();
			if (Judge(hotpoint[0] + 1, hotpoint[1], newD) == 0)
			{
				hotpoint[0]++;
				Direction = newD;
			}
			else
			{
				if (Judge(hotpoint[0], hotpoint[1], newD) == 0)
					Direction = newD;
			}
		}
		else if (newD == Left)
		{
			Redraw();
			if (Judge(hotpoint[0], hotpoint[1] - 1, newD) == 0)
			{
				hotpoint[1]--;
				Direction = newD;
			}
			else
			{
				if (Judge(hotpoint[0], hotpoint[1], newD) == 0)
					Direction = newD;
			}
		}
		else if (newD == Right)
		{
			Redraw();
			if (Judge(hotpoint[0], hotpoint[1] + 1, newD) == 0)
			{
				hotpoint[1]++;
				Direction = newD;
			}
			else
			{
				if (Judge(hotpoint[0], hotpoint[1], newD) == 0)
					Direction = newD;
			}
		}
		if (GameOver == 0 && Life != 0)
			DrawTank();
		ReleaseMutex(Mutex);//�ͷŻ������
		Sleep(500 - 80 * Speed);
	}
}

/*********************�ӵ��̺߳���*******************/
DWORD WINAPI Bulletfly(LPVOID lpParameter)
{
	int *ID = (int *)lpParameter;//ID������ȡ�����ӵ�̹�˵�ID
	int Pos[2];//�ӵ����
	int direction;
	int Speed;
	int type;
	int hit = 0;//���б��
	int oldx, oldy;//�ɻ��
	int flag = 0;//�ӵ��Ƿ����ƶ��ı��
	if (*ID == Player)//��������̹��
	{
		type = PlayerBullet;
		direction = player.GetDirection();
		Speed = player.GetFire();
		Pos[0] = player.GetHotX();
		Pos[1] = player.GetHotY();
	}
	else if (*ID == Enemy)//����ǵ���̹��
	{
		type = EnemyBullet;
		direction = enemy.GetDirection();
		Speed = enemy.GetFire();
		Pos[0] = enemy.GetHotX();
		Pos[1] = enemy.GetHotY();
	}
	if (direction == Up)//����̹�˵�λ�úͷ���ȷ���ӵ��ĳ�ʼ����
	{
		Pos[0]--;
		Pos[1]++;
	}
	else if (direction == Down)
	{
		Pos[0] += 3;
		Pos[1]++;
	}
	else if (direction == Left)
	{
		Pos[0]++;
		Pos[1]--;
	}
	else if (direction == Right)
	{
		Pos[0]++;
		Pos[1] += 3;
	}
	//�ӵ�������
	while (1)
	{
		WaitForSingleObject(Mutex, INFINITE);//�������ע���ˡ���������
		if (flag == 1 && hit != 1)//����ԭλ��
		{
			map[oldx][oldy] = Empty;
			SetPos((oldy + 1) * 2, oldx + 1);
			cout << " ";
		}
		if (GameOver == 1)
			return 0;
		if (hit == 1 || Pos[0]<0 || Pos[0]>22 || Pos[1]<0 || Pos[1]>22)//�������
		{
			ReleaseMutex(Mutex);
			Sleep(500);
			if (type == PlayerBullet)
				player.FireEnable = 1;
			else if (type = EnemyBullet)
				enemy.FireEnable = 1;
			break;
		}
		switch (map[Pos[0]][Pos[1]])//�ӵ�������MAP�ı��
		{

		case Empty://����ǿ�λ�þͻ����ӵ�
			map[Pos[0]][Pos[1]] = type;
			SetPos((Pos[1] + 1) * 2, Pos[0] + 1);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			cout << "��";
			break;
		case Player://��������λ��
			if (type != PlayerBullet)
			{
				player.Life--;//��������
				if (player.Life <= 0)
					GameOver = 1;
			}
			Updata();
			hit = 1;
			break;
		case Enemy://����ǵ���λ��
			if (type != PlayerBullet)
				hit = 1;
			else
			{
				hit = 1;
				Kill++;
				if (Kill % 20 == 0 && player.Life<5)//��ɱ��++
					player.Life++;
				if (enemy.Type == Red)//�����ɱ��̹��
				{
					KillRed++;
					if (KillRed % 10 == 0 && player.GetFire()<5)
						player.IncreaseFire();
				}
				if (enemy.Type == Green)///�����ɱ��̹��
				{
					KillGreen++;
					if (KillGreen % 10 == 0 && player.GetSpeed()<5)
						player.IncreaseSpeed();
				}
				enemy.Redraw();//��������
				enemy.Life = 0;//��������
			}
			Updata();
			break;
		}
		oldx = Pos[0];
		oldy = Pos[1];
		if (direction == Up)//�ӵ��ƶ�
			Pos[0]--;
		else if (direction == Down)
			Pos[0]++;
		else if (direction == Left)
			Pos[1]--;
		else if (direction == Right)
			Pos[1]++;
		ReleaseMutex(Mutex);
		flag = 1;
		Sleep(60 - 10 * Speed);
	}
	return 0;
}


/*************************�����̺߳���***************************/
DWORD WINAPI TankRuning(LPVOID lpParameter)
{
	Sleep(400);
	int Pos;
	int Start[2];//������ʼ��ַ
	int typ;
	int fire;
	int spe;
	while (1)
	{
		if (GameOver == 1)
			return 0;
		srand((int)time(0));//�����������ʼ��ַ
		Pos = rand() % 4;
		if (Pos == 0)
		{
			Start[0] = 2;
			Start[0] = 2;
		}
		else if (Pos == 1)
		{
			Start[0] = 2;
			Start[1] = 18;
		}
		else if (Pos == 2)
		{
			Start[0] = 18;
			Start[1] = 2;
		}
		else if (Pos == 3)
		{
			Start[0] = 18;
			Start[1] = 18;
		}
		if (player.Judge(Start[0], Start[1], Down) == 0)
			break;
	}
	WaitForSingleObject(Mutex, INFINITE);
	srand((int)time(0));
	typ = rand() % 3 + 1;//��������˵�����
	if (typ == Blue)
	{
		spe = 1 + level;
		fire = 1 + level;
	}
	else if (typ == Red)
	{
		spe = 1 + level;
		fire = 3 + level;
	}
	else if (typ == Green)
	{
		spe = 3 + level;
		fire = 1 + level;
	}
	enemy = Tank(Down, Start[0], Start[1], typ, spe, fire);//�������ɵ���̹��
	enemy.ID = Enemy;
	enemy.Life = 1;
	enemy.FireEnable = 1;
	ReleaseMutex(Mutex);
	enemy.Running();
	return 0;
}


void Init()//��ʼ������
{
	Kill = 0;
	KillRed = 0;
	KillGreen = 0;
	player = Tank(Left, 0, 0, Normal, 1, 1);
	enemy = Tank(Left, 0, 0, Red, 1, 1);
	player.Life = 2;
	player.FireEnable = 1;
	enemy.Life = 0;
	enemy.FireEnable = 1;
	player.ID = Player;
	enemy.ID = Enemy;
	EnemyExist = 0;
}

void Updata()//���½�����Ϣ
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	int i;
	SetPos(53, 0);
	cout << "����ֵ��";
	SetPos(53, 1);
	for (i = 0; i<5; i++)
	{
		if (i<player.Life)
			cout << "��";
		else
			cout << " ";
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
	SetPos(53, 3);
	cout << "�ƶ��ٶȣ�";
	SetPos(53, 4);
	for (i = 0; i<5; i++)
	{
		if (i<player.GetSpeed())
			cout << "��";
		else
			cout << " ";
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
	SetPos(53, 5);
	cout << "������";
	SetPos(53, 6);
	for (i = 0; i<5; i++)
	{
		if (i<player.GetFire())
			cout << "��";
		else
			cout << " ";
	}
	SetPos(53, 8);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	cout << "ɱ������" << Kill;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
	SetPos(53, 9);
	cout << "ɱ����̹�ˣ�" << KillRed;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
	SetPos(53, 10);
	cout << "ɱ����̹�ˣ�" << KillGreen;

}
void DrawMap()//������
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	system("cls");
	int i;
	for (i = 0; i<25; i++)
	{
		SetPos(i * 2, 0);
		cout << "��";
	}
	for (i = 1; i<25; i++)
	{
		SetPos(0, i);
		cout << "��";
		SetPos(24 * 2, i);
		cout << "��";
	}
	for (i = 0; i<25; i++)
	{
		SetPos(i * 2, 24);
		cout << "��";
	}

	Updata();

}

void Welcome()//��ӭ����
{
	int x;
	system("cls");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	SetPos(10, 5);
	cout << "������������������������������������������������";
	SetPos(10, 6);
	cout << "��             ̹�˴�ս������                 ��";
	SetPos(10, 7);
	cout << "������������������������������������������������";
	SetPos(10, 8);
	cout << "��       ������ƶ����ո�����               ��";
	SetPos(10, 9);
	cout << "��       ���˷�Ϊ3�֣���ɫΪ��ͨ����          ��";
	SetPos(10, 10);
	cout << "��     ��ɫ���˸����٣���ɫ���˸߻�����       ��";
	SetPos(10, 11);
	cout << "�� ÿɱ��10����̹�ˣ�����������(����弶��  ��";
	SetPos(10, 12);
	cout << "�� ÿɱ��10����̹�ˣ�����ƶ������(����弶����";
	SetPos(10, 13);
	cout << "��   ÿɱ��20��̹�ˣ��������+1��������   ��";
	SetPos(10, 14);
	cout << "������������������������������������������������";
	SetPos(10, 15);
	cout << "��            ��Ȩ���У����ĸ�                ��";
	SetPos(10, 16);
	cout << "��                                   ����ؾ� ��";
	SetPos(10, 17);
	cout << "��           ��1-3ѡ���Ѷ�                    ��";
	SetPos(10, 18);
	cout << "������������������������������������������������";
	while (1)
	{
		x = _getch();
		if (x <= '3'&&x >= '1')
			break;
	}
	level = x - '0' - 1;
}

int _tmain(int argc, TCHAR* argv[]) 
{
	Init();
	HideCurSor();
	Welcome();
	DrawMap();
	HANDLE temp;
	int newD;
	player.DrawTank();
	while (GameOver == 0)
	{
		if (GetAsyncKeyState(VK_UP))//������
		{
			WaitForSingleObject(Mutex, INFINITE);
			newD = Up;
			player.Redraw();
			if (player.Judge(player.GetHotX() - 1, player.GetHotY(), newD) == 0)//�ƶ����̹�ˣ�ԭ��͵��˺���һ��
			{
				player.ChangePos(player.GetHotX() - 1, player.GetHotY());
				player.ChangeDirection(newD);
			}
			else
			{
				if (player.Judge(player.GetHotX(), player.GetHotY(), newD) == 0)
					player.ChangeDirection(newD);
			}
			if (GameOver == 0)
				player.DrawTank();
			ReleaseMutex(Mutex);
			Sleep(200 - player.GetSpeed() * 20);//�����ӳ٣��������̹�˵��ٶ�
		}
		else if (GetAsyncKeyState(VK_DOWN))//�����£�ͬ��
		{
			WaitForSingleObject(Mutex, INFINITE);
			newD = Down;
			player.Redraw();
			if (player.Judge(player.GetHotX() + 1, player.GetHotY(), newD) == 0)
			{
				player.ChangePos(player.GetHotX() + 1, player.GetHotY());
				player.ChangeDirection(newD);
			}
			else
			{
				if (player.Judge(player.GetHotX(), player.GetHotY(), newD) == 0)
					player.ChangeDirection(newD);
			}
			if (GameOver == 0)
				player.DrawTank();
			ReleaseMutex(Mutex);
			Sleep(200 - player.GetSpeed() * 20);
		}
		else if (GetAsyncKeyState(VK_RIGHT))//�����ң�ͬ��
		{
			WaitForSingleObject(Mutex, INFINITE);
			newD = Right;
			player.Redraw();
			if (player.Judge(player.GetHotX(), player.GetHotY() + 1, newD) == 0)
			{
				player.ChangePos(player.GetHotX(), player.GetHotY() + 1);
				player.ChangeDirection(newD);
			}
			else
			{
				if (player.Judge(player.GetHotX(), player.GetHotY(), newD) == 0)
					player.ChangeDirection(newD);
			}
			if (GameOver == 0)
				player.DrawTank();
			ReleaseMutex(Mutex);
			Sleep(200 - player.GetSpeed() * 20);
		}
		else if (GetAsyncKeyState(VK_LEFT))//������ͬ��
		{
			WaitForSingleObject(Mutex, INFINITE);
			newD = Left;
			player.Redraw();
			if (player.Judge(player.GetHotX(), player.GetHotY() - 1, newD) == 0)
			{
				player.ChangePos(player.GetHotX(), player.GetHotY() - 1);
				player.ChangeDirection(newD);
			}
			else
			{
				if (player.Judge(player.GetHotX(), player.GetHotY(), newD) == 0)
					player.ChangeDirection(newD);
			}
			if (GameOver == 0)
				player.DrawTank();
			ReleaseMutex(Mutex);
			Sleep(110 - player.GetSpeed() * 10);
		}
		else if (GetAsyncKeyState(VK_SPACE))//�����ո񣬷����ӵ�
		{
			WaitForSingleObject(Mutex, INFINITE);
			if (player.FireEnable == 1)//������Է���
			{
				HANDLE bullet = CreateThread(NULL, 0, Bulletfly, &(player.ID), 0, NULL);//��������ӵ�����
				CloseHandle(bullet);
				player.FireEnable = 0;
			}
			ReleaseMutex(Mutex);
		}
		if (EnemyExist == 0 && GameOver == 0)//������˲����������µ���
		{
			WaitForSingleObject(Mutex, INFINITE);
			EnemyExist = 1;
			temp = CreateThread(NULL, 0, TankRuning, NULL, 0, NULL);//���������߳�
			CloseHandle(temp);
			ReleaseMutex(Mutex);
		}
	}
	system("cls");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_BLUE);
	SetPos(20, 10);
	cout << "��Ϸ����" << endl;
	SetPos(20, 11);
	cout << "ɱ������" << Kill;
	SetPos(20, 12);
	cout << "ɱ����̹��" << KillRed;
	SetPos(20, 13);
	cout << "ɱ����̹��" << KillGreen << endl;
	return 0;
}

