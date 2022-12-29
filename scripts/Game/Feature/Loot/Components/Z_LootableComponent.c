[ComponentEditorProps(category: "Z/Components", description: "")]
class Z_LootableComponentClass: ScriptComponentClass
{
};

class Z_LootableComponent : ScriptComponent
{
	vector m_InitialSpawnOrigin;
	int m_InitialSpawnTimestampInHours;
	
	override event void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		if (! EL_PersistenceManager.IsPersistenceMaster()) return;
		
		// Must be called next frame so that this component has the correct data to use
		GetGame().GetCallqueue().Call(PostInitPostFrame);
	}
	
	override event protected void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		
		if (! EL_PersistenceManager.IsPersistenceMaster()) return;
		
		if (owner)
		{
			Z_LootGameModeComponent gameMode = Z_LootGameModeComponent.GetInstance();
			
			if (gameMode) gameMode.UnregisterLootableEntity(owner);
		}
	}
	
	void PostInitPostFrame()
	{
		IEntity owner = GetOwner();
		
		if (owner)
		{
			Z_LootGameModeComponent gameMode = Z_LootGameModeComponent.GetInstance();
			
			if (gameMode)
			{
				// Register the lootable if it wasn't cleaned up
				if (! CleanupIfStale())
				{
					gameMode.RegisterLootableEntity(owner);
				}
			}
		}
	}
	
	vector GetInitialSpawnOrigin()
	{
		return m_InitialSpawnOrigin;
	}
	
	void SetInitialSpawnOrigin(vector origin)
	{
		m_InitialSpawnOrigin = origin;
	}
	
	int GetInitialSpawnTimestampInHours()
	{
		return m_InitialSpawnTimestampInHours;
	}
	
	void SetInitialSpawnTimestampInHours(int hours)
	{
		m_InitialSpawnTimestampInHours = hours;
	}
	
	bool CleanupIfStale()
	{
		if (! m_InitialSpawnOrigin) return false;
		
		if (! IsStale()) return false;
		
		// If the current position is different from the initially spawned position,
		// then do not cleanup the item. This likely means a player moved it.
		if (GetOwner().GetOrigin() != m_InitialSpawnOrigin) return false;
		
		delete GetOwner();
		
		return true;
	}
	
	bool IsStale()
	{
		if (! m_InitialSpawnTimestampInHours) return false;
		
		Z_LootGameModeComponent gameMode = Z_LootGameModeComponent.GetInstance();
			
		if (! gameMode) return false;
		
		int current = GetCurrentTimestampInHours();
		
		int diff = current - m_InitialSpawnTimestampInHours;
		
		int stalenessAgeInHours = gameMode.GetLootableStaleAgeInHours();
		
		return diff >= stalenessAgeInHours;
	}
	
	void SetInitialSpawnState()
	{
		m_InitialSpawnOrigin = GetOwner().GetOrigin();
		
		m_InitialSpawnTimestampInHours = GetCurrentTimestampInHours();
		
		EL_PersistenceComponent persistence = EL_PersistenceComponent.Cast(GetOwner().FindComponent(EL_PersistenceComponent));
		
		persistence.Save();
	}
	
	int GetCurrentTimestampInHours()
	{
		int epochYear = 2021;
		int year, month, day, hour, minute, second;
		
		System.GetYearMonthDayUTC(year, month, day);
		System.GetHourMinuteSecondUTC(hour, minute, second);
		
		year = (year - epochYear) * 8760;
		month = month * 730;
		day = day * 24;
		
		return year + month + day + hour;
	}
	
	Z_LootTier GetLootTier()
	{
		ResourceName resource = GetOwner().GetPrefabData().GetPrefabName();
		
		Z_LootGameModeComponent gameMode = Z_LootGameModeComponent.GetInstance();
		
		if (! gameMode) return null;
		
		ref array<ref Z_LootTable> tables = gameMode.GetLootTables();
		
		foreach (ref Z_LootTable table : tables)
		{
			if (table.m_Resource == resource)
			{
				return table.m_Tier;
			}
		}
		
		return null;
	}
};
