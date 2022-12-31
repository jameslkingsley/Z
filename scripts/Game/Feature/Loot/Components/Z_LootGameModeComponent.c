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
	
	ref array<IEntity> m_LootableEntities = {};
	
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

	void RegisterLootableEntity(IEntity ent)
	{
		if (m_LootableEntities.Contains(ent))
			return;
		
		m_LootableEntities.Insert(ent);
	}
	
	void UnregisterLootableEntity(IEntity ent)
	{
		int index = m_LootableEntities.Find(ent);
		
		if (index > -1)
		{
			m_LootableEntities.Remove(index);
		}
	}

	ref array<IEntity> GetLootableEntities()
	{
		return m_LootableEntities;
	}
	
	Z_LootRegionComponent GetLootRegionThatSurroundsEntity(IEntity ent)
	{
		foreach (Z_LootRegionComponent region : m_LootRegions)
		{
			PolylineArea area = region.GetPolylineArea();
			
			if (! area)
				continue;
			
			if (region.GetPolylineArea().IsEntityInside(ent))
				return region;
		}
		
		return null;
	}
	
	void CollectLootVolumes()
	{
		if (! Replication.Runtime()) return;
		
		array<int> players = {};
		GetGame().GetPlayerManager().GetPlayers(players);
		
		if (! players) return;
		
		// TODO Group players together by position
		// No point running near enough the same query if two players are together
		
		for (int i = 0, count = players.Count(); i < count; i++)
		{
			IEntity playerEnt = GetGame().GetPlayerManager().GetPlayerControlledEntity(players.Get(i));
			
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
	
	bool GetLootVolumeEntity(IEntity ent)
	{
		if (ent.Type() == Z_LootVolumeEntity)
		{
			Z_LootVolumeEntity vol = Z_LootVolumeEntity.Cast(ent);
			
			if (! vol.IsSetup()) vol.Setup();
			
			if (vol.IsIgnored() || vol.IsInCooldown()) return true;
			
			if (vol.HasPlayersInside()) return true;
			
			ref array<IEntity> lootables = new ref array<IEntity>();
			
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
