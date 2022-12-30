[BaseContainerProps(description: "Loot Table")]
class Z_LootRegionTier
{
	[Attribute("", UIWidgets.ComboBox, "Tier this probability applies to within this region", "", ParamEnumArray.FromEnum(Z_LootTier))]
	Z_LootTier m_LootTier;
	
	[Attribute("", UIWidgets.Auto, "Minimum probability to use for this tier within this region")]
	float m_ProbabilityMin;
	
	[Attribute("", UIWidgets.Auto, "Maximum probability to use for this tier within this region")]
	float m_ProbabilityMax;
	
	[Attribute("", UIWidgets.Auto, "Lowest number of spawns for this tier that will be permitted within a volume")]
	int m_SpawnRangeLower;
	
	[Attribute("", UIWidgets.Auto, "Highest number of spawns for this tier that will be permitted within a volume")]
	int m_SpawnRangeUpper;
	
	float m_Probability;
	
	float Roll()
	{
		if (m_ProbabilityMin == m_ProbabilityMax) {
			m_Probability = m_ProbabilityMin;
		} else {
			m_Probability = Math.RandomFloat(m_ProbabilityMin, m_ProbabilityMax);
		}
		
		return m_Probability;
	}
	
	float GetProbability()
	{
		if (m_Probability >= 0)
			return m_Probability;
		
		return Roll();
	}
	
	int GetSpawns()
	{
		if (m_SpawnRangeUpper <= 0) return 0;
		
		if (m_SpawnRangeLower < 0) m_SpawnRangeLower = 0;
		
		if (m_SpawnRangeLower >= m_SpawnRangeUpper) return m_SpawnRangeLower;
		
		return Math.RandomInt(m_SpawnRangeLower, m_SpawnRangeUpper);
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
	
	[Attribute("0.5", UIWidgets.Slider, "Minimum percentage of containers to use within a volume", "0 1 0.1")]
	float m_MinimumContainerUsePercentage;
	
	[Attribute("3", UIWidgets.Slider, "Number of dice rolls (higher = luckier)", "1 20 1")]
	int m_DiceRolls;
	
	protected RplComponent m_RplComponent;
	
	override event void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	protected override void EOnInit(IEntity owner)
	{
		m_RplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		
		if (! m_RplComponent) return;
		
		if (IsProxy()) return;
		
		Z_LootGameModeComponent gameMode = Z_LootGameModeComponent.GetInstance();
		
		if (! gameMode) return;
		
		gameMode.RegisterLootRegion(this);
	}
	
	bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	array<Z_LootRegionTier> GetRegionTiersWithConcreteProbabilities(array<ref Z_LootTier> tiers)
	{
		array<Z_LootRegionTier> result = {};
		
		if (! m_LootRegionTiers)
			return result;
		
		foreach (Z_LootRegionTier tier : m_LootRegionTiers)
		{
			if (! tiers.IsEmpty() && ! tiers.Contains(tier.m_LootTier))
				continue;
			
			tier.Roll();
			
			result.Insert(tier);
		}
		
		return result;
	}
	
	float GetMinimumContainerUsePercentage()
	{
		if (! m_MinimumContainerUsePercentage
			|| m_MinimumContainerUsePercentage > 1
			|| m_MinimumContainerUsePercentage < 0)
			return 0.5;
		
		return m_MinimumContainerUsePercentage;
	}
	
	int GetTierSpawns(Z_LootTier tier)
	{
		if (! m_LootRegionTiers)
			return 0;
		
		foreach (Z_LootRegionTier regionTier : m_LootRegionTiers)
		{
			if (tier != regionTier.m_LootTier)
				continue;
			
			return regionTier.GetSpawns();
		}
		
		return 0;
	}
	
	Z_LootRegionTier PickTier(array<ref Z_LootTier> tiers)
	{
		if (! m_LootRegionTiers)
			return null;
		
		array<Z_LootRegionTier> regionTiers = GetRegionTiersWithConcreteProbabilities(tiers);
		
		SCR_Sorting<Z_LootRegionTier, Z_LootRegionTierCompare>.HeapSort(regionTiers);
		
		for (int i = 0; i < m_DiceRolls; i++)
		{
			float random = Math.RandomFloat(0, 1);
		
			foreach (Z_LootRegionTier tier : regionTiers)
			{
				if (random <= tier.GetProbability())
					return tier;
			}
		}
		
		return null;
	}
	
	PolylineArea GetPolylineArea()
	{
		return PolylineArea.Cast(GetOwner());
	}
}
