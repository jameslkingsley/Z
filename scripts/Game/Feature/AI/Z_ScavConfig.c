enum Z_ScavFaction
{
	Scavs,
	Raiders
}

[BaseContainerProps(configRoot: true)]
class Z_ScavConfig
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Scav soldier prefabs to use (low tier)", "et")]
	ref array<ref ResourceName> m_ScavPrefabs;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Raider soldier prefabs to use (high tier)", "et")]
	ref array<ref ResourceName> m_RaiderPrefabs;
	
	string GetFactionKey()
	{
		return "USSR";
	}
	
	ref array<ref ResourceName> GetSoldierPrefabs(Z_ScavFaction faction)
	{
		if (faction == Z_ScavFaction.Scavs) return m_ScavPrefabs;
		if (faction == Z_ScavFaction.Raiders) return m_RaiderPrefabs;
		return null;
	}
}
