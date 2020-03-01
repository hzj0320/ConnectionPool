#include "ConnectionPool.h"
#include "public.h"
#include<string>

//�̰߳�ȫ�ĵ��������ӿ�
ConnectionPool* ConnectionPool::getConnectionPool() {
	static ConnectionPool pool;//�̰߳�ȫ,��̬�����ʼ����������lock��unlock
	return &pool;
}

//�������ļ��м���������
bool ConnectionPool::loadConfigFile() {
	FILE* pf = fopen("mysql.ini", "r");
	if (pf == nullptr) {
		LOG("mysql.ini file is not exist!");
		return false;
	}
	while (!feof(pf)) {
		char line[1024] = { 0 };
		fgets(line, 1024, pf);
		string str = line;
		int idx = str.find('=', 0);
		if (idx == -1) {//��Ч����
			continue;
		}
		int endidx = str.find('\n', idx);
		string key = str.substr(0, idx);
		string value = str.substr(idx + 1, endidx - idx - 1);
		if (key == "ip") {
			_ip = value;
		}
		else if (key == "port") {
			_port = atoi(value.c_str());//ת������
		}
		else if (key == "username") {
			_username = value;
		}
		else if (key == "password") {
			_password = value;
		}
		else if (key == "dbname") {
			_dbname = value;
		}
		else if (key == "initSize") {
			_initSize = atoi(value.c_str());
		}
		else if (key == "maxSize") {
			_maxSize= atoi(value.c_str());
		}
		else if (key == "maxIdleTime") {
			_maxIdleTime= atoi(value.c_str());
		}
		else if (key == "connectionTimeout") {
			_connectionTimeout= atoi(value.c_str());
		}
	}
	return true;
}

//���ӳصĹ���
ConnectionPool::ConnectionPool() {
	//����������
	if (!loadConfigFile()) {
		return;
	}

	//������ʼ����������
	for (int i = 0; i < _initSize; ++i) {
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		p->refreshAliveTime();//ˢ�¿�ʼ���е���ʼʱ��
		_connectionQue.push(p);
		_connectionCnt++;
	}

	//�������߳���Ϊ����������
	thread produce(std::bind(&ConnectionPool::produceConnectionTask,this));//�󶨵�ǰ����ĳ�Ա����
	produce.detach();//�����̺߳�����ʹ��detach()���������߳��ں�̨���У���˵�����̲߳���ȴ����߳����н����Ž���
	
	//�����¶�ʱ�̣߳�ɨ�賬��manIdleTimeʱ��Ŀ������ӣ����ж������ӻ���
	thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));//�󶨵�ǰ����ĳ�Ա����
	scanner.detach();
}

//�������߳�
void ConnectionPool::produceConnectionTask() {
	for (;;) {
		unique_lock<mutex> lock(_queueMutex);//�����߼���
		while (!_connectionQue.empty()) {
			cv.wait(lock);//���в��գ��˴��߳̽���ȴ�״̬�����ͷ�
		}
		//����������
		if (_connectionCnt < _maxSize) {
			Connection* p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			p->refreshAliveTime();//ˢ�¿�ʼ���е���ʼʱ��
			_connectionQue.push(p);
			_connectionCnt++;
		}
		//֪ͨ�������߳̿���������
		cv.notify_all();
	}
	/*
	�����߽������������пգ��������ӣ�֪ͨ�����ߣ������ߴӵȴ���������״̬�������ţ����ͷţ����������ѡ�
	���зǿգ��ȴ�״̬�����ͷţ����������ѣ�
	*/
}

shared_ptr<Connection> ConnectionPool::getConnection() {
	unique_lock<mutex> lock(_queueMutex);
	while(_connectionQue.empty()) {
		//cv.wait_for(lock, chrono::milliseconds(_connectionTimeout));

		//sleep
		if (cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeout))) {
			if (_connectionQue.empty()) {
				LOG("��ȡ���ӳ�ʱ�ˣ�����ʧ�ܣ�");//���ѻ�ʱ�����п�
				return nullptr;
			}
		}


	}
	//����ָ������ʱ�����connectiond��Դdelete�����൱�ڵ���connection������������
	//���Զ���shared_ptr����Դ�ͷŷ�ʽ
	shared_ptr<Connection> sp(_connectionQue.front(), [&](Connection * pcon){//lambda���ʽ��ֱ���޸�ɾ����
		unique_lock<mutex> lock(_queueMutex);//�ڷ�����Ӧ���߳��е��ã���Ҫ��֤�̰߳�ȫ���ͷ���
		pcon->refreshAliveTime();//ˢ�¿�ʼ���е���ʼʱ��
		_connectionQue.push(pcon);
		});

	_connectionQue.pop();
	cv.notify_all();//����֪ͨ�����߼��
	return sp;
}
/*
	�����߽�������������Ϊ�գ��ȴ���ʱʱ�䣬���˻��ѣ��������Ƿ�Ϊ�ա�ʹ������ָ�룬�Զ���ɾ������
*/

/*
	�ȴ�ʱ�����ѣ��������̰߳�connection�����ˣ����л��ǿգ�
*/


void ConnectionPool::scannerConnectionTask() {
	for (;;) {
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));//ģ�ⶨʱ����

		//ɨ����У��ͷ�����
		unique_lock<mutex> lock(_queueMutex);//�̻߳���
		if (_connectionCnt > _initSize) {
			Connection* p = _connectionQue.front();
			if (p->getAliveTime() >= (_maxIdleTime * 1000)) {
				_connectionQue.pop();
				_connectionCnt--;
				delete p;//����~connection �ͷ�����
			}
			else {
				break;//ֻ���ж϶�ͷԪ��
			}
		}
	}
}


