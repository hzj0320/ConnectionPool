#include<iostream>
#include"Connection.h"
#include"ConnectionPool.h"
using namespace std;


int main() {
	//Connection conn;
	//conn.connect("127.0.0.1", 3306, "root", "19960320", "chat");//无连接池，线程内多次登录不合法
	clock_t begin = clock();
	
	thread t1([]() {
		ConnectionPool* cp = ConnectionPool::getConnectionPool();//获取连接池
		for (int i = 0; i < 1250; ++i) {
			shared_ptr<Connection> sp = cp->getConnection();//获取连接
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 20, "male");
			sp->update(sql);
		}
	});

	thread t2([]() {
		ConnectionPool* cp = ConnectionPool::getConnectionPool();
		for (int i = 0; i < 1250; ++i) {
			shared_ptr<Connection> sp = cp->getConnection();
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 20, "male");
			sp->update(sql);
		}
	});

	thread t3([]() {
		ConnectionPool* cp = ConnectionPool::getConnectionPool();
		for (int i = 0; i < 1250; ++i) {
			shared_ptr<Connection> sp = cp->getConnection();
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 20, "male");
			sp->update(sql);
		}
	});

	thread t4([]() {
		ConnectionPool* cp = ConnectionPool::getConnectionPool();
		for (int i = 0; i < 1250; ++i) {
			shared_ptr<Connection> sp = cp->getConnection();
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 20, "male");
			sp->update(sql);
		}
	});
	t1.join();
	t2.join();
	t3.join();
	t4.join();

	clock_t end = clock();
	cout << end - begin << "ms" << endl;

	return 0;
}