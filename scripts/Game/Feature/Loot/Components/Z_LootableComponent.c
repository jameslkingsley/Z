[ComponentEditorProps(category: "Z/Components", description: "")]
class Z_LootableComponentClass: ScriptComponentClass
{
};

class Z_LootableComponent : ScriptComponent
{
	vector m_InitialSpawnOrigin;
	int m_CleanupAtYear, m_CleanupAtMonth, m_CleanupAtDay, m_CleanupAtHour, m_CleanupAtMinute, m_CleanupAtSecond;
	
	override event void OnPostInit(IEntity owner)
	{
		if (! EL_PersistenceManager.IsPersistenceMaster()) return;
		
		if (owner)
		{
			Z_LootGameModeComponent gameMode = Z_LootGameModeComponent.GetInstance();
			
			if (gameMode) gameMode.RegisterLootableEntity(owner);
		}
	}
	
	override event protected void OnDelete(IEntity owner)
	{
		if (! EL_PersistenceManager.IsPersistenceMaster()) return;
		
		if (owner)
		{
			Z_LootGameModeComponent gameMode = Z_LootGameModeComponent.GetInstance();
			
			if (gameMode) gameMode.UnregisterLootableEntity(owner);
			
			// TODO Not working
			// CleanupIfStale();
		}
	}
	
	void CleanupIfStale()
	{
		if (! m_InitialSpawnOrigin) return;
		
		if (! IsStale()) return;
		
		if (GetOwner().GetOrigin() != m_InitialSpawnOrigin) return;
		
		Print("Cleaning up stale lootable");
		
		delete GetOwner();
	}
	
	bool IsStale()
	{
		ref array<int> cleanupAt = GetCleanupAt();
		ref array<int> current = GetCurrentTimestamp();
		
		Print("Checking if stale: " + cleanupAt + " " + current);
		
		foreach (int i, int value : cleanupAt)
		{
			if (! value && current[i] > value)
			{
				return true;
			}
		}
		
		Print("Cleanup at timestamp not in past");
		
		return false;
	}
	
	ref array<int> GetCleanupAt()
	{
		ref array<int> timestamp = {m_CleanupAtYear, m_CleanupAtMonth, m_CleanupAtDay, m_CleanupAtHour, m_CleanupAtMinute, m_CleanupAtSecond};
		
		return timestamp;
	}
	
	ref array<int> GetCurrentTimestamp()
	{
		ref array<int> timestamp = {};
		int year, month, day, hour, minute, second;
		
		System.GetYearMonthDayUTC(year, month, day);
		System.GetHourMinuteSecondUTC(hour, minute, second);
		
		timestamp = {year, month, day, hour, minute, second};
		
		return timestamp;
	}
	
	void SetInitialSpawnState()
	{
		Print("Setting initial spawn state");
		
		m_InitialSpawnOrigin = GetOwner().GetOrigin();
		
		ref array<int> current = GetCurrentTimestamp();
		
		m_CleanupAtYear = current[0];
		m_CleanupAtMonth = current[1];
		m_CleanupAtDay = current[2];
		m_CleanupAtHour = current[3];
		m_CleanupAtMinute = current[4] + 1;
		m_CleanupAtSecond = current[5];
		
		EL_PersistenceComponent persistence = EL_PersistenceComponent.Cast(GetOwner().FindComponent(EL_PersistenceComponent));
		
		persistence.Save();
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
