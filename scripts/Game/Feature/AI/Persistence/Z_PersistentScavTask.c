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
	
	static Z_PersistentScavTask Create(Z_ScavTaskBase task, vector origin)
	{
		Z_PersistentScavTask instance();
		
		instance.m_Task = Z_ScavTask.Get(task.Type());
		instance.m_Origin = origin;
		instance.m_EntityStubs = task.GetEntityStubs(instance);
		
		return instance;
	}
	
	vector GetOrigin()
	{
		return m_Origin;
	}
	
	string GetTaskType()
	{
		return m_Task;
	}
	
	bool IsEmpty()
	{
		return m_EntityStubs.IsEmpty();
	}
	
	bool HasSpawned()
	{
		string persistentId = GetPersistentId();
		
		if (! persistentId)
		{
			return false;
		}
		
		return Z_ScavGameModeComponent.GetInstance().GetManagedTasks().Contains(persistentId);
	}
	
	void Despawn(IEntity region)
	{
		Z_ScavRegionComponent regionComponent = Z_ScavRegionComponent.Cast(region.FindComponent(Z_ScavRegionComponent));
		
		string persistentId = GetPersistentId();
		
		if (! persistentId)
		{
			return;
		}
		
		Z_ScavGameModeComponent gameMode = Z_ScavGameModeComponent.GetInstance();
		
		if (! gameMode) return;
		
		if (! gameMode.GetManagedTasks().Contains(persistentId)) return;
		
		array<ref Z_ScavTaskEntityStubInternal> internalStubs = gameMode.GetManagedTasks().Get(persistentId);
		
		Update(internalStubs);
		
		foreach (Z_ScavTaskEntityStubInternal stub : internalStubs)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(stub.ent);
		}
		
		gameMode.GetManagedTasks().Remove(persistentId);
		
		if (IsEmpty())
		{
			Delete();
			
			regionComponent.UnregisterTask(persistentId);
		}
	}
	
	void Spawn(IEntity region)
	{
		Z_ScavRegionComponent regionComponent = Z_ScavRegionComponent.Cast(region.FindComponent(Z_ScavRegionComponent));
		
		typename taskType = Z_ScavTask.GetTypeByName(m_Task);
		
		Z_ScavTaskBase task = regionComponent.GetAllowedTaskByType(taskType);
		
		SCR_AIGroup.IgnoreSpawning(true);
		
		array<ref Z_ScavTaskEntityStubInternal> internalStubs = task.SpawnEntityStubs(m_Origin, m_EntityStubs);
		
		SCR_AIGroup.IgnoreSpawning(false);
		
		string persistentId = GetPersistentId();
		
		if (! persistentId)
		{
			Save();
			
			persistentId = GetPersistentId();
		}
		
		Z_ScavGameModeComponent.GetInstance().GetManagedTasks().Set(persistentId, internalStubs);
	}
	
	void Update(notnull array<ref Z_ScavTaskEntityStubInternal> internalStubs)
	{
		array<ref Z_ScavTaskEntityStub> newStubs();
		
		foreach (Z_ScavTaskEntityStubInternal stub : internalStubs)
		{
			if (! stub.ent) continue;
			
			SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(stub.ent);
				
			if (! char) continue;
			
			CharacterControllerComponent charController = char.GetCharacterController();
			
			if (! charController) continue;
			
			if (charController.IsDead()) continue;
			
			Z_ScavTaskEntityStub newStub();
			
			newStub.resource = stub.resource;
			newStub.origin = stub.ent.GetOrigin();
			newStub.direction = charController.GetMovementDirWorld();
			newStub.stance = charController.GetStance();
			
			newStubs.Insert(newStub);
		}
		
		m_EntityStubs = newStubs;
	}
	
	override EL_ScriptedStateSaveDataBase Save()
	{
		if (EL_PersistenceManager.GetInstance().GetState() == EL_EPersistenceManagerState.SHUTDOWN)
		{
			string persistentId = GetPersistentId();
		
			if (persistentId)
			{
				Z_ScavGameModeComponent gameMode = Z_ScavGameModeComponent.GetInstance();
			
				if (gameMode && gameMode.GetManagedTasks().Contains(persistentId))
				{
					array<ref Z_ScavTaskEntityStubInternal> internalStubs = gameMode.GetManagedTasks().Get(persistentId);
				
					Update(internalStubs);
					
					if (IsEmpty())
					{
						Delete();
						
						return null;
					}
				}
			}
		}
		
		return super.Save();
	}
}
