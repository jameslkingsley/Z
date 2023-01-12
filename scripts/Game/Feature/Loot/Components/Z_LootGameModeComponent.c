[ComponentEditorProps(category: "Z/Components", description: "")]
class Z_LootGameModeComponentClass: SCR_BaseGameModeComponentClass
{}

class Z_LootGameModeComponent: SCR_BaseGameModeComponent
{
	[Attribute("10", UIWidgets.Auto, "Interval in seconds for when nearby loot volumes are collected")]
	int m_LootVolumeCollectionIntervalInSeconds;
	
	[Attribute("100", UIWidgets.Auto, "Radius of the loot volume collection sphere around players")]
	int m_LootVolumeQueryRadius;
	
	[Attribute("3600", UIWidgets.Auto, "Cooldown in seconds before loot volumes refill (if insufficiently filled)")]
	int m_LootVolumeCooldownInSeconds;
	
	[Attribute("0.8", UIWidgets.Slider, "Percentage of empty containers within volumes until they are deemed insufficiently filled", "0 1 0.1")]
	float m_VolumeInsufficiencyPercentage;
	
	[Attribute("2", UIWidgets.Auto, "Age of a lootable in hours before it is considered stale (and will be cleaned up on next restart)", "1 168")]
	int m_LootableStaleAgeInHours;

	[Attribute("{82F1EADE5E5A0569}Config/Z_LootTableConfig.conf", UIWidgets.ResourceNamePicker, "Loot table config")]
	ResourceName m_LootTableConfig;
	
	ref Z_LootTableConfig m_LootTableConfigCache;
	
	ref array<Z_LootRegionComponent> m_LootRegions = {};
	
	ref map<string, ref array<IEntity>> m_LootableEntities = new ref map<string, ref array<IEntity>>();
	
	ref map<string, ref array<IEntity>> m_LootVolumeEntities = new ref map<string, ref array<IEntity>>();
	
	ref array<int> m_PlayerIds = {};
	
