[ComponentEditorProps(category: "Z/Components", description: "")]
class Z_LootGameModeComponentClass: SCR_BaseGameModeComponentClass
{}

class Z_LootGameModeComponent: SCR_BaseGameModeComponent
{
	[Attribute("300", UIWidgets.Auto, "Seconds before loot containers are dehydrated")]
	int m_LootContainerExpiryInSeconds;
	
	[Attribute("10", UIWidgets.Auto, "Interval in seconds for when nearby loot containers are collected")]
	int m_LootContainerCollectionIntervalInSeconds;
	
	[Attribute("500", UIWidgets.Auto, "Radius of the loot container collection sphere around players")]
	int m_LootContainerQueryRadius;
	
	[Attribute("3600", UIWidgets.Auto, "Cooldown in seconds before loot containers refill")]
	int m_LootContainerCooldownInSeconds;
	
	[Attribute("{EEEF74405A235519}Config/Z_LootTableDefinitionsConfig.conf", UIWidgets.ResourceNamePicker, "Loot table definitions config")]
	ResourceName m_LootTableDefinitionsConfig;
	
	ref array<Z_LootVolumeEntity> m_LootVolumeEntities = {};
	
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
		GetGame().GetCallqueue().CallLater(CollectLootContainers, m_LootContainerCollectionIntervalInSeconds * 1000, true);
	}
	
	int GetLootContainerCooldown()
	{
		return m_LootContainerCooldownInSeconds;
	}
	
	ref array<ref Z_LootTableDefinition> GetLootTableDefinitions()
	{
		Resource container = BaseContainerTools.LoadContainer(m_LootTableDefinitionsConfig);
					
		Z_LootTableDefinitionsConfig config = Z_LootTableDefinitionsConfig.Cast(
			BaseContainerTools.CreateInstanceFromContainer(container.GetResource().ToBaseContainer())
		);
		
		return config.m_LootTableDefinitions;
	}
	
	Z_LootTableDefinition GetLootTableDefinition(Z_LootCategory category)
	{
		foreach (ref Z_LootTableDefinition table : GetLootTableDefinitions())
		{
			if (table.m_Category == category) {
				return table;
			}
		}
		
		return null;
	}
	
	ref array<Z_LootVolumeEntity> GetLootVolumeEntities(bool withNonEmptyLootVolumes = true)
	{
		if (withNonEmptyLootVolumes) {
			ref array<Z_LootVolumeEntity> vols = {};
			
			foreach (Z_LootVolumeEntity v : m_LootVolumeEntities)
			{
				if (v.HasLootVolumes()) {
					vols.Insert(v);
				}
			}
			
			return vols;
		}
		
		return m_LootVolumeEntities;
	}
	
	void RegisterLootVolume(Z_LootVolumeEntity ent)
	{
		if (!ent)
			return;
		
		m_LootVolumeEntities.Insert(ent);
	}
	
	void HydrateLootContainer(IEntity ent)
	{
		Z_LootContainerComponent container = Z_LootContainerComponent.Cast(ent.FindComponent(Z_LootContainerComponent));
		
		GetGame().GetCallqueue().RemoveByName(container, "Dehydrate");
		
		GetGame().GetCallqueue().CallLaterByName(container, "Dehydrate", m_LootContainerExpiryInSeconds * 1000, false, ent);
		
		container.Hydrate(ent);
	}
	
	void CollectLootContainers()
	{
		array<int> players = {};
		GetGame().GetPlayerManager().GetPlayers(players);
		
		for (int i = 0, count = players.Count(); i < count; i++)
		{
			IEntity playerEnt = GetGame().GetPlayerManager().GetPlayerControlledEntity(players.Get(i));
			
			if (! playerEnt)
				continue;
			
			GetGame().GetWorld().QueryEntitiesBySphere(
				playerEnt.GetOrigin(),
				m_LootContainerQueryRadius,
				GetLootContainerEntity,
				FilterLootContainerEntities,
				EQueryEntitiesFlags.ALL
			);
		}
	}
	
	bool GetLootContainerEntity(IEntity ent)
	{
		if (ent.FindComponent(Z_LootContainerComponent))
			HydrateLootContainer(ent);
		
		if (ent.GetChildren())
		{
			IEntity child = ent.GetChildren();
						
			while (child)
			{
				if (child.FindComponent(Z_LootContainerComponent))
					HydrateLootContainer(child);
				
				child = child.GetSibling();
			}			
		}

		return true;
	}
	
	bool FilterLootContainerEntities(IEntity ent)
	{
		if (ent.FindComponent(Z_LootContainerComponent)) {
			return true;
		}
		
		if (ent.GetChildren())
		{
			IEntity child = ent.GetChildren();
						
			while (child)
			{
				if (child.FindComponent(Z_LootContainerComponent)) {
					return true;
				}
				
				child = child.GetSibling();
			}			
		}

		return false;
	}
}
