[ComponentEditorProps(category: "Z/Components", description: "")]
class Z_ScavRegionComponentClass : ScriptComponentClass
{};

class Z_ScavRegionComponent : ScriptComponent
{
	[Attribute("0", UIWidgets.ComboBox, "Difficulty of AI in this region", "", ParamEnumArray.FromEnum(Z_ScavDifficulty))]
	Z_ScavDifficulty m_Difficulty;
	
	[Attribute("1000", UIWidgets.Auto, "Starting attrition for this region")]
	int m_StartingAttrition;
	
	[Attribute("10", UIWidgets.Auto, "Attrition gained per hour (baseline)")]
	int m_BaseAttritionGain;
	
	[Attribute("1", UIWidgets.Auto, "List of allowed tasks within this region")]
	ref array<ref Z_ScavTaskBase> m_AllowedTasks;
	
	// Persistent props below
	
	int m_Attrition;
	
	ref array<ref Z_ScavEncounter> m_Encounters;
	
	ref map<string, Z_PersistentScavTask> m_Tasks;
	
	void Z_ScavRegionComponent()
	{
		m_Encounters = new array<ref Z_ScavEncounter>();
		m_Tasks = new map<string, Z_PersistentScavTask>();
	}
	
	// Similar to loot volume - this class is responsible for choosing where to spawn things but does not do the spawning itself
	
	// Spawn is determined using heat map of encounters
	// Some randomisation is applied to that spawn position
	// If heat map is not dense enough (ie no encounters) then choose random position inside region
	// Position chosen must be nowhere near players
	// Possibly have generous delay on game start before spawning AI, to cater for when a server restarts and players reconnect
	// - don't want them spawning in into a firefight
	
	// AI is not persisted by default
	// A ScavTask is a persisted entity that stores:
	// - where it is (should automatically be handled for us)
	// - task info (type, parameters etc)
	// - how many scavs are left (started with 5, 2 died, so only spawn 3)
	// - each scav's position so they can spawn back in same place
	// - class can extend a base which manages the persistence
	// - sub-class can just deal with waypointing
	// - spawning is handled in base class (as it needs tracking)
	
	protected override void OnPostInit(IEntity owner)
	{
		if (! GetGame().InPlayMode()) return;
		
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		if (! rplComponent)
		{
			Debug.Error("Scav region owner entity is missing RplComponent");
		}
		
		EL_PersistenceComponent persistence = EL_PersistenceComponent.Cast(owner.FindComponent(EL_PersistenceComponent));
		
		if (! persistence)
		{
			Debug.Error("Scav region must have a persistence component");
		}
		
		if (rplComponent.Role() == RplRole.Authority)
		{
			Z_ScavGameModeComponent gameMode = Z_ScavGameModeComponent.GetInstance();
		
			if (! gameMode) return;
			
			gameMode.RegisterScavRegion(this);
			
			LoadTasksAsync();
		}
	}
	
	void LoadTasksAsync()
	{
		EL_PersistenceComponent persistence = EL_PersistenceComponent.Cast(GetOwner().FindComponent(EL_PersistenceComponent));
		
		if (m_Tasks.IsEmpty())
		{
			Print("Scav region has no tasks in persistence", LogLevel.WARNING);
			
			return;
		}
		
		array<string> taskIds();
		foreach (string id, Z_PersistentScavTask task : m_Tasks)
		{
			taskIds.Insert(id);
		}
			
		array<ref Z_PersistentScavTask> tasks = EL_PersistentScriptedStateLoader<Z_PersistentScavTask>.Load(taskIds);
		
		if (tasks.IsEmpty())
		{
			Print("Scav region could not resolve tasks from database", LogLevel.ERROR);
			
			return;
		}
		
		foreach (ref Z_PersistentScavTask task : tasks)
		{
			task.Spawn();
			
			RegisterTask(task);
		}
		
		persistence.Save();
	}
	
	void RegisterTask(Z_PersistentScavTask task)
	{
		m_Tasks.Set(task.GetPersistentId(), task);
		
		Print("Region has new task: " + task.GetPersistentId());
		
		foreach (string id, auto _ : m_Tasks)
		{
			Print("- " + id);
		}
	}
	
	void UnregisterTask(Z_PersistentScavTask task)
	{
		m_Tasks.Remove(task.GetPersistentId());
		
		Print("Region removed existing task: " + task.GetPersistentId());
	}
	
	PolylineArea GetPolylineArea()
	{
		return PolylineArea.Cast(GetOwner());
	}
}