[EntityEditorProps(category: "Z/Entities", description: "")]
class Z_LootVolumeEntityClass: GenericEntityClass
{
};

class Z_LootVolumeEntity: GenericEntity
{
	[Attribute("-1", UIWidgets.Auto, "Maximum number of duplicate lootables within this volume")]
	int m_MaximumDuplicates;
	
	[Attribute("", UIWidgets.Object, category: "Loot volumes")]
	ref array<ref Z_LootVolume> m_LootVolumes;
	
	ref map<ResourceName, int> m_SpawnedLootables;
	
	void Z_LootVolumeEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		
		if (! parent)
			return;
		
		parent.AddChild(this, -1, EAddChildFlags.RECALC_LOCAL_TRANSFORM);
	}
	
	override event void EOnInit(IEntity owner)
	{
		Z_LootGameModeComponent comp = Z_LootGameModeComponent.GetInstance();
		
		if (! comp)
			return;
		
		comp.RegisterLootVolume(this);
	}
	
	bool HasLootVolumes()
	{
		return ! m_LootVolumes.IsEmpty();
	}
	
	void AddSpawnedLootable(ResourceName lootable)
	{
		if (! m_SpawnedLootables) {
			m_SpawnedLootables = new map<ResourceName, int>();
		}
		
		if (m_SpawnedLootables.Contains(lootable)) {
			int count = m_SpawnedLootables.Get(lootable);
			m_SpawnedLootables.Set(lootable, count + 1);
			return;
		}
		
		m_SpawnedLootables.Insert(lootable, 1);
	}
	
	void RemoveSpawnedLootable(ResourceName lootable, int count = 1)
	{
		if (! m_SpawnedLootables)
			return;
		
		if (! m_SpawnedLootables.Contains(lootable)) {
			return;
		}
		
		int c = m_SpawnedLootables.Get(lootable) - count;
		
		if (c <= 0) {
			m_SpawnedLootables.Remove(lootable);
		} else {
			m_SpawnedLootables.Set(lootable, c);
		}
	}
	
	int GetMaximumSpawnCountOfLootable(ResourceName lootable)
	{
		if (m_MaximumDuplicates) {
			return m_MaximumDuplicates;
		}
		
		return -1;
	}
	
	int GetSpawnCountOfLootable(ResourceName lootable)
	{
		if (! m_SpawnedLootables)
			return 0;
		
		if (m_SpawnedLootables.Contains(lootable)) {
			return m_SpawnedLootables.Get(lootable);
		}
		
		return 0;
	}
	
	bool HasReachedLimitForLootable(ResourceName lootable)
	{
		int max = GetMaximumSpawnCountOfLootable(lootable);
		
		if (max == -1)
			return false;
		
		return GetSpawnCountOfLootable(lootable) >= max;
	}
	
	array<Z_LootVolume> GetLootVolumesOrderedByProbability(array<ref Z_LootCategory> categories)
	{
		array<Z_LootVolume> volumes = GetLootVolumesMatchingCategories(categories);
		
		SCR_Sorting<Z_LootVolume, Z_LootVolumeCompare>.HeapSort(volumes);
		
		return volumes;
	}
	
	array<Z_LootVolume> GetLootVolumesMatchingCategories(array<ref Z_LootCategory> categories)
	{
		array<Z_LootVolume> filtered = {};
		
		foreach (ref Z_LootVolume volume : m_LootVolumes)
		{
			if (! categories.Contains(volume.GetCategory())) {
				continue;
			}
			
			filtered.Insert(volume);
		}
		
		return filtered;
	}
	
	Z_LootVolume GetRandomLootVolume(array<ref Z_LootCategory> categories, float probability)
	{
		array<Z_LootVolume> volumes = GetLootVolumesOrderedByProbability(categories);
		
		if (! volumes) {
			return null;
		}
		
		Z_LootVolume chosenVolume;
		
		foreach (Z_LootVolume volume : volumes)
		{
			if (probability <= volume.GetProbability()) {
				chosenVolume = volume;
				
				break;
			}
		}
		
		return chosenVolume;
	}
}
