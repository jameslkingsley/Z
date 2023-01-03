[EL_ComponentSaveDataType(Z_ScavRegionComponentSaveData, Z_ScavRegionComponent, "Z_ScavRegionComponent"), BaseContainerProps()]
class Z_ScavRegionComponentSaveData : EL_ComponentSaveDataBase
{
	int m_Attrition;
	
	ref array<string> m_TaskIds;
	
	// ref array<ref Z_ScavEncounter> m_Encounters = new ref array<ref Z_ScavEncounter>();
	
	override bool ReadFrom(notnull GenericComponent worldEntityComponent)
	{
		Z_ScavRegionComponent region = Z_ScavRegionComponent.Cast(worldEntityComponent);
		
		m_Attrition = region.m_Attrition;
		
		array<string> taskIds();
		
		foreach (string id, ref Z_PersistentScavTask task : region.m_Tasks)
		{
			taskIds.Insert(id);
		}
		
		m_TaskIds = taskIds;
		
		return true;
	}

	override bool ApplyTo(notnull GenericComponent worldEntityComponent)
	{
		Z_ScavRegionComponent region = Z_ScavRegionComponent.Cast(worldEntityComponent);
		
		region.m_Attrition = m_Attrition;
		
		map<string, ref Z_PersistentScavTask> tasks();
		
		foreach (string id : m_TaskIds)
		{
			tasks.Set(id, null);
		}
		
		region.m_Tasks = tasks;
		
		return true;
	}
}
