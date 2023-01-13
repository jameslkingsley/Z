class Z_LootTableUtilities
{
	static ref Z_LootTable GetRandomTable(
		array<ref Z_LootTable> tables,
		Z_LootTier tier
	)
	{
		ref array<ref Z_LootTable> filtered = {};
		
		foreach (ref Z_LootTable table : tables)
		{
			if (table.m_Tier == tier)
				filtered.Insert(table);
		}
		
		if (filtered.IsEmpty())
			return null;
		
		return filtered.GetRandomElement();
	}
	
	static ref array<ref Z_LootTable> GetTablesHaving(
		array<ref Z_LootCategory> categories,
		array<ref Z_LootLocation> locations,
		array<ref Z_LootTier> tiers
	)
	{
		ref array<ref Z_LootTable> result = {};
		ref array<ref Z_LootTable> tables = Z_LootGameModeComponent.GetInstance().GetLootTables();
		
		foreach (ref Z_LootTable table : tables)
		{
			if (! tiers.Contains(table.m_Tier))
				continue;
			
			if (! IntArrayContainsSome(table.m_Categories, categories))
				continue;
			
			if (! IntArrayContainsSome(table.m_Locations, locations))
				continue;
			
			result.Insert(table);
		}
		
		return result;
	}
	
	static bool IntArrayContainsSome(array<ref int> haystack, array<ref int> needles)
	{
		foreach (int needle : needles)
		{
			if (haystack.Contains(needle))
				return true;
		}
		
		return false;
	}
	
	static array<int> EnumToArray(typename e)
	{
		array<int> result = {};

		for (int i = 0; i < e.GetVariableCount(); i++)
		{
			result.Insert(i);
		}
				
		return result;
	}
}
