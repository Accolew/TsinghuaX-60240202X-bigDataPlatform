#include "server.h"

using namespace std;
using namespace homework;

#include <stdlib.h>
#include <algorithm>

bool Server::UpdateTable(const string& tableName, const string& content)
{
 		/* �����⣬����worker������ */
    std::map<size_t, WorkerPtr>::iterator  it_workers;
    std::vector<size_t> worker_list;
    	    
    // �׶�1���������Ƿ����ִ���ύ����
    bool result = true;
    
    for (it_workers = mWorkers.begin(); it_workers != mWorkers.end(); ++it_workers) {
    	 result = it_workers->second->UpdateTable(tableName, content);

    	 if (result == false) {
    	 		cout<<"worker "<<it_workers->first<<" UpdateTable fail"<<endl;
    	 		
    	 		break;
    	 }
    }
        
    // �׶ζ����ع� �� �����ύ
    if (result == false)    // ��ʧ�ܵģ��ع�
    {
    	 for (it_workers = mWorkers.begin(); it_workers != mWorkers.end(); ++it_workers)
    	 {
		    	 result = it_workers->second->Rollback(tableName);
		    	 if (result == false) {  
		    	 	  // �ع�ʧ��
		    	 }
		   }
		   
		   result = false;
    }  
    else // ���ж��ɹ�
    {
    	for (it_workers = mWorkers.begin(); it_workers != mWorkers.end(); ++it_workers)
    	{
    	 		result = it_workers->second->Commit(tableName);
    	 		
    	 		if (result == false) {  
    	 			// �ύʧ��, ����worker
    	 		}  else {
    	 			worker_list.push_back(it_workers->first);
    	 		}
    	 }
    	 
    	 result = true;
    }
    
    mTables[tableName] = worker_list;
        
    return result;
}


bool Worker::UpdateTable(const string& tableName, const string& content)
{	 
	  std::map<std::string, std::vector<std::string> >::iterator it;
	  std::vector<std::string> tmpFileList;
	  	
	  	
	  if ( (it = mTableFilesTmpToCommit.find(tableName) ) != mTableFilesTmpToCommit.end()) { // ��δ�ύ��ع�������
	  	return false;
	  } 	 
	  
	  // ����Ҫ�ֶ��ٿ�
	  int fileNum = content.length()/BlockSize;
	  if (content.length() % BlockSize > 0) fileNum++;
	  	
	  // �ֿ�д�����ļ�
	  for (size_t i=0; i< fileNum; i++) {
	  	
	  	  // �������ļ�
	  	  std::string newFileId = GetFileId();
	  	  	
	  	  if (CreateFile(newFileId) == false) 
	  	  	return false;
	  	  	
	  	  if (WriteToFile(newFileId,content.substr(i*BlockSize, BlockSize)) == false) {
	  	  	
	  	  	// ɾ���Ѿ����ɵ���ʱ�ļ�
			for (size_t j=0; j<tmpFileList.size();j++) 
			{
				   DeleteFile(tmpFileList[j]);
			}
	  	  		
	  	  	return false;
	  	  }
	  	  	
	  	  tmpFileList.push_back(newFileId);
	  }

    // ���浽���ύ�޸���
    mTableFilesTmpToCommit[tableName] = tmpFileList;
    
    return true;
	  
}


bool Worker::Commit(const std::string& tableName){
	
		std::map<std::string, std::vector<std::string> >::iterator it_new, it_old;
		
		it_new = mTableFilesTmpToCommit.find(tableName);
		it_old = mTableFiles.find(tableName);
		
		if ( (it_new == mTableFilesTmpToCommit.end()) && (it_old == mTableFiles.end())) {   // û����Ҫ�ύ������
	  	return true;
	  }
	  
	  std::vector<std::string> files;
	  if ( it_old != mTableFiles.end()) {   // ��ɾ���ľ��ļ�
	  		files = it_old->second;
	  }
	  
	  if (it_new != mTableFilesTmpToCommit.end() ) {  // ����������ļ�
	  		mTableFiles[tableName] = it_new->second;  
	  	  mTableFilesTmpToCommit.erase(it_new);
	  }
	  
	  // ɾ�����ļ�		
	  for (size_t i=0; i<files.size(); i++) {
		 	  DeleteFile(files[i]);
		}
		return true;
	  
}

bool Worker::Rollback(const std::string& tableName){
	
		std::map<std::string, std::vector<std::string> >::iterator it_new;
		it_new = mTableFilesTmpToCommit.find(tableName);

		if ( it_new == mTableFilesTmpToCommit.end()) {   // û����Ҫ�ύ������
	  	return true;
	  }
	  
	  // ɾ�����ļ�
	  std::vector<std::string> files = it_new->second;
	  mTableFilesTmpToCommit.erase(it_new);

	  for (size_t i=0; i<files.size(); i++) {
		 	  DeleteFile(files[i]);
		}
		
	  return true;
}
