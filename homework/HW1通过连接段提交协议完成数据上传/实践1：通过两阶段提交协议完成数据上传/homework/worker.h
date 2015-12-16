#ifndef WORKER_H
#define WORKER_H

#include <iostream>
#include <map>
#include <vector>

namespace homework
{
class Worker
{
public:
    Worker(uint8_t workerId)
    : mWorkerId(workerId)
    {}

    /*��Ҫʵ�ֵĺ�������content������д��tableName��ָ���ĸ�����*/
    bool UpdateTable(const std::string& tableName, const std::string& content);
    	
    bool Commit(const std::string& tableName);

    bool Rollback(const std::string& tableName);


    /*��ȡ����������*/
    bool ReadTable(const std::string& tableName, std::string& content);
    	

private:
	  /*���Ի�ȡһ��ȫ��Ψһ���ļ�id*/
    static std::string GetFileId();
    
    /*Worker���ļ��Ĳ������������true˵�������ɹ�������false˵��ִ��ʧ��*/
    bool CreateFile(const std::string& fileId);

    bool DeleteFile(const std::string& fileId);

    bool RenameFile(const std::string& oriFileId, const std::string& newFileId);

    bool WriteToFile(const std::string& fileId, const std::string& content);

    bool ReadFile(const std::string& fileId, std::string& content);    

    /*worker��id*/
    uint8_t mWorkerId;
    
    /*��������洢�ļ�list��ӳ��*/
    std::map<std::string, std::vector<std::string> > mTableFiles;
    	
    std::map<std::string, std::vector<std::string> > mTableFilesTmpToCommit;   /* ��һ�׶��Ѿ�ɾ��������û��commit��rollback�� */
    	
    /*�ļ�id���ļ���ʵ·����ӳ��*/
    std::map<std::string, std::string> mFiles;
    	
    /* BlockSize */
    static const int BlockSize = 1024;
};
typedef Worker* WorkerPtr;

}
#endif
