#pragma once
#include<string>
#include<queue>
#include<mutex>
#include<fstream>
#include<memory>
#include<thread>
#include<ctime>
#include<atomic>
#include<functional>
#include<condition_variable>
#include"Connection.h"

class ConnectionPool
{
public:
    static ConnectionPool* GetConnectionPool();
    //给外部提供接口，从连接池获取一个连接
    std::shared_ptr<Connection> GetConnection(); //用智能指针自动管理连接的创建与归还
private:
    ConnectionPool();
    ConnectionPool(ConnectionPool& )=delete;
    ConnectionPool& operator =(ConnectionPool&)=delete;

    //加载配置文件
    bool LoadConfigFile();

    //运行在独立的线程中，专门负责生产新连接
    void Produce_conn_Task();

    void Scan_Conn_Task();
    //static ConnectionPool pool;
    std::string _Ip; //ip 地址
    unsigned short _Port; //数据库端口号
    std::string _userame; //用户名
    std::string _Password; //密码
    std::string _dbname;
    int _InitSize;   //初始连接数量
    int _MaxSize;    //最大连接数量
    int _MaxIdleTime; //最大空闲时间
    int _ConnectionTimeout; //连接池获取连接的超时时间

    std::queue<Connection*> _conn_q;//存储连接队列

    std::mutex _q_mutex; //维护线程安全的互斥锁

    std::atomic_int _ConnectionCnt; //记录连接所创建的的总数量

    std::condition_variable cv;//设置条件变量，用于连接生产线程和消费线程的通信
};

ConnectionPool::ConnectionPool(){

    if(!LoadConfigFile()) return;

    for(int i=0;i<_InitSize;i++) {
        Connection* ptr=new Connection();
        ptr->connect(_Ip,_Port,_userame,_Password,_dbname);
        ptr->refreshAliveTime(); //刷新一下进入空闲的起始时间
        _conn_q.push(ptr);
        _ConnectionCnt++;
    }

    std::thread produce(std::bind(Produce_conn_Task,this));
    produce.detach();
    //启动一个新的定时线程，扫描多余的空闲连接，超过maxidletime 的  空闲连接，进行连接回收，
    std::thread scan(std::bind(Scan_Conn_Task,this));
    scan.detach();
}

void ConnectionPool::Scan_Conn_Task(){
    while(true){
        //通过sleep 模拟定时效果
        std::this_thread::sleep_for(std::chrono::seconds(_MaxIdleTime));

        //扫描队列释放，多余的连接
        std::unique_lock<std::mutex> lock(_q_mutex);

        while(_ConnectionCnt>_InitSize){
            Connection* ptr=_conn_q.front();
            if(ptr->GetAliveTime()>=(_MaxIdleTime*1000)){
                _conn_q.pop();
                _ConnectionCnt--;
                delete ptr;
            }
            else break; //队头连接没有超过MaxIdleTime ,其他的也没有超过，
        }
    }
}

std::shared_ptr<Connection> ConnectionPool::GetConnection(){
    std::unique_lock<std::mutex> lock(_q_mutex);
    while(_conn_q.empty()){
        if(std::cv_status::timeout==cv.wait_for(lock,std::chrono::milliseconds(_ConnectionTimeout))){
            if(_conn_q.empty()){
            Log("获取空闲连接超时");
            return nullptr;
        }
        }
        //cv.wait_for(lock,std::chrono::milliseconds(_ConnectionTimeout));
    }

    std::shared_ptr<Connection> sp(_conn_q.front(),[&](Connection* pcon){
        //这里是在服务器应用线程调用的，所以要考虑应用线程安全操作，
        std::unique_lock<std::mutex> lock(_q_mutex); 
        pcon->refreshAliveTime(); //刷新一下进入空闲的起始时间
        _conn_q.push(pcon);

    });
    _conn_q.pop();
    //if(_conn_q.empty()) cv.notify_all(); //谁消费最后一个，谁通知生产
    cv.notify_all(); 
    return sp;

}
void ConnectionPool::Produce_conn_Task(){

    while(true){
        std::unique_lock<std::mutex> lock(_q_mutex);
        while(!_conn_q.empty()){
            cv.wait(lock); //队列不空，生产线程进入等待状态
        }
        //连接数量没有到达上限，继续创建
        if(_ConnectionCnt < _MaxSize){
            Connection* ptr=new Connection();
            ptr->connect(_Ip,_Port,_userame,_Password,_dbname);
            ptr->refreshAliveTime(); //刷新一下进入空闲的起始时间
            _conn_q.push(ptr);
            _ConnectionCnt++;
        }

        //通知消费者线程，连接
        cv.notify_all(); 
    }
}
ConnectionPool* ConnectionPool::GetConnectionPool(){
    static ConnectionPool pool;
    return &pool;
}

bool ConnectionPool::LoadConfigFile(){
    std::ifstream pf("MySql.ini");
    if(!pf){ 
        Log("MySql.ini file is not exist.");
        return false;
    }
    std::string line;
    while(getline(pf,line)){
        int idx=line.find('=');
        if(idx==-1) continue;
        //int endidx=line.find("\n",idx);
        std::string key=line.substr(0,idx);
        std::string value=line.substr(idx+1);
        //std::cout<<key<<" "<<value<<std::endl;
        if(key=="ip") _Ip=value;
        else if(key=="port") _Port=std::stoi(value);
        else if(key=="username") _userame=value;
        else if(key=="password") _Password=value;
        else if(key=="dbname") _dbname=value;
        else if(key=="initSize") _InitSize=std::stoi(value);
        else if(key=="maxsSize") _MaxSize=std::stoi(value);
        else if(key=="maxIdleTime") _MaxIdleTime=std::stoi(value);
        else if(key=="maxConnectionTimeOut") _ConnectionTimeout=std::stoi(value);
    }
    pf.close();
    return true;
}