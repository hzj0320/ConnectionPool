#pragma once
#include<string>
#include<queue>
#include<mutex>
#include<atomic>
#include<thread>
#include<condition_variable>
#include<memory>
#include<functional>
#include"Connection.h"
using namespace std;
//���ӳع���ģ��
//�̰߳�ȫ����
class ConnectionPool {
public:
	static ConnectionPool* getConnectionPool();//��ȡ���ӳض���ʵ��
	//���ⲿ�ṩ�ӿڣ������ӳ��л�ȡһ�����õĿ�������
	shared_ptr<Connection> getConnection();//�Զ������ͷţ��ڷ������߳�������
private:
	ConnectionPool();//����1�����캯��˽�л�
	bool loadConfigFile();
	void produceConnectionTask();//�����ڶ����߳��У�ר�Ÿ�������������
	void scannerConnectionTask();
	string _ip;//mysql��ip��ַ
	unsigned short _port;//�˿ں�
	string _username;//�û���
	string _password;//��¼����
	string _dbname;//���ݿ���
	int _initSize;//��ʼ������
	int _maxSize;//���������
	int _maxIdleTime;//���ӳ�������ʱ��
	int _connectionTimeout;//���ӳػ�ȡ���ӳ�ʱʱ��

	queue<Connection*> _connectionQue;//�洢connection���ӵĶ���
	mutex _queueMutex;//ά�����е��̰߳�ȫ�Ļ�����
	atomic_int _connectionCnt;//��¼������������connection���ӵ�������,�̰߳�ȫ��++����
	condition_variable cv;//���������������������������̺߳����������̵߳�ͨ��
};