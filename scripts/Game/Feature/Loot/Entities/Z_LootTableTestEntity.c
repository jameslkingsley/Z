[EntityEditorProps(category: "Z/Entities", description: "")]
class Z_LootTableTestEntityClass: GenericEntityClass
{
};

class Z_LootTableTestEntity : GenericEntity
{
	void Z_LootTableTestEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
	
	override protected void EOnInit(IEntity owner)
	{
		DoSpawn();
	}
	
	void DoSpawn()
	{
		array<ref Z_LootTable> tables = Z_LootGameModeComponent.GetInstance().GetLootTables();
		
		float currentOffset = 0;
		
		foreach (Z_LootTable table : tables)
		{
			if (! table.m_Enabled) continue;
			if (! table.m_Resource) continue;
			
			EntitySpawnParams spawnParams = EntitySpawnParams();
			spawnParams.TransformMode = ETransformMode.WORLD;
			spawnParams.Transform[3] = GetSpawnOrigin(table) + Vector(currentOffset, 0, 0);
			
			IEntity ent = GetGame().SpawnEntityPrefab(
				Resource.Load(table.m_Resource),
				GetGame().GetWorld(),
				spawnParams
			);
			
			if (! ent.FindComponent(Z_LootableComponent) || ! ent.FindComponent(EL_PersistenceComponent))
			{
				ent.SetOrigin(ent.GetOrigin() + Vector(0, 1, 0));
				Print("Loot table item is missing core components", LogLevel.ERROR);
			}
			
			ent.SetYawPitchRoll(GetSpawnYawPitchRoll(table));
			
			currentOffset += 0.66;
		}
		
		Print("Spawned entire loot table for testing", LogLevel.WARNING);
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
