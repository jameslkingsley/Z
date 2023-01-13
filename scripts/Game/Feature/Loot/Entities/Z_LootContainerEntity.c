[EntityEditorProps(category: "Z/Entities", description: "")]
class Z_LootContainerEntityClass: GenericEntityClass
{}

class Z_LootContainerEntity: GenericEntity
{
	IEntity m_Entity;
	ref Z_LootTable m_Table;
	
	void Load()
	{
		if (! m_Table) return;
		if (m_Entity) return;
		
		SpawnLootable(m_Table);
	}
	
	void Unload()
	{
		if (! m_Entity) return;
		
		Z_LootableComponent lootableComponent = Z_LootableComponent.Cast(m_Entity.FindComponent(Z_LootableComponent));
		
		if (! lootableComponent) return;
		
		if (lootableComponent.HasMovedFromSpawnOrigin())
		{
			lootableComponent.MarkAsLooted();
			
			m_Table = null;
			m_Entity = null;
			
			return;
		}
		
		Z_LootGameModeComponent.GetInstance().UnregisterLootableEntity(this);
		
		RplComponent.DeleteRplEntity(m_Entity, false);
		
		if (m_Entity) m_Entity = null;
	}
	
	IEntity SpawnLootable(Z_LootTable table)
	{
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
			RplComponent.DeleteRplEntity(ent, false);
			
			Print("Lootable entity does not have lootable component: " + table.m_Resource.GetPath(), LogLevel.ERROR);
			
			return null;
		}
		
		lootableComponent.SetInitialSpawnState();
		
		m_Entity = ent;
		m_Table = table;
		
		return ent;
	}
	
	vector GetSpawnOrigin(Z_LootTable table)
	{
		vector origin = GetOrigin();
		
		if (table.m_TransformOffset)
		{
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
