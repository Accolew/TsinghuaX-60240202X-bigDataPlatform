package com.aliyun.odps.mapred;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

import com.aliyun.odps.counter.Counters;

public class LocalRunningJob implements RunningJob {
	private Map<TaskId, Thread> mapThreadList; 
	private Map<TaskId, Thread> reduceThreadList;  
	private Map<TaskId, Thread> otherThreadList;  

	private Map<TaskId, LocalTaskStatus> taskStatusList;
	
	private JobStatus jobstatus;
	
	private Set<LocalTaskStatus> sucStatus;
	private Set<LocalTaskStatus> waitStatus;
	private Set<LocalTaskStatus> endStatus;
	
	LocalRunningJob() {
		jobstatus = JobStatus.PREP;
		mapThreadList = new ConcurrentHashMap<TaskId, Thread>();
		reduceThreadList = new ConcurrentHashMap<TaskId, Thread>();
		otherThreadList = new ConcurrentHashMap<TaskId, Thread>();

		
		taskStatusList = new ConcurrentHashMap<TaskId, LocalTaskStatus>();
		
		sucStatus = new HashSet<LocalTaskStatus>();
		sucStatus.add(LocalTaskStatus.SUCCEEDED);
		
		waitStatus = new HashSet<LocalTaskStatus>();
		waitStatus.add(LocalTaskStatus.PREPARED);
		waitStatus.add(LocalTaskStatus.WAITNG);

		endStatus = new HashSet<LocalTaskStatus>();
		endStatus.add(LocalTaskStatus.KILLED);
		endStatus.add(LocalTaskStatus.SUCCEEDED);
		endStatus.add(LocalTaskStatus.FAILED);
	}
	
	public synchronized void  add(TaskId taskId, Thread t, TASK_TYPE type) {
		switch (type) {
			case TASK_TYPE_MAP:  mapThreadList.put(taskId, t);
								break;
			case TASK_TYPE_REDUCE: reduceThreadList.put(taskId, t);
								break;
			default: otherThreadList.put(taskId, t);
		}

		this.taskStatusList.put(taskId, LocalTaskStatus.PREPARED);
		// System.out.println("add: "+taskId+","+LocalTaskStatus.PREPARED);
		// printTaskList();
	}
	
	public synchronized void setStatus(TaskId taskId, LocalTaskStatus status) {
		this.taskStatusList.put(taskId, status);
		
		//System.out.println("*************************");
		// System.out.println("set status: "+taskId+","+status.name()); 
		
		if (status == LocalTaskStatus.FAILED) {
			jobstatus = JobStatus.FAILED;
		}  else 
			if ((jobstatus != JobStatus.FAILED) && (status == LocalTaskStatus.RUNNING)) {
				jobstatus = JobStatus.RUNNING;
			}
		
		// printTaskList();
	}
	
	@Override
	public Counters getCounters() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public String getDiagnostics() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public String getInstanceID() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public synchronized JobStatus getJobStatus() {
		// TODO Auto-generated method stub
		if (jobstatus == JobStatus.FAILED) 
			return jobstatus;
		
		boolean isAllSuc = true;
		boolean isAllWait = true;
		boolean isAllComplete = true;
		boolean hasRunning = false;
		boolean hasKilled = false;
		
		for (LocalTaskStatus ts : taskStatusList.values()) {
			if (ts.equals(LocalTaskStatus.RUNNING)) hasRunning = true;
			if (ts.equals(LocalTaskStatus.KILLED)) hasKilled = true;
			
			if (!sucStatus.contains(ts)) isAllSuc = false;
			if (!waitStatus.contains(ts)) isAllWait = false;
			if (!endStatus.contains(ts)) isAllComplete = false;
		}
		
		if (hasRunning) {  // û��fail, ��running��, ����running
			jobstatus = JobStatus.RUNNING;
		}  else 
			if (isAllSuc) {
				jobstatus = JobStatus.SUCCEEDED;  // ���е�succeed, ����succeed

			}  else 
				if (isAllWait) {  // ȫ������wait,��˵������prepare 
						jobstatus = JobStatus.PREP;

				}  else
					if (isAllComplete && hasKilled) {
						// kill��suc���ģʽ
						jobstatus = JobStatus.KILLED;
					}
		
		return jobstatus;
	}

