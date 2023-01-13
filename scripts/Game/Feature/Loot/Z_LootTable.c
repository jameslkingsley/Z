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

enum Z_LootTier
{
	Tier1 = 0,
	Tier2 = 1,
	Tier3 = 2,
	Tier4 = 3,
	Tier5 = 4
}

[BaseContainerProps(description: "Loot Table")]
class Z_LootTable
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail)]
	ResourceName m_Resource;
	
	[Attribute("", UIWidgets.ComboBox, "Tier rating of this item (higher = better)", "", ParamEnumArray.FromEnum(Z_LootTier))]
	Z_LootTier m_Tier;
	
	[Attribute("", UIWidgets.ComboBox, "Categories that this item represents", "", ParamEnumArray.FromEnum(Z_LootCategory))]
	ref array<ref Z_LootCategory> m_Categories;
	
	[Attribute("", UIWidgets.ComboBox, "Locations that this item can be spawned in", "", ParamEnumArray.FromEnum(Z_LootLocation))]
	ref array<ref Z_LootLocation> m_Locations;
	
	[Attribute("0 0 0", UIWidgets.Coords, "Transform offset the item should have when spawned in")]
	vector m_TransformOffset;
	
	[Attribute("0 0 0", UIWidgets.Coords, "Yaw pitch roll the item should have when spawned in")]
	vector m_YawPitchRoll;
	
	[Attribute("1", UIWidgets.CheckBox, "Enable or disable this loot item")]
	bool m_Enabled;
}
