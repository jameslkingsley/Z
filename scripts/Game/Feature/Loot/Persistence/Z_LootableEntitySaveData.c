[EL_ComponentSaveDataType(Z_LootableEntitySaveData, Z_LootableComponent, "Z_Lootable"), BaseContainerProps()]
class Z_LootableEntitySaveData : EL_ComponentSaveDataBase
{
	vector m_InitialSpawnOrigin;
	int m_CleanupAtYear, m_CleanupAtMonth, m_CleanupAtDay, m_CleanupAtHour, m_CleanupAtMinute, m_CleanupAtSecond;
	
	override bool ReadFrom(notnull GenericComponent worldEntityComponent)
	{
		Z_LootableComponent comp = Z_LootableComponent.Cast(worldEntityComponent);
		
		m_InitialSpawnOrigin = comp.m_InitialSpawnOrigin;
		m_CleanupAtYear = comp.m_CleanupAtYear;
		m_CleanupAtMonth = comp.m_CleanupAtMonth;
		m_CleanupAtDay = comp.m_CleanupAtDay;
		m_CleanupAtHour = comp.m_CleanupAtHour;
		m_CleanupAtMinute = comp.m_CleanupAtMinute;
		m_CleanupAtSecond = comp.m_CleanupAtSecond;
		
		return true;
	}

	override bool ApplyTo(notnull GenericComponent worldEntityComponent)
	{
		Z_LootableComponent comp = Z_LootableComponent.Cast(worldEntityComponent);
		
		comp.m_InitialSpawnOrigin = m_InitialSpawnOrigin;
		comp.m_CleanupAtYear = m_CleanupAtYear;
		comp.m_CleanupAtMonth = m_CleanupAtMonth;
		comp.m_CleanupAtDay = m_CleanupAtDay;
		comp.m_CleanupAtHour = m_CleanupAtHour;
		comp.m_CleanupAtMinute = m_CleanupAtMinute;
		comp.m_CleanupAtSecond = m_CleanupAtSecond;
		
		return true;
	}
	
	/*
	override protected bool SerializationSave(BaseSerializationSaveContext saveContext)
	{
		saveContext.WriteValue("m_InitialSpawnOrigin", m_InitialSpawnOrigin);
		saveContext.WriteValue("m_CleanupAtYear", m_CleanupAtYear);
		saveContext.WriteValue("m_CleanupAtMonth", m_CleanupAtMonth);
		saveContext.WriteValue("m_CleanupAtDay", m_CleanupAtDay);
		saveContext.WriteValue("m_CleanupAtHour", m_CleanupAtHour);
		saveContext.WriteValue("m_CleanupAtMinute", m_CleanupAtMinute);
		saveContext.WriteValue("m_CleanupAtSecond", m_CleanupAtSecond);

		return true;
	}

	override protected bool SerializationLoad(BaseSerializationLoadContext loadContext)
	{
		loadContext.ReadValue("m_InitialSpawnOrigin", m_InitialSpawnOrigin);
		saveContext.ReadValue("m_CleanupAtYear", m_CleanupAtYear);
		saveContext.ReadValue("m_CleanupAtMonth", m_CleanupAtMonth);
		saveContext.ReadValue("m_CleanupAtDay", m_CleanupAtDay);
		saveContext.ReadValue("m_CleanupAtHour", m_CleanupAtHour);
		saveContext.ReadValue("m_CleanupAtMinute", m_CleanupAtMinute);
		saveContext.ReadValue("m_CleanupAtSecond", m_CleanupAtSecond);

		return true;
	}
	*/
}
