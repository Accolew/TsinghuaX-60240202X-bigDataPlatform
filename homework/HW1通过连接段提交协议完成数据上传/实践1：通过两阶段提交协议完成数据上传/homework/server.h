#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <map>
#include <vector>
#include <stdint.h>

#include "worker.h"

namespace homework
{

class Server
{
public:
    Server();

    ~Server();
    
    /*��Ҫʵ�ֵĺ�������content������д��tableNameָ���ı���*/

    bool UpdateTable(const std::string& content, const std::string& tableName);
    
private:
	  // workerId �� ָ���worker��ָ��
    std::map<uint8_t, WorkerPtr> mWorkers;
    	
    // Table�����俽������Worker�� list 
    /* table����table���������worker��ӳ�䣬����һ��table�����ж�ݿ��������Կ��ܴ���ڶ��Worker��*/
    std::map<std::string, std::vector<uint8_t> > mTables;
    	 
    
};
}

#endif
