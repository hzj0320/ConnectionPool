#include "public.h"
#include "Connection.h"

Connection::Connection() {
	_conn = mysql_init(nullptr);//初始化数据库连接,指针指向MySQL结构体内存
}

Connection::~Connection() {
	if (_conn != nullptr)
		mysql_close(_conn);//关闭连接
}

//连接数据库
bool Connection::connect(string ip, unsigned short port, string username, string password, string dbname) {
	MYSQL* p = mysql_real_connect(_conn, ip.c_str(), username.c_str(), password.c_str(), dbname.c_str(), port, nullptr, 0);
	return p != nullptr;
}

//更新操作
bool Connection::update(string sql) {
	if (mysql_query(_conn, sql.c_str())) {//执行制定为一个空结尾的字符串的sql语句
		LOG("更新失败：" + sql);
		return false;
	}
	return true;
}

//查询操作
MYSQL_RES* Connection::query(string sql) {
	if (mysql_query(_conn, sql.c_str())) {
		LOG("更新失败：" + sql);
		return nullptr;
	}
	return mysql_use_result(_conn);
}