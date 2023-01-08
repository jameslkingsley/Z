[ComponentEditorProps(category: "Z/Components", description: "")]
class Z_LootableComponentClass: ScriptComponentClass
{
};

class Z_LootableComponent : ScriptComponent
{
	vector m_InitialSpawnOrigin;
	int m_InitialSpawnTimestampInHours;
	
	protected override void OnPostInit(IEntity owner)
	{
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		if (rplComponent && rplComponent.Role() == RplRole.Authority)
		{
			SetupLootable();
		}
	}
	
	override protected void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		if (rplComponent && rplComponent.Role() == RplRole.Authority)
		{
			Z_LootGameModeComponent gameMode = Z_LootGameModeComponent.GetInstance();
			
			if (gameMode) gameMode.UnregisterLootableEntity(owner);
		}
	}
	
	void SetupLootable()
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
		
		GetGame().GetCallqueue().Call(DeleteLootable);
		
		return true;
	}
	
	void DeleteLootable()
	{
		IEntity owner = GetOwner();
		
		if (owner) RplComponent.DeleteRplEntity(owner, false);
	}
	
	bool IsStale()
	{
		if (! m_InitialSpawnTimestampInHours) return false;
		
		Z_LootGameModeComponent gameMode = Z_LootGameModeComponent.GetInstance();
			
		if (! gameMode) return false;
		
		int current = Z_Core.GetCurrentTimestampInHours();
		
		int diff = current - m_InitialSpawnTimestampInHours;
		
		int stalenessAgeInHours = gameMode.GetLootableStaleAgeInHours();
		
		return diff >= stalenessAgeInHours;
	}
	
	void SetInitialSpawnState()
	{
		m_InitialSpawnOrigin = GetOwner().GetOrigin();
		
		m_InitialSpawnTimestampInHours = Z_Core.GetCurrentTimestampInHours();
		
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
