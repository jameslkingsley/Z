[EL_ComponentSaveDataType(Z_LootableEntitySaveData, Z_LootableComponent, "Z_Lootable"), BaseContainerProps()]
class Z_LootableEntitySaveData : EL_ComponentSaveDataBase
{
	vector m_InitialSpawnOrigin;
	int m_InitialSpawnTimestampInHours;
	
	override bool ReadFrom(notnull GenericComponent worldEntityComponent)
	{
		Z_LootableComponent component = Z_LootableComponent.Cast(worldEntityComponent);
		
		m_InitialSpawnOrigin = component.GetInitialSpawnOrigin();
		
		m_InitialSpawnTimestampInHours = component.GetInitialSpawnTimestampInHours();
		
		return true;
	}

	override bool ApplyTo(notnull GenericComponent worldEntityComponent)
	{
		Z_LootableComponent lootable = Z_LootableComponent.Cast(worldEntityComponent);
		
		lootable.SetInitialSpawnOrigin(m_InitialSpawnOrigin);
		
		lootable.SetInitialSpawnTimestampInHours(m_InitialSpawnTimestampInHours);
		
		return true;
	}
}
