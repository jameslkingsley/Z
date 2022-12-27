class Z_LootVolumeCompare : SCR_SortCompare<Z_LootVolume>
{
	override static int Compare(Z_LootVolume left, Z_LootVolume right)
	{		
		if (left.GetProbability() < right.GetProbability())
			return 1;
		else
			return 0;
	}
}
