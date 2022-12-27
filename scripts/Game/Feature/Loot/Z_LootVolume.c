[BaseContainerProps(description: "Loot Volume")]
class Z_LootVolume: Managed
{
	[Attribute("0", UIWidgets.ComboBox, "Category", "", ParamEnumArray.FromEnum(Z_LootCategory))]
	Z_LootCategory m_Category;
	
	[Attribute("0", UIWidgets.ComboBox, "Rarity", "", ParamEnumArray.FromEnum(Z_LootRarity))]
	Z_LootRarity m_Rarity;
	
	// [Attribute("0.1", UIWidgets.Slider, "Probability", "0 1 0.01")]
	// float m_Probability;
	
	Z_LootCategory GetCategory()
	{
		return m_Category;
	}
	
	Z_LootRarity GetRarity()
	{
		return m_Rarity;
	}
	
	float GetProbability()
	{
		// TODO Replace with larger "LootRegion" entity that can
		// impose different probabilities over larger areas of the map.
		float value;
		
		switch (GetRarity()) {
			case Z_LootRarity.Common:
				value = Math.RandomFloat(0.33, 0.66);
				break;
			case Z_LootRarity.Uncommon:
				value = Math.RandomFloat(0.125, 0.33);
				break;
			case Z_LootRarity.Rare:
				value = Math.RandomFloat(0.033, 0.0875);
				break;
			case Z_LootRarity.Epic:
				value = Math.RandomFloat(0.01, 0.033);
				break;
			case Z_LootRarity.Impossible:
				value = 0;
				break;
		}
		
		return value;
	}
}
