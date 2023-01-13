[BaseContainerProps(configRoot: true)]
class Z_LootTableConfig
{
	[Attribute("", UIWidgets.Object, category: "Loot tables")]
	ref array<ref Z_LootTable> m_LootTables;
}

class Z_LootTableConfigInternal
{
	ref array<ref Z_LootTable> m_LootTables = new ref array<ref Z_LootTable>();
	
	void Merge(array<ref Z_LootTable> tables)
	{
		foreach (ref Z_LootTable tbl : tables)
		{
			if (! tbl.m_Enabled) continue;
			
			m_LootTables.Insert(tbl);
		}
	}
}
