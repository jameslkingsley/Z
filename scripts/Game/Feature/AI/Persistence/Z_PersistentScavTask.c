[EL_DbName(Z_PersistentScavTaskSaveData, "Z_ScavTask")]
class Z_PersistentScavTaskSaveData : EL_ScriptedStateSaveDataBase
{
	string m_Task;
	
	vector m_Origin;
	
	ref array<ref Z_ScavTaskEntityStub> m_EntityStubs;
	
	override bool ReadFrom(notnull EL_PersistentScriptedStateBase scriptedState)
	{
		Z_PersistentScavTask task = Z_PersistentScavTask.Cast(scriptedState);
		
		m_Task = task.m_Task;
		m_Origin = task.m_Origin;
		m_EntityStubs = task.m_EntityStubs;
		
		return true;
	}

	override bool ApplyTo(notnull EL_PersistentScriptedStateBase scriptedState)
	{
		Z_PersistentScavTask task = Z_PersistentScavTask.Cast(scriptedState);
		
		task.m_Task = m_Task;
		task.m_Origin = m_Origin;
		task.m_EntityStubs = m_EntityStubs;
		
		return true;
	}
}

[EL_PersistentScriptedStateSettings(Z_PersistentScavTask, Z_PersistentScavTaskSaveData, autoSave: false, shutDownSave: true, selfDelete: false)]
class Z_PersistentScavTask : EL_PersistentScriptedStateBase
{
	// IEntity m_Region;
	
	string m_Task;
	
	vector m_Origin;
	
	ref array<ref Z_ScavTaskEntityStub> m_EntityStubs;
	
	static Z_PersistentScavTask Create(string task, vector origin)
	{
		Z_PersistentScavTask instance();
		
		instance.m_Task = task;
		instance.m_Origin = origin;
		instance.m_EntityStubs = new array<ref Z_ScavTaskEntityStub>();
		
		instance.Save();
		
		Print("Saved new task: " + instance.m_sId);
		
		return instance;
	}
	
	void Spawn()
	{
		// Z_ScavTaskBase task = Z_ScavTaskFactory.Make(m_Task);
		// task.Spawn(this);
		// GetGame().GetCallqueue().CallLater(Watch, 2000, true);
	}
	
	void Watch()
	{
		Print("Watching task for changes...");
		
		Save();
	}
	
	override void Delete()
	{
		GetGame().GetCallqueue().Remove(Watch);
		
		super.Delete();
	}
}
