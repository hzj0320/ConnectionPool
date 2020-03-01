#pragma once
//MySQL数据库操作
#include <mysql.h>
#include <string>
#include <ctime>
using namespace std;
class Connection {
public:
	Connection();//初始化数据库连接
	~Connection();//释放数据库连接
	//连接数据库
	bool connect(string ip, unsigned short port, string username, string password, string dbname);
	//更新操作
	bool update(string sql);
	//查询操作
	MYSQL_RES* query(string sql);

	//刷新连接起始空闲时间
	void refreshAliveTime() {
		_alivetime = clock();
	}
	//返回存活时间
	clock_t getAliveTime() {
		return clock() - _alivetime;
	}
private:
	MYSQL* _conn;//表示和MySQL的一条连接
	clock_t _alivetime;//记录进入空闲状态后的起始存活时间
};