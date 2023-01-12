enum Z_VehicleTier
{
	Tier1 = 0,
	Tier2 = 1,
	Tier3 = 2
}

[BaseContainerProps(configRoot: true)]
class Z_VehicleConfig
{
	[Attribute("", UIWidgets.Object, category: "Vehicles")]
	ref array<ref Z_Vehicle> m_Vehicles;
	
	ref array<ref Z_Vehicle> GetVehicles()
	{
		return m_Vehicles;
	}
}
