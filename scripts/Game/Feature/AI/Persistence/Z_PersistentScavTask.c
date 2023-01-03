[EL_DbName(Z_PersistentScavTaskSaveData, "Z_ScavTask")]
class Z_PersistentScavTaskSaveData : EL_ScriptedStateSaveDataBase
{
    string m_Task;
    vector m_Origin;
    ref array<ref Z_ScavTaskEntityStub> m_EntityStubs;
}

[EL_PersistentScriptedStateSettings(Z_PersistentScavTask, Z_PersistentScavTaskSaveData, autoSave: false, shutDownSave: true, selfDelete: false)]
class Z_PersistentScavTask : EL_PersistentScriptedStateBase
{
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
		
		return instance;
	}
	
	void Spawn()
	{
		Z_ScavTaskBase task = Z_ScavTaskFactory.Make(m_Task);
		
		map<IEntity, ref Z_ScavTaskEntityStub> entities();
		
		task.Spawn(this, entities);
		
		GetGame().GetCallqueue().CallLaterByName(this, "Watch", 2000, true, entities);
	}
	
	void Watch(map<IEntity, ref Z_ScavTaskEntityStub> entities)
	{
		Print("Updating scav task entities: " + entities.Count());
		
		ref array<ref Z_ScavTaskEntityStub> newStubs();
		
		foreach (IEntity ent, ref Z_ScavTaskEntityStub stub : entities)
		{
			stub.Fill(ent);
			
			newStubs.Insert(stub);
		}
		
		m_EntityStubs = newStubs;
		
		Save();
	}
	
	override void Delete()
	{
		GetGame().GetCallqueue().Remove(Watch);
		
		super.Delete();
	}
}
