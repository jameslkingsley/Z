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
	
	bool m_IsIgnored = false;
	
	bool m_IsSetup = false;
	
	int m_RefilledAtTimestampInSeconds;
	
	private bool m_DebugLogs = false;
	
	private void Log(string message, LogLevel level = LogLevel.NORMAL)
	{
		if (! m_DebugLogs) return;
		
		Print(message, level);
	}
	
	bool IsSetup()
	{
		return m_IsSetup;
	}
	
	void Setup()
	{
		if (m_IsSetup) return;
		
		if (m_Categories.IsEmpty() || m_Locations.IsEmpty())
		{
			Print("Loot volume has no configured categories or locations", LogLevel.ERROR);
			
			m_IsSetup = true;
			m_IsIgnored = true;
			
			return;
		}
		
		CacheLootContainers();
		
		m_IsSetup = true;
		
		Print("Loot volume setup");
	}
	
	void CacheLootContainers()
	{
		IEntity parent = GetParent();
		
		if (! parent)
		{
			Print("No parent for volume", LogLevel.ERROR);
			
			m_IsIgnored = true;
			
			return;
		}
		
		// TODO Replace this with a Workbench plugin that caches this
		
		vector mins, maxs;
		
		parent.GetWorldBounds(mins, maxs);
		
		GetGame().GetWorld().QueryEntitiesByAABB(mins, maxs, GetLootContainerEntity);
	}
	
	bool GetLootContainerEntity(IEntity ent)
	{
		if (ent.Type() == Z_LootContainerEntity)
		{
			Z_LootContainerEntity container = Z_LootContainerEntity.Cast(ent);
			
			if (! container) return true;
			
			if (! m_Containers.Contains(container))
			{
				m_Containers.Insert(container);
			}
		}

		return true;
	}

	bool IsInCooldown()
	{
		if (! m_RefilledAtTimestampInSeconds) return false;
		
		int timestampInSeconds = System.GetTickCount() / 1000;
		int elapsedTime = timestampInSeconds - m_RefilledAtTimestampInSeconds;
		int cooldown = Z_LootGameModeComponent.GetInstance().GetLootVolumeCooldown();
		
		return elapsedTime < cooldown;
	}
	
	bool IsIgnored()
	{
		return m_IsIgnored;
	}
	
	bool IsPointInsideBBox(float minX, float maxY, float maxX, float minY, float x, float y)
	{
		return (x >= minX && x <= maxX && y >= minY && y <= maxY);
	}
	
	bool IsEntityInsideBBox(vector mins, vector maxs, IEntity ent)
	{
		vector origin = ent.GetOrigin();
		
		return IsPointInsideBBox(mins[0], maxs[2], maxs[0], mins[2], origin[0], origin[2]);
	}
	
	bool HasPlayersInside()
	{
		array<int> players = {};
		GetGame().GetPlayerManager().GetPlayers(players);
		
		vector mins, maxs;
		GetParent().GetWorldBounds(mins, maxs);
		
		for (int i = 0, count = players.Count(); i < count; i++)
		{
			IEntity playerEnt = GetGame().GetPlayerManager().GetPlayerControlledEntity(players.Get(i));
			
			if (! playerEnt) continue;
			
			if (IsEntityInsideBBox(mins, maxs, playerEnt)) return true;
		}
		
		return false;
	}
	
	// Volume is considered sufficiently filled if its bounding box contains lootables
	// This item-hungry approach also means players dropping loot inside
	// buildings will prevent those buildings from spawning too much stuff.
	bool HasSufficientLoot(notnull out array<IEntity> lootables)
	{
		if (! m_Containers) return true;
		
		vector mins, maxs;
		
		GetParent().GetWorldBounds(mins, maxs);
		
		ref array<IEntity> entities = Z_LootGameModeComponent.GetInstance().GetLootableEntities();
		
		float insufficiency = Z_LootGameModeComponent.GetInstance().GetVolumeInsufficiencyPercentage();
		
		foreach (IEntity ent : entities)
		{
			if (! ent) continue;
			
			Managed component = ent.FindComponent(Z_LootableComponent);
			
			if (! component) continue;
			
			if (! IsEntityInsideBBox(mins, maxs, ent)) continue;
			
			lootables.Insert(ent);
		}
		
		int emptyContainers = m_Containers.Count() - lootables.Count();
		
		float percent = (float) emptyContainers / (float) m_Containers.Count();
		
		return percent < insufficiency;
	}
	
	void Refill(array<IEntity> lootables)
	{
		if (! IsSetup()) return;
		
		// If this volume doesn't have any containers
		// then just mark it as ignored and forget about it.
		// It will never gain containers mid-session.
		if (m_Containers.IsEmpty())
		{
			Log("Volume does not contain any containers", LogLevel.ERROR);
			
			m_IsIgnored = true;
			
			return;
		}
		
		HydrateParametersFromLootables(lootables);
		
		// TODO Cleanup volume of old container loot
		// ^ this is pretty confusing but maybe leaving loot
		// is actually a better / more realistic experience?
		// I suppose loot could get stuck - where no one wants it
		// and it never gets recycled into loot players want...
		// Cleanup would need a longer life (days) and only cleanup
		// if the item is not looted / interacted with in some way
		
		int hydratedCount = 0;
		
		Z_LootRegionComponent region = GetLootRegion();
		
		vector pos = GetOrigin();
		
		ref array<Z_LootContainerEntity> emptyContainers = GetEmptyContainers(lootables);
		
		int countOfContainersToUse = Math.RandomInt(
			(int) Math.Ceil(emptyContainers.Count() * region.GetMinimumContainerUsePercentage()),
			emptyContainers.Count()
		);
		
		array<Z_LootContainerEntity> touchedContainers = {};
		
		for (int i = 0; i < countOfContainersToUse; i++)
		{
			Z_LootContainerEntity container = emptyContainers.GetRandomElement();
			
			if (touchedContainers.Contains(container)) continue;
			
			array<ref Z_LootTier> acceptableTiers = GetAcceptableTiers(region);
			
			ref array<ref Z_LootTable> tables = Z_LootTableUtilities.GetTablesHaving(
				// TODO Add acceptable category limit as well
				m_Categories, m_Locations, acceptableTiers
			);
			
			if (tables.IsEmpty())
			{
				Log("No loot table matching parameters: " + m_Categories + " / " + m_Locations + " / " + acceptableTiers, LogLevel.SPAM);
				
				break;
			}
			
			Z_LootRegionTier tier = region.PickTier(acceptableTiers);
			
			if (! tier) continue;
			
			Z_LootTable table = Z_LootTableUtilities.GetRandomTable(tables, tier.m_LootTier);
			
			if (! table)
			{
				Log("Could not pick random table for tier", LogLevel.SPAM);
				
				continue;
			}
			
			IEntity lootable = container.SpawnLootable(table);
			
			if (! lootable) continue;
			
			Z_LootGameModeComponent.GetInstance().RegisterLootableEntity(lootable);
			
			IncrementSpawnedTiers(table.m_Tier);
			
			touchedContainers.Insert(container);
			
			hydratedCount++;
		}
		
		MarkAsRefilled();
		
		Log("Refilled loot volume: " + hydratedCount + " lootables");
	}
	
	void MarkAsRefilled()
	{
		m_RefilledAtTimestampInSeconds = System.GetTickCount() / 1000;
	}
	
	void HydrateParametersFromLootables(array<IEntity> lootables)
	{
		if (! lootables) return;
		
		m_SpawnedTiers.Clear();
		
		foreach (IEntity lootable : lootables)
		{
			if (! lootable) continue;
			
			Z_LootableComponent component = Z_LootableComponent.Cast(lootable.FindComponent(Z_LootableComponent));
			
			if (! component) continue;
			
			Z_LootTier tier = component.GetLootTier();
			
			if (tier >= 0)
			{
				IncrementSpawnedTiers(tier);
			}
		}
	}
	
	ref array<Z_LootContainerEntity> GetEmptyContainers(array<IEntity> lootables)
	{
		if (! lootables)
		{
			Print("No lootables (null), returning all containers", LogLevel.WARNING);
			
			return m_Containers;
		}
		
		if (lootables.IsEmpty())
		{
			Print("No lootables (empty), returning all containers", LogLevel.WARNING);
			
			return m_Containers;
		}
		
		ref array<Z_LootContainerEntity> empty = {};
		
		foreach (Z_LootContainerEntity container : m_Containers)
		{
			bool containerIsEmpty = true;
			
			foreach (IEntity lootable : lootables)
			{
				float distance = vector.Distance(container.GetOrigin(), lootable.GetOrigin());
				
				if (distance < 2)
				{
					containerIsEmpty = false;
					
					break;
				}
			}
			
			if (containerIsEmpty)
			{
				empty.Insert(container);
			}
		}
		
		return empty;
	}
	
	Z_LootRegionComponent GetLootRegion()
	{
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
	
	array<ref Z_LootTier> GetAcceptableTiers(Z_LootRegionComponent region)
	{
		array<ref Z_LootTier> result = {};
		
		array<int> tiers = Z_LootTableUtilities.EnumToArray(Z_LootTier);
		
		foreach (int tier : tiers)
		{
			if (HasReachedTierLimit(tier, region)) continue;
			
			result.Insert(tier);
		}
		
		return result;
	}
	
	bool HasReachedTierLimit(Z_LootTier tier, Z_LootRegionComponent region)
	{
		int tierSpawns = m_SpawnedTiers.Get(tier);
		
		if (! tierSpawns) tierSpawns = 0;
		
		int regionTierSpawns = region.GetTierSpawns(tier);

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
}
