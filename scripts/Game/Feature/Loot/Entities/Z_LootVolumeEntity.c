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
	
	bool m_IsHydrated = false;
	
	bool m_IsIgnored = false;
	
	ref map<Z_LootTier, int> m_SpawnedTiers = new ref map<Z_LootTier, int>();
	
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
		
		IEntity parent = GetParent();
		
		if (! parent)
			return;
		
		SetVolumeOriginToParentCentroid(parent);
		
		CacheLootContainers(parent);
	}
	
	void Hydrate()
	{
		// TODO Re-add cooldown
		if (m_IsIgnored || m_IsHydrated)
			return;
		
		if (m_Categories.IsEmpty() || m_Locations.IsEmpty()) {
			Print("Loot volume has no configured categories or locations");
			
			return;
		}
		
		// If this volume doesn't have any containers
		// then just mark it as ignored and forget about it.
		// It will never gain containers mid-session.
		if (m_Containers.IsEmpty()) {
			Print("Volume does not contain any containers");
			
			m_IsIgnored = true;
			
			return;
		}
		
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
			
			Print("Hydrated loot volume from previous state");
		}
		
		Z_LootRegionComponent region = GetLootRegion();
		
		vector pos = GetOrigin();
		
		int containerCount = containersForNewHydrations.Count();
		
		int countOfContainersToUse = Math.RandomInt((int) Math.Ceil(containerCount / 2), containerCount);
		
		for (int i = 0; i < countOfContainersToUse; i++)
		{
			Z_LootContainerEntity container = containersForNewHydrations.GetRandomElement();
			
			if (container.IsInCooldown())
				continue;
			
			array<ref Z_LootTier> acceptableTiers = GetAcceptableTiers();
			
			ref array<ref Z_LootTable> tables = Z_LootTableUtilities.GetTablesHaving(
				// TODO Possibly add acceptable category limit as well
				m_Categories, m_Locations, acceptableTiers
			);
			
			if (tables.IsEmpty()) {
				break;
			}
			
			Z_LootTier tier = region.PickTier(acceptableTiers);
			
			Z_LootTable table = Z_LootTableUtilities.GetRandomTable(tables, tier);
			
			if (! table)
				continue;
			
			container.SpawnLootTable(table, this);
			
			IncrementSpawnedTiers(table.m_Tier);
			
			// TODO Record and limit by resource name?
		}
		
		m_IsHydrated = true;
		
		Print("Hydrated loot volume with new state");
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
			dist / 1.5,
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
	
	array<ref Z_LootTier> GetAcceptableTiers()
	{
		array<ref Z_LootTier> result = {};
		
		array<int> tiers = Z_LootTableUtilities.EnumToArray(Z_LootTier);
		
		foreach (int tier : tiers)
		{
			if (HasReachedTierLimit(tier))
				continue;
			
			result.Insert(tier);
		}
		
		return result;
	}
	
	bool HasReachedTierLimit(Z_LootTier tier)
	{
		int tierSpawns = m_SpawnedTiers.Get(tier);
		
		if (! tierSpawns)
			tierSpawns = 0;
		
		// TODO Replace with proper region checking
		// Region defines lower/upper limit for maximum tier spawns per volume
		// Harder to reach areas are more rewarding
		// return false;
		return tierSpawns > Math.RandomInt(1, 10);
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
		
		Print("Dehydrated loot volume");
	}
}