	@Override
	public boolean isComplete() {
		// TODO Auto-generated method stub
		System.out.println("enter isComplete()");
		JobStatus s = getJobStatus();
		
		return (s == JobStatus.FAILED)||(s == JobStatus.KILLED)||(s == JobStatus.SUCCEEDED);
	}

	@Override
	public boolean isSuccessful() {
		// TODO Auto-generated method stub
		return getJobStatus() == JobStatus.SUCCEEDED;
	}

	@Override
	public void killJob() {
		// TODO Auto-generated method stub
		// System.out.println("######### Kill Job!"+ taskStatusList.size());
		// printTaskList();
		
		for (Thread t: otherThreadList.values()) {
			if (t.isAlive()) {
				t.interrupt();
			}
		}
		
		for (Thread t: reduceThreadList.values()) {
			if (t.isAlive()) {
				t.interrupt();
			}
		}
		
		for (Thread t: mapThreadList.values()) {
			if (t.isAlive()) {
				t.interrupt();
			}
		}

		System.out.println("out killJob()"+ taskStatusList.size());
	}

	@Override
	public float mapProgress() throws IOException {
		// TODO Auto-generated method stub
		if (mapThreadList.size()==0) return 0;
		
		int cnt = 0;
		
		for (TaskId id : mapThreadList.keySet()) {
			if (endStatus.contains(taskStatusList.get(id))) {
				cnt++;
			}
		}
		return (float)cnt/mapThreadList.size();
	}

	@Override
	public float reduceProgress() throws IOException {
		// TODO Auto-generated method stub
		if (reduceThreadList.size()==0) return 0;
		
		int cnt = 0;
		
		for (TaskId id : reduceThreadList.keySet()) {
			if (endStatus.contains(taskStatusList.get(id))) {
				cnt++;
			}
		}
		return (float)cnt/reduceThreadList.size();
	}

	@Override
	public void waitForCompletion() {
		// TODO Auto-generated method stub
		try {
			for (Thread t: otherThreadList.values()) {
				if (t.isAlive()) {
					t.join();
				}
			}
			
			for (Thread t: reduceThreadList.values()) {
				if (t.isAlive()) {
					t.join();
				}
			}
			
			for (Thread t: mapThreadList.values()) {
				if (t.isAlive()) {
					t.join();
				}
			} 
		} catch (InterruptedException e) {
			
		}
	}

	public void printTaskList() {
		for (Entry<TaskId, LocalTaskStatus> e: taskStatusList.entrySet()) {
			System.out.println("************ ["+e.getKey()+","+e.getValue()+"]");
		}
	}
	
	public Collection<Thread> getMapThreadList() {
		return mapThreadList.values();
	}
	
	public Collection<Thread> getReduceWaitingThreadList() {
		ArrayList<Thread> waitList = new ArrayList<Thread>(mapThreadList.values());
		waitList.addAll(otherThreadList.values());
			
		return waitList;
	}
	
	public LocalTaskStatus getMapStatus() {
		for (TaskId mapTaskId : mapThreadList.keySet()) {
			LocalTaskStatus s = taskStatusList.get(mapTaskId);
			if ( s == LocalTaskStatus.FAILED) {
				return LocalTaskStatus.FAILED;
			}  else 
				if ( s == LocalTaskStatus.KILLED) {
					return  LocalTaskStatus.KILLED;
				}
		}
		
		return  LocalTaskStatus.SUCCEEDED;
	}
	
	public LocalTaskStatus getOtherTaskStatus() {
		for (TaskId mapTaskId : otherThreadList.keySet()) {
			LocalTaskStatus s = taskStatusList.get(mapTaskId);
			if ( s == LocalTaskStatus.FAILED) {
				return LocalTaskStatus.FAILED;
			}  else 
				if ( s == LocalTaskStatus.KILLED) {
					return  LocalTaskStatus.KILLED;
				}
		}
		
		return  LocalTaskStatus.SUCCEEDED;
	}

}
