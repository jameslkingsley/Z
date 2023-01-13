[EL_ComponentSaveDataType(Z_LootableEntitySaveData, Z_LootableComponent, "Z_Lootable"), BaseContainerProps()]
class Z_LootableEntitySaveData : EL_ComponentSaveDataBase
{
	vector m_InitialSpawnOrigin;
	int m_InitialSpawnTimestampInHours;
	bool m_Looted;
	
	override bool ReadFrom(notnull GenericComponent worldEntityComponent)
	{
		if (! super.ReadFrom(worldEntityComponent)) return false;
		
		Z_LootableComponent component = Z_LootableComponent.Cast(worldEntityComponent);
		
		m_InitialSpawnOrigin = component.GetInitialSpawnOrigin();
		
		m_InitialSpawnTimestampInHours = component.GetInitialSpawnTimestampInHours();
		
		m_Looted = component.IsLooted();
		
		return true;
	}

	override bool ApplyTo(notnull GenericComponent worldEntityComponent)
	{
		if (! super.ApplyTo(worldEntityComponent)) return false;
		
		Z_LootableComponent lootable = Z_LootableComponent.Cast(worldEntityComponent);
		
		lootable.SetInitialSpawnOrigin(m_InitialSpawnOrigin);
		
		lootable.SetInitialSpawnTimestampInHours(m_InitialSpawnTimestampInHours);
		
		lootable.m_Looted = m_Looted;
		
		return true;
	}
}