	static Z_LootGameModeComponent GetInstance()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		
		if (gameMode)
			return Z_LootGameModeComponent.Cast(gameMode.FindComponent(Z_LootGameModeComponent));
		else
			return null;
	}
	
	override void OnWorldPostProcess(World world)
	{
		if (! Replication.IsServer() || ! GetGame().InPlayMode()) return;
		
		GetGame().GetCallqueue().CallLater(CollectLootVolumes, m_LootVolumeCollectionIntervalInSeconds * 1000, true);
		
		Print("---- ReforgerZ Loot OnWorldPostProcess Complete ----");
	}
	
	int GetLootVolumeCooldown()
	{
		return m_LootVolumeCooldownInSeconds;
	}
	
	int GetLootableStaleAgeInHours()
	{
		return m_LootableStaleAgeInHours;
	}

	float GetVolumeInsufficiencyPercentage()
	{
		return m_VolumeInsufficiencyPercentage;
	}

	ref array<ref Z_LootTable> GetLootTables()
	{
		if (m_LootTableConfigCache)
		{
			return m_LootTableConfigCache.m_LootTables;
		}
		
		Resource container = BaseContainerTools.LoadContainer(m_LootTableConfig);
					
		m_LootTableConfigCache = Z_LootTableConfig.Cast(
			BaseContainerTools.CreateInstanceFromContainer(container.GetResource().ToBaseContainer())
		);
		
		return m_LootTableConfigCache.m_LootTables;
	}

	void RegisterLootRegion(Z_LootRegionComponent region)
	{
		if (m_LootRegions.Contains(region))
			return;
		
		m_LootRegions.Insert(region);
	}
	
	void RegisterLootVolume(IEntity ent)
	{
		string cell = SCR_MapEntity.GetGridPos(ent.GetOrigin());
		
		if (m_LootVolumeEntities.Contains(cell))
		{
			if (! m_LootVolumeEntities.Get(cell).Contains(ent))
			{
				m_LootVolumeEntities.Get(cell).Insert(ent);
			}
		}
		else
		{
			ref array<IEntity> ents();
			ents.Insert(ent);
			m_LootVolumeEntities.Set(cell, ents);
		}
	}

	void RegisterLootableEntity(IEntity ent)
	{
		string cell = SCR_MapEntity.GetGridPos(ent.GetOrigin());
		
		if (m_LootableEntities.Contains(cell))
		{
			if (! m_LootableEntities.Get(cell).Contains(ent))
			{
				m_LootableEntities.Get(cell).Insert(ent);
			}
		}
		else
		{
			ref array<IEntity> ents();
			ents.Insert(ent);
			m_LootableEntities.Set(cell, ents);
		}
	}
	
	void UnregisterLootableEntity(IEntity ent)
	{
		string cell = SCR_MapEntity.GetGridPos(ent.GetOrigin());
		
		if (m_LootableEntities.Contains(cell))
		{
			int index = m_LootableEntities.Get(cell).Find(ent);
		
			if (index > -1)
			{
				m_LootableEntities.Get(cell).Remove(index);
			}
		}
	}
	
	ref array<IEntity> GetLootVolumeEntitiesInCell(string cell)
	{
		if (m_LootVolumeEntities.Contains(cell))
		{
			return m_LootVolumeEntities.Get(cell);
		}
		
		return new ref array<IEntity>();
	}

	ref array<IEntity> GetLootableEntitiesInCell(string cell)
	{
		if (m_LootableEntities.Contains(cell))
		{
			return m_LootableEntities.Get(cell);
		}
		
		return new ref array<IEntity>();
	}
	
	Z_LootRegionComponent GetLootRegionThatSurroundsEntity(IEntity ent)
	{
		Tuple2<Z_LootRegionComponent, float> current(null, -1);
		
		foreach (Z_LootRegionComponent region : m_LootRegions)
		{
			PolylineArea area = region.GetPolylineArea();
			
			if (! area) continue;
			
			if (! region.GetPolylineArea().IsEntityInside(ent)) continue;
			
			float dist = GetDistanceToClosestPolylineAreaPoint(area, ent.GetOrigin());
			
			if (dist == -1) continue;
			
			if (current.param2 == -1 || dist < current.param2)
			{
				current.param1 = region;
				current.param2 = dist;
			}
		}
		
		return current.param1;
	}
	
	float GetDistanceToClosestPolylineAreaPoint(PolylineArea area, vector origin)
	{
		PolylineShapeEntity shape = PolylineShapeEntity.Cast(area.GetChildren());
		
		if (! shape)
		{
			Print(string.Format("PolylineArea %1 does not contain a PolylineShapeEntity", area.GetName()), LogLevel.ERROR);
			
			return -1;
		}
		
		array<vector> points();
		shape.GetPointsPositions(points);
		
		
		float closest = -1;
		
		foreach (vector point : points)
		{
			vector pointInWorld = shape.CoordToParent(point);
			
			float dist = vector.DistanceXZ(origin, pointInWorld);
			
			if (closest == -1 || dist < closest)
			{
				closest = dist;
			}
		}
		
		return closest;
	}
	
	void CollectLootVolumes()
	{
		if (! Replication.Runtime()) return;
		
		foreach (string cell, ref array<IEntity> volumes : m_LootVolumeEntities)
		{
			// Add second arg to specify number of grid cells around cell arg to check
			// Split string and add/subtract to surround the player cell arg
			if (Z_Core.IsPlayerInsideCell(cell))
			{
				// Load loot volume
				// After checking whether to load (check if already loaded, ignored etc)
				// If previously loaded then tell all containers to load
				// Containers store loot table instance on them + entity
				// Container needs to store spawn origin so it can know when item is looted
				// Possibly just use lootable component to determine this since most logic is already written there
				// Lastly call existing code for filling volume - loaded containers will be excluded from empty list
				// Mark volume as loaded + previously loaded
			}
			else
			{
				// Unload loot volume
				// Loop all containers and if they have a tracked entity then store the table and delete the entity + unregister from lootables
				// Delete remaining lootables in cell (that haven't been moved by players)
				// Mark volume as unloaded
			}
		}
		
		m_PlayerIds.Clear();
		GetGame().GetPlayerManager().GetPlayers(m_PlayerIds);
		
		if (! m_PlayerIds) return;
		
		// TODO Group players together by position
		// No point running near enough the same query if two players are together
		
		for (int i = 0, count = m_PlayerIds.Count(); i < count; i++)
		{
			IEntity playerEnt = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_PlayerIds.Get(i));
			
			if (! playerEnt) continue;
			
			GetGame().GetWorld().QueryEntitiesBySphere(
				playerEnt.GetOrigin(),
				m_LootVolumeQueryRadius,
				GetLootVolumeEntity,
				FilterLootVolumeEntities,
				EQueryEntitiesFlags.ALL
			);
		}
	}
	
	void LoadLootVolume(Z_LootVolumeEntity vol)
	{
		if (! vol.IsSetup()) vol.Setup();
		
		if (vol.IsIgnored() || vol.IsInCooldown()) return;
		
		if (vol.HasPlayersInside(m_PlayerIds)) return;
		
		ref array<IEntity> lootables();
		
		if (vol.HasSufficientLoot(lootables)) return;
		
		vol.Refill(lootables);
	}
	
	bool GetLootVolumeEntity(IEntity ent)
	{
		if (ent.Type() == Z_LootVolumeEntity)
		{
			Z_LootVolumeEntity vol = Z_LootVolumeEntity.Cast(ent);
			
			if (! vol.IsSetup()) vol.Setup();
			
			if (vol.IsIgnored() || vol.IsInCooldown()) return true;
			
			if (vol.HasPlayersInside(m_PlayerIds)) return true;
			
			ref array<IEntity> lootables();
			
			if (vol.HasSufficientLoot(lootables)) return true;
			
			vol.Refill(lootables);
		}

		return true;
	}
	
	bool FilterLootVolumeEntities(IEntity ent)
	{
		if (ent.Type() == Z_LootVolumeEntity)
		{
			return true;
		}

		return false;
	}
}
