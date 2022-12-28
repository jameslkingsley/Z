class Z_LootRegionTierCompare : SCR_SortCompare<Z_LootRegionTier>
{
	override static int Compare(Z_LootRegionTier left, Z_LootRegionTier right)
	{		
		if (left.m_ProbabilityMin < right.m_ProbabilityMin)
			return 1;
		else
			return 0;
	}
}
