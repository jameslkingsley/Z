[EL_DbName(Z_PersistentScavTaskSaveData, "Z_ScavTask")]
class Z_PersistentScavTaskSaveData : EL_ScriptedStateSaveDataBase
{
    string m_Task;
    vector m_Origin;
    ref array<ref Z_ScavTaskEntityStub> m_EntityStubs;
}

[EL_PersistentScriptedStateSettings(Z_PersistentScavTask, Z_PersistentScavTaskSaveData, autoSave: true, shutDownSave: true, selfDelete: false)]
class Z_PersistentScavTask : EL_PersistentScriptedStateBase
{
	string m_Task;
	vector m_Origin;
	ref array<ref Z_ScavTaskEntityStub> m_EntityStubs;
	
	static Z_PersistentScavTask Create(typename taskType, vector origin)
	{
		Z_PersistentScavTask instance();
		
		string task = Z_ScavTask.Get(taskType);
		
		Z_ScavTaskBase taskInstance = Z_ScavTaskFactory.Make(task);
		
		instance.m_Task = task;
		instance.m_Origin = origin;
		instance.m_EntityStubs = taskInstance.GetEntityStubs(instance);
		
		return instance;
	}
	
	static Z_PersistentScavTask Create(Z_ScavTaskBase task, vector origin)
	{
		Z_PersistentScavTask instance();
		
		instance.m_Task = Z_ScavTask.Get(task.Type());
		instance.m_Origin = origin;
		instance.m_EntityStubs = task.GetEntityStubs(instance);
		
		return instance;
	}
	
	void Spawn(IEntity region)
	{
		Z_ScavTaskBase task = Z_ScavTaskFactory.Make(m_Task);
		
		ref map<IEntity, ref Z_ScavTaskEntityStub> entities = task.SpawnEntityStubs(m_EntityStubs);
		
		Save();
		
		GetGame().GetCallqueue().CallLater(Watch, 2000, true, region, entities);
	}
	
	void Watch(IEntity region, map<IEntity, ref Z_ScavTaskEntityStub> entities)
	{
		if (entities.IsEmpty()) return;
		
		Z_ScavTaskBase task = Z_ScavTaskFactory.Make(m_Task);
		
		m_EntityStubs = task.UpdateEntityStubs(entities);
		
		if (! m_EntityStubs.IsEmpty()) return;
		
		GetGame().GetCallqueue().Remove(Watch);
		
		if (! region) return;
		
		Z_ScavRegionComponent regionComponent = Z_ScavRegionComponent.Cast(region.FindComponent(Z_ScavRegionComponent));
		
		if (! regionComponent) return;
		
		string persistentId = GetPersistentId();
		
		Delete();
		
		GetGame().GetCallqueue().CallByName(regionComponent, "UnregisterTask", persistentId);
	}
}
