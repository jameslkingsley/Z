[ComponentEditorProps(category: "Z/Components", description: "")]
class Z_ScavRegionComponentClass : ScriptComponentClass
{};

class Z_ScavRegionComponent : ScriptComponent
{
	[Attribute("0", UIWidgets.ComboBox, "Difficulty of AI in this region", "", ParamEnumArray.FromEnum(Z_ScavDifficulty))]
	Z_ScavDifficulty m_Difficulty;
	
	[Attribute("1000", UIWidgets.Auto, "Starting attrition for this region")]
	int m_StartingAttrition;
	
	[Attribute("1000", UIWidgets.Auto, "Maximum attrition for this region (gains won't stack)")]
	int m_MaximumAttrition;
	
	[Attribute("1", UIWidgets.Auto, "Attrition gained per minute (baseline)")]
	int m_BaseAttritionGain;
	
	// TODO Attrition gain modifiers (such as player destroying convoy = attrition deboost)
	
	[Attribute("", UIWidgets.Auto, "List of allowed tasks within this region")]
	ref array<ref Z_ScavTaskBase> m_AllowedTasks;
	
	int m_Attrition;
	
	ref map<string, ref Z_PersistentScavTask> m_Tasks;
	
	ref map<string, int> m_CellTaskCounts;
	
	void Z_ScavRegionComponent()
	{
		m_Tasks = new map<string, ref Z_PersistentScavTask>();
		m_CellTaskCounts = new map<string, int>();
		m_Attrition = m_StartingAttrition;
	}
	
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
			
