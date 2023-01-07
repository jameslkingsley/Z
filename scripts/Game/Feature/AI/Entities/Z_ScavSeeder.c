[EntityEditorProps(category: "Z/Entities", description: "")]
class Z_ScavSeederEntityClass: GenericEntityClass
{}

class Z_ScavSeederEntity: GenericEntity
{
	[Attribute("100", UIWidgets.Auto, "Number of encounters to spawn around this seeder")]
	int m_NumberOfEncounters;
	
	[Attribute("500", UIWidgets.Auto, "Radius to spawn encounters within")]
	int m_Radius;
	
	[Attribute("3", UIWidgets.ComboBox, "Importance of encounters", "", ParamEnumArray.FromEnum(Z_ScavEncounterImportance))]
	Z_ScavEncounterImportance m_Importance;
	
	void Z_ScavSeederEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
	
	override protected void EOnInit(IEntity owner)
	{
		if (! Replication.IsServer() || ! GetGame().InPlayMode()) return;
		
		if (Z_Core.GetInstance().HasSeededScavEncounters()) return;
		
		for (int i = 0; i < m_NumberOfEncounters; i++)
		{
			RandomGenerator gen();
			
			vector origin = gen.GenerateRandomPointInRadius(1, m_Radius, GetOrigin(), false);
			
			Z_ScavEncounter.Create(origin, m_Importance);
		}
	}
}
