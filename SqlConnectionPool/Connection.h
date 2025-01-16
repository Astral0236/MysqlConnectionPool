#pragma once
#include"mysql.h"
#include"Public.h"
// mysql 数据库的增删改查
class Connection
{
public:
    Connection();
    ~Connection();

    bool connect(std::string ,unsigned short ,std::string , std::string ,std::string );
    bool update(std::string sql);

    void refreshAliveTime(); //刷新一下连接起始的空闲时间

    clock_t GetAliveTime() const {   //返回存活的时间
        return clock()-_alivetime;
    }

    MYSQL_RES* query(std::string sql);
private:
    MYSQL* _conn;

    clock_t _alivetime; //记录进入空闲状态后的存活时间
};

Connection::Connection(){
    _conn=mysql_init(nullptr); //初始化连接
}
Connection::~Connection(){
    if(_conn!=nullptr){
        mysql_close(_conn);
    }
}
void Connection::refreshAliveTime(){
    _alivetime=clock();

}
bool Connection::connect(std::string ip,unsigned short port,std::string username,std::string password,std::string dbname){
    MYSQL *ptr=mysql_real_connect(_conn,ip.c_str(),username.c_str(),password.c_str(),dbname.c_str(),port,nullptr,0);
    return ptr!= nullptr;
}

bool Connection::update(std::string sql){ //insert update delete
    if(mysql_query(_conn,sql.c_str())){
        Log("更新失败"+sql);
        return false;
    }
    return true;
}

MYSQL_RES* Connection::query(std::string sql){ //select
    if(mysql_query(_conn,sql.c_str())){
        Log("查询失败:"+sql);
        return nullptr;
    }
    return mysql_use_result(_conn);
}

