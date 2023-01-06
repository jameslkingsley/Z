[ComponentEditorProps(category: "Z/Components", description: "")]
class Z_ScavRegionComponentClass : ScriptComponentClass
{};

class Z_ScavRegionComponent : ScriptComponent
{
	[Attribute("0 0 0", UIWidgets.Coords, "Approximate center of the region", params: "inf inf 0 purpose=coords space=entity")]
	vector m_Centroid;
	
	[Attribute("0", UIWidgets.ComboBox, "Difficulty of AI in this region", "", ParamEnumArray.FromEnum(Z_ScavDifficulty))]
	Z_ScavDifficulty m_Difficulty;
	
	[Attribute("1000", UIWidgets.Auto, "Starting attrition for this region")]
	int m_StartingAttrition;
	
	[Attribute("10", UIWidgets.Auto, "Attrition gained per hour (baseline)")]
	int m_BaseAttritionGain;
	
	[Attribute("", UIWidgets.Auto, "List of allowed tasks within this region")]
	ref array<ref Z_ScavTaskBase> m_AllowedTasks;
	
	// Persistent props below
	
	int m_Attrition;
	
	ref array<ref Z_ScavEncounter> m_Encounters;
	
	ref map<string, ref Z_PersistentScavTask> m_Tasks;
	
	void Z_ScavRegionComponent()
	{
		m_Encounters = new array<ref Z_ScavEncounter>();
		m_Tasks = new map<string, ref Z_PersistentScavTask>();
		m_Attrition = m_StartingAttrition;
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
			
			GetGame().GetCallqueue().CallLater(LoadTasksAsync, 500);
			
			GetGame().GetCallqueue().CallLater(SpawnTasksAsync, 5000, true);
		}
	}
	
	float GetAttritionProbability()
	{
		Tuple2<int, int> weights = Z_ScavGameModeComponent.GetInstance().GetScavRegionAttritionWeights();
		
		if (weights.param2 == 0) return 0;
		
		return Math.InverseLerp(weights.param1, weights.param2, m_Attrition);
	}
	
	vector GenerateRandomPositionInsideRegion()
	{	
		vector result;
		SCR_WorldTools.FindEmptyTerrainPosition(result, GetOwner().CoordToParent(m_Centroid), 500);
		return result;
	}
	
	void SpawnTasksAsync()
	{
		// Gotta rethink this...
		//
		// Maybe iterate over all map grid cells and go from there
		// Will be tricky to seed map with realistic encounters
		// I suppose there could be a seed entity that can be dropped in to artificially seed encounters
		// Can generate coords around entity to create encounters for
		// Can then set some config on a singleton scripted state to remember we already seeded
		// Then with a seeded world, we can properly figure out where to spawn
		//
		// Iterate all grid cells, each cell has its probability
		// If rand float <= cell prob then we can decide if we should continue
		// Check region attrition of cell and available tasks
		// Spawn available task and deduct attrition
		//
		// Deciding task to spawn could use weighted array of lerp'd task attrition
		// Filter tasks by cost (to affordable ones)
		// Sort by cost ascending
		// Lerp costs into percentage (max is highest cost in list)
		// Multiply percentages by 100 to get weighted int
		// Use array utils to make weighted array to pick task
		// High cost (but affordable) tasks are higher prob than lower cost
		//
		// Spawn position would be within grid cell, could just do random radius coord + find empty terrain pos around that
		// Regions can have a maximum task count to avoid over populated areas
		// Might need to heap sort all grid cells to iterate them in random order, otherwise some places could get repeated when eating through attrition
		// Cell probabilities drive the population, so it's important to have logarithmic scale to encounter's importance
		
		vector origin = GenerateRandomPositionInsideRegion();
		
		string cell = SCR_MapEntity.GetGridPos(origin);
		
		float prob = Z_HeatMap.GetProbability(cell);
		
		if (Math.RandomFloat(0, 1) <= prob)
		{
			// TODO Change to high-low order when more tasks added
			Z_ScavTaskBase taskType = m_AllowedTasks.GetRandomElement();
			
			if (m_Attrition < taskType.m_AttritionCost) return;
			
			Z_PersistentScavTask task = Z_PersistentScavTask.Create(taskType, origin);
			
			task.Spawn(GetOwner());
			
			RegisterTask(task);
			
			m_Attrition -= taskType.m_AttritionCost;
			
			Print("Spawned task inside region, new attrition: " + m_Attrition);
			
			Z_HeatMap.RecalculateProbability(cell, this);
		}
	}
	
	// TODO Convert this back into async
	void LoadTasksAsync()
	{
		EL_PersistenceComponent persistence = EL_PersistenceComponent.Cast(GetOwner().FindComponent(EL_PersistenceComponent));
		
		if (m_Tasks.IsEmpty())
		{
			Print("Scav region has no tasks in persistence", LogLevel.WARNING);
			
			return;
		}
		
		array<string> taskIds();
		foreach (string id, ref Z_PersistentScavTask task : m_Tasks)
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
			task.Spawn(GetOwner());
			
			RegisterTask(task);
		}
		
		persistence.Save();
	}
	
	Z_ScavTaskBase GetAllowedTaskByType(typename type)
	{
		foreach (Z_ScavTaskBase task : m_AllowedTasks)
		{
			if (task.Type() == type)
			{
				return task;
			}
		}
		
		return null;
	}
	
	void RegisterTask(Z_PersistentScavTask task)
	{
		m_Tasks.Set(task.GetPersistentId(), task);
	}
	
	void UnregisterTask(string persistentId)
	{
		m_Tasks.Remove(persistentId);
	}
	
	PolylineArea GetPolylineArea()
	{
		return PolylineArea.Cast(GetOwner());
	}
}
