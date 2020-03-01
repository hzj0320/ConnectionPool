#include "ConnectionPool.h"
#include "public.h"
#include<string>

//线程安全的单例函数接口
ConnectionPool* ConnectionPool::getConnectionPool() {
	static ConnectionPool pool;//线程安全,静态对象初始化，编译器lock和unlock
	return &pool;
}

//从配置文件中加载配置项
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
		if (idx == -1) {//无效配置
			continue;
		}
		int endidx = str.find('\n', idx);
		string key = str.substr(0, idx);
		string value = str.substr(idx + 1, endidx - idx - 1);
		if (key == "ip") {
			_ip = value;
		}
		else if (key == "port") {
			_port = atoi(value.c_str());//转成整数
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

//连接池的构造
ConnectionPool::ConnectionPool() {
	//加载配置项
	if (!loadConfigFile()) {
		return;
	}

	//创建初始数量的链接
	for (int i = 0; i < _initSize; ++i) {
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		p->refreshAliveTime();//刷新开始空闲的起始时间
		_connectionQue.push(p);
		_connectionCnt++;
	}

	//启动新线程作为连接生产者
	thread produce(std::bind(&ConnectionPool::produceConnectionTask,this));//绑定当前对象的成员方法
	produce.detach();//分离线程函数，使用detach()函数会让线程在后台运行，即说明主线程不会等待子线程运行结束才结束
	
	//启动新定时线程，扫描超过manIdleTime时间的空闲连接，进行多余连接回收
	thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));//绑定当前对象的成员方法
	scanner.detach();
}

//生产者线程
void ConnectionPool::produceConnectionTask() {
	for (;;) {
		unique_lock<mutex> lock(_queueMutex);//生产者加锁
		while (!_connectionQue.empty()) {
			cv.wait(lock);//队列不空，此处线程进入等待状态，锁释放
		}
		//创建新连接
		if (_connectionCnt < _maxSize) {
			Connection* p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			p->refreshAliveTime();//刷新开始空闲的起始时间
			_connectionQue.push(p);
			_connectionCnt++;
		}
		//通知消费者线程可消费连接
		cv.notify_all();
	}
	/*
	生产者进来加锁，队列空，生产连接，通知消费者，消费者从等待跳到阻塞状态，出括号，锁释放，消费者消费。
	队列非空，等待状态，锁释放，消费者消费；
	*/
}

shared_ptr<Connection> ConnectionPool::getConnection() {
	unique_lock<mutex> lock(_queueMutex);
	while(_connectionQue.empty()) {
		//cv.wait_for(lock, chrono::milliseconds(_connectionTimeout));

		//sleep
		if (cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeout))) {
			if (_connectionQue.empty()) {
				LOG("获取连接超时了，连接失败！");//唤醒或超时，队列空
				return nullptr;
			}
		}


	}
	//智能指针析构时，会把connectiond资源delete掉，相当于调用connection的析构函数，
	//需自定义shared_ptr的资源释放方式
	shared_ptr<Connection> sp(_connectionQue.front(), [&](Connection * pcon){//lambda表达式，直接修改删除器
		unique_lock<mutex> lock(_queueMutex);//在服务器应用线程中调用，需要保证线程安全，释放锁
		pcon->refreshAliveTime();//刷新开始空闲的起始时间
		_connectionQue.push(pcon);
		});

	_connectionQue.pop();
	cv.notify_all();//负责通知生产者检查
	return sp;
}
/*
	消费者进来加锁，队列为空，等待超时时间，有人唤醒，检查队列是否为空。使用智能指针，自定义删除器。
*/

/*
	等待时被唤醒，被其他线程把connection拿走了，队列还是空，
*/


void ConnectionPool::scannerConnectionTask() {
	for (;;) {
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));//模拟定时连接

		//扫描队列，释放连接
		unique_lock<mutex> lock(_queueMutex);//线程互斥
		if (_connectionCnt > _initSize) {
			Connection* p = _connectionQue.front();
			if (p->getAliveTime() >= (_maxIdleTime * 1000)) {
				_connectionQue.pop();
				_connectionCnt--;
				delete p;//调用~connection 释放连接
			}
			else {
				break;//只需判断队头元素
			}
		}
	}
}


