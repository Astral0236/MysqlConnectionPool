#include<iostream>
#include<string>
#include"Connection.h"
#include"ConnectionPool.h"
#include"Public.h"


std::string name="jerry";
int age=21;
std::string sex="male";
//1000 次不用数据库连接池 耗时5132 ms 
void test(){

    clock_t begin=clock();
    for(int i=0;i<10000;i++){
        Connection conn;
        //std::cin>>name>>sex>>age;
        std::string sql = "INSERT INTO user (name, age, sex) VALUES ('" + name + "', " + std::to_string(age) + ", '" + sex + "');";
        conn.connect("127.0.0.1",3306,"root","Xsk2825394371.","chat");
        conn.update(sql);
    }
    clock_t end=clock();
    std::cout<<end-begin<<" ms"<<std::endl; //  
}
// 使用连接池单线程
void test2(){
    clock_t begin=clock();
    for(int i=0;i<10000;i++){

        ConnectionPool* pool=ConnectionPool::GetConnectionPool();
        std::shared_ptr<Connection> sp=pool->GetConnection();
        //std::cin>>name>>sex>>age;
        std::string sql = "INSERT INTO user (name, age, sex) VALUES ('" + name + "', " + std::to_string(age) + ", '" + sex + "');";
        sp->update(sql);
    }
    clock_t end=clock();
    std::cout<<end-begin<<" ms"<<std::endl;
}
// 使用连接池多线程
void test3(){

    clock_t begin=clock();
    ConnectionPool* pool=ConnectionPool::GetConnectionPool();

    std::thread t1([pool](){
        for(int i=0;i<2500;i++){
        //std::cin>>name>>sex>>age;
        std::shared_ptr<Connection> sp=pool->GetConnection();
        std::string sql = "INSERT INTO user (name, age, sex) VALUES ('" + name + "', " + std::to_string(age) + ", '" + sex + "');";
        sp->update(sql);
    }
    });
    std::thread t2([pool]() {
        for(int i=0;i<2500;i++){
        //std::cin>>name>>sex>>age;
        std::shared_ptr<Connection> sp=pool->GetConnection();
        std::string sql = "INSERT INTO user (name, age, sex) VALUES ('" + name + "', " + std::to_string(age) + ", '" + sex + "');";
        sp->update(sql);
    }
    });

    std::thread t3([pool](){
        for(int i=0;i<2500;i++){
        //std::cin>>name>>sex>>age;
        std::shared_ptr<Connection> sp=pool->GetConnection();
        std::string sql = "INSERT INTO user (name, age, sex) VALUES ('" + name + "', " + std::to_string(age) + ", '" + sex + "');";
        sp->update(sql);
    }
    });
    std::thread t4([pool](){
        for(int i=0;i<2500;i++){
        //std::cin>>name>>sex>>age;
        std::shared_ptr<Connection> sp=pool->GetConnection();
        std::string sql = "INSERT INTO user (name, age, sex) VALUES ('" + name + "', " + std::to_string(age) + ", '" + sex + "');";
        sp->update(sql);
    }
    });
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    clock_t end=clock();
    std::cout<<end-begin<<" ms"<<std::endl;
}
// 未使用连接池多线程
void test4(){

    clock_t begin=clock();
    std::thread t1([](){
        for(int i=0;i<2500;i++){
        Connection conn;
        //std::cin>>name>>sex>>age;
        std::string sql = "INSERT INTO user (name, age, sex) VALUES ('" + name + "', " + std::to_string(age) + ", '" + sex + "');";
        conn.connect("127.0.0.1",3306,"root","Xsk2825394371.","chat");
        conn.update(sql);
    }
    });
    std::thread t2([](){
        for(int i=0;i<2500;i++){
        Connection conn;
        //std::cin>>name>>sex>>age;
        std::string sql = "INSERT INTO user (name, age, sex) VALUES ('" + name + "', " + std::to_string(age) + ", '" + sex + "');";
        conn.connect("127.0.0.1",3306,"root","Xsk2825394371.","chat");
        conn.update(sql);
    }
    });
    std::thread t3([](){
        for(int i=0;i<2500;i++){
        Connection conn;
        //std::cin>>name>>sex>>age;
        std::string sql = "INSERT INTO user (name, age, sex) VALUES ('" + name + "', " + std::to_string(age) + ", '" + sex + "');";
        conn.connect("127.0.0.1",3306,"root","Xsk2825394371.","chat");
        conn.update(sql);
    }
    });
    std::thread t4([](){
        for(int i=0;i<2500;i++){
        Connection conn;
        //std::cin>>name>>sex>>age;
        std::string sql = "INSERT INTO user (name, age, sex) VALUES ('" + name + "', " + std::to_string(age) + ", '" + sex + "');";
        conn.connect("127.0.0.1",3306,"root","Xsk2825394371.","chat");
        conn.update(sql);
    }
    });
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    clock_t end=clock();
    std::cout<<end-begin<<" ms"<<std::endl; //
}
int main()
{
    //数据库连接池只要1355 ms真他娘的快
    //std::cout<<"hello world"<<std::endl;
    test4();
    return 0;
}