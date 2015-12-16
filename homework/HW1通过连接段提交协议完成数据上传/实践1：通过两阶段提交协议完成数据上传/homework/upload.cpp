#include "server.h"

using namespace std;
using namespace homework;

bool Server::UpdateTable(const string& tableName, const string& content)
{
 		/*��ȡtable���п������ڵ�Worker id*/
    std::vector<size_t> workers = mTables[tableName];
    
    if (workers.size() == 0)   // ȫ�µ�table������Work_id
    {
       if (allocWorkers() == null) {
       	   return false;
       }
       
       workers = mTables[tableName];
    }
    
    // �׶�1���������Ƿ����ִ���ύ����
    bool result = true;
    size_t i;
    
    for (i = 0; i < workers.size(); ++i�� {
    	 result = mWorkers[workers[i]]->UpdateTable(tableName, content);
    	 if (result == false) {
    	 		break;
    	 }
    }
    
    // �׶ζ����ع� �� �����ύ
    if (result == false)    // ��ʧ�ܵģ��ع�
    {
    	 for (size_t j = 0; j < i; ++i�� 
    	 {
		    	 result = mWorkers[workers[j]]->Rollback(tableName);
		    	 if (result == false) {  
		    	 	  // �ع�ʧ��
		    	 }
		   }
		   
		   result = false;
    }  
    else // ���ж��ɹ�
    {
    	for (size_t i = 0; i < workers.size(); ++i�� 
    	{
    	 		result = mWorkers[workers[i]]->Commit(tableName);
    	 		if (result == false) {  
    	 			// �ύʧ��, ����worker
    	 		}
    	 }
    	 
    	 result = true;
    }
    
    return result;
}

bool Server::allocWorkers(const string& tableName)  {
	maxCopyNum = max(mWorkers.size(), 3);
	
	// ����[0,maxCopyNum)֮����������
	 srand((unsigned)time(NULL)); 
	 size_t idx = rand() % maxCopyNum;
	 
	 // ������ʼλ��
	 std::map<uint8_t, WorkerPtr> ::iterator it = mWorkers.begin() + idx;
	 
	 std::vector<std::string> work_list;
	 	
	 // ȡmaxCopyNum��worker
	 for (size_t i=0; i<maxCopyNum; i++)
	 {
	 	  work_list[i] = it->first;
	 	  it++;
	 	  if (it == mWorkers.end()) 
	 	  {
	 	      it = mWorkers.begin();
	 	  }
	 	  	
	 }
	 
	 mTables[tableName] = work_list;
	 return true;
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
	  if (content.length % BlockSize > 0) fileNum++;
	  	
	  // �ֿ�д�����ļ�
	  for (size_t i=0; i< fileNum; i++) {
	  	
	  	  // �������ļ�
	  	  std::string newFileId = GetFileId();
	  	  	
	  	  if (CreateFile(newFileId) == false) 
	  	  	return false;
	  	  	
	  	  if (WriteToFile(newFileId,content.substr(i*BlockSize, BlockSize)) == false) {
	  	  	
	  	  	// ɾ���Ѿ����ɵ���ʱ�ļ�
	  	  	std::for_each( tmpFileList.begin(), tmpFileList.end(), DeleteFile);
	  	  		
	  	  	return false;
	  	  }
	  	  	
	  	  tmpFileList[i] = newFileId;
	  }

    // ���浽���ύ�޸���
    mTableFilesTmpToCommit[tableName] = tmpFileList;
    
    return true;
	  
}


bool Worker::Commit(const std::string& tableName){
	
		std::map<std::string, std::vector<std::string> >::iterator it_new, it_old;
		
		it_new = mTableFilesTmpToCommit.find(tableName);
		it_old = mTableFiles.find(tableName);
		
		if ( it_new == mTableFilesTmp.end() && (it_old == mTableFiles.end()) {   // û����Ҫ�ύ������
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
		
		if ( it_new == mTableFilesTmp.end()) {   // û����Ҫ�ύ������
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