			GetGame().GetCallqueue().CallLater(AccrueAttrition, 60000, true);
		}
	}
	
	void ImpactAttrition(int impact)
	{
		int newAttrition = m_Attrition + impact;
		
		if (newAttrition >= m_MaximumAttrition)
		{
			// Print(string.Format("Region %1 has reached maximum attrition of %2", GetOwner().GetName(), m_MaximumAttrition));
			
			return;
		}
		
		// Print(string.Format("Region %1 gained %2 attrition (from external force), total is now %3", GetOwner().GetName(), impact, newAttrition));
		
		m_Attrition = newAttrition;
	}
	
	void AccrueAttrition()
	{
		int newAttrition = m_Attrition + m_BaseAttritionGain;
		
		if (newAttrition >= m_MaximumAttrition)
		{
			// Print(string.Format("Region %1 has reached maximum attrition of %2", GetOwner().GetName(), m_MaximumAttrition));
			
			return;
		}
		
		// Print(string.Format("Region %1 gained %2 attrition, total is now %3", GetOwner().GetName(), m_BaseAttritionGain, newAttrition));
		
		m_Attrition = newAttrition;
	}
	
	ref map<string, ref Z_PersistentScavTask> GetTasks()
	{
		return m_Tasks;
	}
	
	void InitializeTasks(string cell, vector origin)
	{
		if (! m_AllowedTasks || m_AllowedTasks.IsEmpty()) return;
		
		// Task limit within a cell for this region
		// TODO Add workbench attribute
		if (m_CellTaskCounts.Contains(cell) && m_CellTaskCounts.Get(cell) >= 1) return;
		
		array<Z_ScavTaskBase> affordableTasks = GetAffordableTasks();
		
		if (affordableTasks.IsEmpty()) return;
		
		array<float> weights = GetTasksAsWeights(affordableTasks);
		
		int index = SCR_ArrayHelper.GetWeightedIndex(weights, Math.RandomFloat01());
		
		if (! affordableTasks.IsIndexValid(index))
		{
			Print(string.Format("Chosen task has invalid index: %1 / %2", index, affordableTasks.Count()), LogLevel.ERROR);
			
			return;
		}
		
		Z_ScavTaskBase task = affordableTasks.Get(index);
		
		RandomGenerator gen();
		vector pos = gen.GenerateRandomPointInRadius(1, Z_HeatMap.CELL_SIZE_DIAGONAL, origin);
		
		vector finalPos;
		SCR_WorldTools.FindEmptyTerrainPosition(finalPos, pos, Z_HeatMap.CELL_SIZE_DIAGONAL);
		
		if (Z_Core.IsUnderwater(finalPos)) return;
		
		Z_PersistentScavTask persistentTask = Z_PersistentScavTask.Create(task, finalPos);
		
		persistentTask.Save();
		
		RegisterTask(persistentTask);
		
		m_Attrition -= task.m_AttritionCost;
	}
	
	array<float> GetTasksAsWeights(array<Z_ScavTaskBase> tasks)
	{
		array<int> weights();
		array<float> result();
		
		if (tasks.IsEmpty()) return result;
		
		foreach (Z_ScavTaskBase task : tasks)
		{
			weights.Insert(task.m_AttritionCost);
		}
		
		weights.Sort();
		
		int min = weights.Get(0);
		int max = weights.Get(weights.Count() - 1);
		
		foreach (int w : weights)
		{
			float weightPercent = 1;
			
			if (min < max) weightPercent = Math.InverseLerp(min, max, w);
			
			result.Insert((float) weightPercent * 100);
		}
		
		return result;
	}
	
	array<Z_ScavTaskBase> GetAffordableTasks()
	{
		array<Z_ScavTaskBase> tasks();
		
		foreach (Z_ScavTaskBase task : m_AllowedTasks)
		{
			if (task.m_AttritionCost <= m_Attrition)
			{
				tasks.Insert(task);
			}
		}
		
		SCR_Sorting<Z_ScavTaskBase, Z_ScavTaskBaseCompare>.HeapSort(tasks);
		
		return tasks;
	}
	
	// TODO Convert this back into async
	void LoadTasksAsync()
	{
		EL_PersistenceComponent persistence = EL_PersistenceComponent.Cast(GetOwner().FindComponent(EL_PersistenceComponent));
		
		if (m_Tasks.IsEmpty()) return;
		
		array<string> taskIds();
		foreach (string id, ref Z_PersistentScavTask task : m_Tasks)
		{
			taskIds.Insert(id);
		}
			
		array<ref Z_PersistentScavTask> tasks = EL_PersistentScriptedStateLoader<Z_PersistentScavTask>.Load(taskIds);
		
		if (tasks.IsEmpty())
		{
			Print("Scav region could not resolve tasks from database", LogLevel.WARNING);
			
			return;
		}
		
		foreach (Z_PersistentScavTask task : tasks)
		{
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
		
		string taskCell = SCR_MapEntity.GetGridPos(task.m_Origin);
		
		int count = 0;
		
		if (m_CellTaskCounts.Contains(taskCell))
		{
			count = m_CellTaskCounts.Get(taskCell);
		}
		
		m_CellTaskCounts.Set(taskCell, count + 1);
	}
	
	void UnregisterTask(string persistentId)
	{
		if (! m_Tasks.Contains(persistentId)) return;
		
		Z_PersistentScavTask task = m_Tasks.Get(persistentId);
		
		string taskCell = SCR_MapEntity.GetGridPos(task.m_Origin);
		
		if (m_CellTaskCounts.Contains(taskCell))
		{
			int count = m_CellTaskCounts.Get(taskCell);
			
			m_CellTaskCounts.Set(taskCell, count - 1);
		}
		
		m_Tasks.Remove(persistentId);
	}
	
	PolylineArea GetPolylineArea()
	{
		return PolylineArea.Cast(GetOwner());
	}
}

class Z_ScavTaskBaseCompare : SCR_SortCompare<Z_ScavTaskBase>
{
	override static int Compare(Z_ScavTaskBase left, Z_ScavTaskBase right)
	{		
		if (left.m_AttritionCost < right.m_AttritionCost)
			return 1;
		else
			return 0;
	}
}
