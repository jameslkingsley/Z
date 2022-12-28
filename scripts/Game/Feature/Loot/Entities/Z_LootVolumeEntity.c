[EntityEditorProps(category: "Z/Entities", description: "")]
class Z_LootVolumeEntityClass: GenericEntityClass
{
};

class Z_LootVolumeEntity: GenericEntity
{
	[Attribute("", UIWidgets.ComboBox, "Categories that can spawn in this volume", "", ParamEnumArray.FromEnum(Z_LootCategory))]
	ref array<ref Z_LootCategory> m_Categories;
	
	[Attribute("", UIWidgets.ComboBox, "Locations that this volume represents", "", ParamEnumArray.FromEnum(Z_LootLocation))]
	ref array<ref Z_LootLocation> m_Locations;
	
	ref array<Z_LootContainerEntity> m_Containers = new ref array<Z_LootContainerEntity>();
	
	ref map<Z_LootTier, int> m_SpawnedTiers = new ref map<Z_LootTier, int>();
	
	bool m_IsHydrated = false;
	
	bool m_IsIgnored = false;
	
	bool m_DebugUseAllContainers = false;
	
	void Z_LootVolumeEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		
		if (! parent)
			return;
		
		parent.AddChild(this, -1, EAddChildFlags.RECALC_LOCAL_TRANSFORM);
	}
	
	override protected void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (! EL_PersistenceManager.IsPersistenceMaster()) return;
		
		if (m_Categories.IsEmpty() || m_Locations.IsEmpty()) {
			Print("Loot volume has no configured categories or locations", LogLevel.SPAM);
			
			m_IsIgnored = true;
			
			return;
		}
		
		IEntity parent = GetParent();
		
		if (! parent)
			return;
		
		SetVolumeOriginToParentCentroid(parent);
		
		CacheLootContainers(parent);
	}
	
	void Hydrate()
	{
		if (m_IsIgnored || m_IsHydrated)
			return;
		
		// If this volume doesn't have any containers
		// then just mark it as ignored and forget about it.
		// It will never gain containers mid-session.
		if (m_Containers.IsEmpty()) {
			Print("Volume does not contain any containers", LogLevel.ERROR);
			
			m_IsIgnored = true;
			
			return;
		}
		
		int hydratedCount = 0;
		
		ref array<Z_LootContainerEntity> containersForNewHydrations = {};
		
		foreach (Z_LootContainerEntity container : m_Containers)
		{
			if (container.HasSpawned())
			{
				continue;
			}
			
			if (! container.HasLootable())
			{
				containersForNewHydrations.Insert(container);
				
				continue;
			}
			
			container.SpawnOwnLootable(this);
			
			hydratedCount++;
		}
		
		Z_LootRegionComponent region = GetLootRegion();
		
		Print("Chose loot region: " + region.GetRegionSummary(), LogLevel.DEBUG);
		
		vector pos = GetOrigin();
		
		int containerCount = containersForNewHydrations.Count();
		
		int countOfContainersToUse = Math.RandomInt((int) Math.Ceil(containerCount * region.GetMinimumContainerUsePercentage()), containerCount);
		
		if (m_DebugUseAllContainers) {
			countOfContainersToUse = containersForNewHydrations.Count();
		}
		
		Print("Using " + countOfContainersToUse + " loot containers for volume", LogLevel.DEBUG);
		
		for (int i = 0; i < countOfContainersToUse; i++)
		{
			Z_LootContainerEntity container;
			
			if (m_DebugUseAllContainers) {
				container = containersForNewHydrations.Get(i);
			} else {
				container = containersForNewHydrations.GetRandomElement();
			}
			
			if (container.IsInCooldown() || container.HasSpawned())
				continue;
			
			array<ref Z_LootTier> acceptableTiers = GetAcceptableTiers(region);
			
			ref array<ref Z_LootTable> tables = Z_LootTableUtilities.GetTablesHaving(
				// TODO Possibly add acceptable category limit as well
				m_Categories, m_Locations, acceptableTiers
			);
			
			if (tables.IsEmpty()) {
				Print("No loot table matching parameters: " + m_Categories + " / " + m_Locations + " / " + acceptableTiers, LogLevel.WARNING);
				
				break;
			}
			
			Z_LootRegionTier tier = region.PickTier(acceptableTiers);
			
			if (! tier) {
				// Print("Could not pick tier (unlucky)");
				
				continue;
			}
			
			Z_LootTable table = Z_LootTableUtilities.GetRandomTable(tables, tier.m_LootTier);
			
			if (! table) {
				Print("Could not pick random table for tier", LogLevel.WARNING);
				
				continue;
			}
			
			container.SpawnLootTable(table, this);
			
			IncrementSpawnedTiers(table.m_Tier);
			
			hydratedCount++;
			
			// TODO Record and limit by resource name?
		}
		
		m_IsHydrated = true;
		
		Print("Hydrated loot volume: " + hydratedCount + " lootables", LogLevel.DEBUG);
	}
	
	Z_LootRegionComponent GetLootRegion()
	{
		// TODO Not working for some reason...
		Z_LootRegionComponent region = Z_LootGameModeComponent.GetInstance().GetLootRegionThatSurroundsEntity(this);
		
		if (! region)
			return DefaultLootRegion();
		
		return region;
	}
	
	Z_LootRegionComponent DefaultLootRegion()
	{
		return Z_LootRegionComponent.Cast(
			GetGame().GetGameMode().FindComponent(Z_LootRegionComponent)
		);
	}
	
	void SetVolumeOriginToParentCentroid(IEntity parent)
	{
		vector mins, maxs;
		
		parent.GetWorldBounds(mins, maxs);
		
		vector center = vector.Lerp(mins, maxs, 0.5);
		
		SetOrigin(center);
	}
	
	void CacheLootContainers(IEntity parent)
	{
		// This function sucks but for some reason furniture
		// isn't properly added as children of their building parents...
		// so you can't iterate down the tree.
		
		if (! parent || ! GetGame().GetWorld())
			return;
		
		vector mins, maxs;
		
		parent.GetWorldBounds(mins, maxs);
		
		float dist = vector.Distance(mins, maxs);
		
		GetGame().GetWorld().QueryEntitiesBySphere(
			GetOrigin(),
			dist,
			GetLootContainerEntity,
			FilterLootContainerEntities,
			EQueryEntitiesFlags.ALL
		);
	}
	
	bool GetLootContainerEntity(IEntity ent)
	{
		if (ent.Type() == Z_LootContainerEntity) {
			Z_LootContainerEntity container = Z_LootContainerEntity.Cast(ent);
			
			if (! m_Containers.Contains(container)) {
				m_Containers.Insert(container);
			}
		}

		return true;
	}
	
	bool FilterLootContainerEntities(IEntity ent)
	{
		if (ent.Type() == Z_LootContainerEntity) {
			return true;
		}

		return false;
	}
	
	array<ref Z_LootTier> GetAcceptableTiers(Z_LootRegionComponent region)
	{
		array<ref Z_LootTier> result = {};
		
		array<int> tiers = Z_LootTableUtilities.EnumToArray(Z_LootTier);
		
		foreach (int tier : tiers)
		{
			if (HasReachedTierLimit(tier, region)) {
				Print("Tier has reached limit in volume: " + tier, LogLevel.DEBUG);
				
				continue;
			}
			
			result.Insert(tier);
		}
		
		return result;
	}
	
	bool HasReachedTierLimit(Z_LootTier tier, Z_LootRegionComponent region)
	{
		int tierSpawns = m_SpawnedTiers.Get(tier);
		
		if (! tierSpawns)
			tierSpawns = 0;
		
		int regionTierSpawns = region.GetTierSpawns(tier);
		
		Print("Checking tier spawns: " + tierSpawns + " >= " + regionTierSpawns, LogLevel.DEBUG);

		return tierSpawns >= regionTierSpawns;
	}
	
	void IncrementSpawnedTiers(Z_LootTier tier)
	{
		m_SpawnedTiers.Set(tier, m_SpawnedTiers.Get(tier) + 1);
	}
	
	void DecrementSpawnedTiers(Z_LootTier tier)
	{
		m_SpawnedTiers.Set(tier, m_SpawnedTiers.Get(tier) - 1);
	}
	
	void Dehydrate()
	{
		if (m_Containers)
		{
			foreach (Z_LootContainerEntity container : m_Containers)
			{
				if (container.HasSpawned() && container.HasLootable())
				{
					bool lootableDestroyed = container.Clear();
				
					if (! lootableDestroyed)
					{
						if (container.m_Lootable)
							DecrementSpawnedTiers(container.m_Lootable.tier);
					}
				}
			}
		}
		
		m_IsHydrated = false;
		
		Print("Dehydrated loot volume", LogLevel.DEBUG);
	}
}
