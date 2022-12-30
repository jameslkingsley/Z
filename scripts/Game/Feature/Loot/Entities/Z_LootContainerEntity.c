[EntityEditorProps(category: "Z/Entities", description: "")]
class Z_LootContainerEntityClass: GenericEntityClass
{}

class Z_LootContainerEntity: GenericEntity
{
	void Z_LootContainerEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
	
	override event protected void EOnInit(IEntity owner)
	{
		if (! Replication.IsServer()) return;
	}
	
	IEntity SpawnLootable(Z_LootTable table)
	{
		if (! Replication.IsServer()) return null;
		
		IEntity ent;
		
		EntitySpawnParams spawnParams = EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[3] = GetSpawnOrigin(table);
		
		ent = GetGame().SpawnEntityPrefab(
			Resource.Load(table.m_Resource),
			GetGame().GetWorld(),
			spawnParams
		);
		
		ent.SetYawPitchRoll(GetSpawnYawPitchRoll(table));
		
		Z_LootableComponent lootableComponent = Z_LootableComponent.Cast(ent.FindComponent(Z_LootableComponent));
		
		if (! lootableComponent)
		{
			delete ent;
			
			Print("Lootable entity does not have lootable component: " + table.m_Resource.GetPath(), LogLevel.ERROR);
			
			return null;
		}
		
		lootableComponent.SetInitialSpawnState();
		
		return ent;
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
