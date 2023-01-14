enum Z_VehicleTier
{
	Tier1 = 0,
	Tier2 = 1,
	Tier3 = 2
}

enum Z_VehicleContainerSize
{
	Small,
	Large
}

[BaseContainerProps(configRoot: true)]
class Z_VehicleConfig
{
	[Attribute("", UIWidgets.Object, category: "Vehicles")]
	ref array<ref Z_Vehicle> m_Vehicles;
	
	ref Z_Vehicle FindVehicle(ResourceName resource)
	{
		foreach (ref Z_Vehicle vehicle : m_Vehicles)
		{
			if (vehicle.GetResource() == resource)
			{
				return vehicle;
			}
		}
		
		return null;
	}
	
	ref array<ref Z_Vehicle> FilterByTier(Z_VehicleTier tier)
	{
		ref array<ref Z_Vehicle> filtered();
		
		foreach (ref Z_Vehicle vehicle : m_Vehicles)
		{
			if (vehicle.GetTier() == tier)
			{
				filtered.Insert(vehicle);
			}
		}
		
		return filtered;
	}
	
	ref array<ref Z_Vehicle> GetVehicles()
	{
		return m_Vehicles;
	}
}
