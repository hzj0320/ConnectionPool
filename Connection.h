#pragma once
//MySQL���ݿ����
#include <mysql.h>
#include <string>
#include <ctime>
using namespace std;
class Connection {
public:
	Connection();//��ʼ�����ݿ�����
	~Connection();//�ͷ����ݿ�����
	//�������ݿ�
	bool connect(string ip, unsigned short port, string username, string password, string dbname);
	//���²���
	bool update(string sql);
	//��ѯ����
	MYSQL_RES* query(string sql);

	//ˢ��������ʼ����ʱ��
	void refreshAliveTime() {
		_alivetime = clock();
	}
	//���ش��ʱ��
	clock_t getAliveTime() {
		return clock() - _alivetime;
	}
private:
	MYSQL* _conn;//��ʾ��MySQL��һ������
	clock_t _alivetime;//��¼�������״̬�����ʼ���ʱ��
};