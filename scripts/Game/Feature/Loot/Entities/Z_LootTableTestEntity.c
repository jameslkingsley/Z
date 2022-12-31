[EntityEditorProps(category: "Z/Entities", description: "")]
class Z_LootTableTestEntityClass: GenericEntityClass
{
};

class Z_LootTableTestEntity : GenericEntity
{
	[Attribute("0", UIWidgets.CheckBox, "Enable of disable the debug tool")]
	bool m_Enabled;
	
	void Z_LootVolumeEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
	
	override protected void EOnInit(IEntity owner)
	{
		if (! m_Enabled || ! GetGame().InPlayMode()) return;
		
		if (! Replication.IsServer())
		{
			return;
		}
		
		GetGame().GetCallqueue().CallLater(DoSpawn, 100);
	}
	
	void DoSpawn()
	{
		ref array<ref Z_LootTable> tables = Z_LootGameModeComponent.GetInstance().GetLootTables();
		
		int currentOffset = 0;
		
		foreach (Z_LootTable table : tables)
		{
			EntitySpawnParams spawnParams = EntitySpawnParams();
			spawnParams.TransformMode = ETransformMode.WORLD;
			spawnParams.Transform[3] = GetSpawnOrigin(table) + Vector(currentOffset, 0, 0);
			
			IEntity ent = GetGame().SpawnEntityPrefab(
				Resource.Load(table.m_Resource),
				GetGame().GetWorld(),
				spawnParams
			);
			
			ent.SetYawPitchRoll(GetSpawnYawPitchRoll(table));
			
			currentOffset++;
		}
		
		Print("Spawned entire loot table for testing");
	}
	
	vector GetSpawnOrigin(Z_LootTable table)
	{
		vector origin = GetOrigin();
		
		if (table.m_TransformOffset)
		{
			// Might need converting to world space...
			origin = origin + table.m_TransformOffset;
		}
		
		return origin;
	}
	
	vector GetSpawnYawPitchRoll(Z_LootTable table)
	{
		if (table.m_YawPitchRoll)
		{
			return table.m_YawPitchRoll;
		}
		
		return Vector(0, 0, 0);
	}
}
