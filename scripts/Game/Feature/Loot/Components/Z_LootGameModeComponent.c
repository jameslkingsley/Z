[ComponentEditorProps(category: "Z/Components", description: "")]
class Z_LootGameModeComponentClass: SCR_BaseGameModeComponentClass
{}

class Z_LootGameModeComponent: SCR_BaseGameModeComponent
{
	[Attribute("300", UIWidgets.Auto, "Seconds before loot volumes are dehydrated")]
	int m_LootVolumeExpiryInSeconds;
	
	[Attribute("10", UIWidgets.Auto, "Interval in seconds for when nearby loot volumes are collected")]
	int m_LootVolumeCollectionIntervalInSeconds;
	
	[Attribute("100", UIWidgets.Auto, "Radius of the loot volume collection sphere around players")]
	int m_LootVolumeQueryRadius;
	
	[Attribute("3600", UIWidgets.Auto, "Cooldown in seconds before loot volumes refill")]
	int m_LootVolumeCooldownInSeconds;

	[Attribute("{82F1EADE5E5A0569}Config/Z_LootTableConfig.conf", UIWidgets.ResourceNamePicker, "Loot table config")]
	ResourceName m_LootTableConfig;
	
	ref array<Z_LootRegionComponent> m_LootRegions = {};
	
	static Z_LootGameModeComponent GetInstance()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (gameMode)
			return Z_LootGameModeComponent.Cast(gameMode.FindComponent(Z_LootGameModeComponent));
		else
			return null;
	}
	
	override void OnPostInit(IEntity owner)
	{
		GetGame().GetCallqueue().CallLater(CollectLootVolumes, m_LootVolumeCollectionIntervalInSeconds * 1000, true);
	}
	
	int GetLootVolumeCooldown()
	{
		return m_LootVolumeCooldownInSeconds;
	}

	ref array<ref Z_LootTable> GetLootTables()
	{
		Resource container = BaseContainerTools.LoadContainer(m_LootTableConfig);
					
		Z_LootTableConfig config = Z_LootTableConfig.Cast(
			BaseContainerTools.CreateInstanceFromContainer(container.GetResource().ToBaseContainer())
		);
		
		return config.m_LootTables;
	}
	
	void RegisterLootRegion(Z_LootRegionComponent region)
	{
		if (m_LootRegions.Contains(region))
			return;
		
		m_LootRegions.Insert(region);
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
	
	void HydrateLootVolume(Z_LootVolumeEntity ent)
	{
		GetGame().GetCallqueue().RemoveByName(ent, "Dehydrate");
		
		GetGame().GetCallqueue().CallLaterByName(ent, "Dehydrate", m_LootVolumeExpiryInSeconds * 1000, false);
		
		GetGame().GetCallqueue().CallByName(ent, "Hydrate");
	}
	
	void CollectLootVolumes()
	{
		array<int> players = {};
		GetGame().GetPlayerManager().GetPlayers(players);
		
		for (int i = 0, count = players.Count(); i < count; i++)
		{
			IEntity playerEnt = GetGame().GetPlayerManager().GetPlayerControlledEntity(players.Get(i));
			
			if (! playerEnt)
				continue;
			
			// TODO Might be faster to loop all registered
			// loot volumes and check distance to each player.
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
		if (ent.Type() == Z_LootVolumeEntity) {
			HydrateLootVolume(Z_LootVolumeEntity.Cast(ent));
		}

		return true;
	}
	
	bool FilterLootVolumeEntities(IEntity ent)
	{
		if (ent.Type() == Z_LootVolumeEntity) {
			return true;
		}

		return false;
	}
}
