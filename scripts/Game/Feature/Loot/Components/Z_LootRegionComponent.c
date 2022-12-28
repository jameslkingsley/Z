[BaseContainerProps(description: "Loot Table")]
class Z_LootRegionTier
{
	[Attribute("", UIWidgets.ComboBox, "Tier this probability applies to within this region", "", ParamEnumArray.FromEnum(Z_LootTier))]
	Z_LootTier m_LootTier;
	
	[Attribute("", UIWidgets.Auto, "Minimum probability to use for this tier within this region")]
	float m_ProbabilityMin;
	
	[Attribute("", UIWidgets.Auto, "Maximum probability to use for this tier within this region")]
	float m_ProbabilityMax;
	
	float m_Probability;
	
	float Roll()
	{
		m_Probability = Math.RandomFloat(m_ProbabilityMin, m_ProbabilityMax);
		
		return m_Probability;
	}
	
	float GetProbability()
	{
		if (m_Probability)
			return m_Probability;
		
		return Roll();
	}
}

[ComponentEditorProps(category: "Z/Components", description: "")]
class Z_LootRegionComponentClass: ScriptComponentClass
{
};

class Z_LootRegionComponent: ScriptComponent
{
	[Attribute("", UIWidgets.Object)]
	ref array<ref Z_LootRegionTier> m_LootRegionTiers;
	
	override event void OnPostInit(IEntity owner)
	{
		Z_LootGameModeComponent gameMode = Z_LootGameModeComponent.GetInstance();
		
		if (! gameMode)
			return;
		
		gameMode.RegisterLootRegion(this);
	}
	
	array<Z_LootRegionTier> GetRegionTiersWithConcreteProbabilities(array<ref Z_LootTier> tiers)
	{
		array<Z_LootRegionTier> result = {};
		
		foreach (Z_LootRegionTier tier : m_LootRegionTiers)
		{
			if (! tiers.IsEmpty() && ! tiers.Contains(tier.m_LootTier))
				continue;
			
			tier.Roll();
			
			result.Insert(tier);
		}
		
		return result;
	}
	
	Z_LootTier PickTier(array<ref Z_LootTier> tiers)
	{
		array<Z_LootRegionTier> regionTiers = GetRegionTiersWithConcreteProbabilities(tiers);
		
		SCR_Sorting<Z_LootRegionTier, Z_LootRegionTierCompare>.HeapSort(regionTiers);
		
		float random = Math.RandomFloat(0, 1);
		
		foreach (Z_LootRegionTier tier : regionTiers)
		{
			if (random <= tier.GetProbability())
				return tier;
		}
		
		return null;
	}
	
	PolylineArea GetPolylineArea()
	{
		return PolylineArea.Cast(GetOwner());
	}
}
