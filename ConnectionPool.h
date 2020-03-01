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
//连接池功能模块
//线程安全单例
class ConnectionPool {
public:
	static ConnectionPool* getConnectionPool();//获取连接池对象实例
	//给外部提供接口，从连接池中获取一个可用的空闲连接
	shared_ptr<Connection> getConnection();//自动析构释放，在服务器线程中运行
private:
	ConnectionPool();//单例1，构造函数私有化
	bool loadConfigFile();
	void produceConnectionTask();//运行在独立线程中，专门负责生产新连接
	void scannerConnectionTask();
	string _ip;//mysql的ip地址
	unsigned short _port;//端口号
	string _username;//用户名
	string _password;//登录密码
	string _dbname;//数据库名
	int _initSize;//初始连接量
	int _maxSize;//最大连接量
	int _maxIdleTime;//连接池最大空闲时间
	int _connectionTimeout;//连接池获取连接超时时间

	queue<Connection*> _connectionQue;//存储connection连接的队列
	mutex _queueMutex;//维护对列的线程安全的互斥锁
	atomic_int _connectionCnt;//记录连接所创建的connection连接的总数量,线程安全的++操作
	condition_variable cv;//设置条件变量，用于连接生产线程和连接消费线程的通信
};