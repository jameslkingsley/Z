enum Z_LootCategory
{
	Clothing,
	Containers,
	Explosives,
	Food,
	Tools,
	Medical,
	Weapons,
	Ammunition,
	Attachments
}

enum Z_LootLocation
{
	Coast,
	Farm,
	Hunting,
	Industrial,
	Medical,
	Military,
	Office,
	Police,
	Prison,
	School,
	Town,
	Village
}

enum Z_LootRarity
{
	Common,
	Uncommon,
	Rare,
	Epic,
	Impossible
}

[BaseContainerProps(description: "Loot Table Definition")]
class Z_LootTableDefinition
{
	[Attribute("0", UIWidgets.ComboBox, "Category", "", ParamEnumArray.FromEnum(Z_LootCategory))]
	Z_LootCategory m_Category;
	
	[Attribute("", UIWidgets.ResourceAssignArray, "Lootables", "et")]
	ref array<ResourceName> m_Lootables;
	
	bool IsEmpty()
	{
		return m_Lootables.IsEmpty();
	}
	
	ResourceName GetRandomLootable()
	{
		return m_Lootables.GetRandomElement();
	}
}
