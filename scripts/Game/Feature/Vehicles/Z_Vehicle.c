[BaseContainerProps(description: "Vehicle")]
class Z_Vehicle
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Prefab to spawn", "et")]
	ResourceName m_Resource;
	
	[Attribute("0", UIWidgets.ComboBox, "Tier rating of this vehicle (higher = better)", "", ParamEnumArray.FromEnum(Z_VehicleTier))]
	Z_VehicleTier m_Tier;
	
	[Attribute("0", UIWidgets.ComboBox, "Size of this vehicle", "", ParamEnumArray.FromEnum(Z_VehicleContainerSize))]
	Z_VehicleContainerSize m_Size;
	
	ResourceName GetResource()
	{
		return m_Resource;
	}
	
	Z_VehicleTier GetTier()
	{
		return m_Tier;
	}
	
	Z_VehicleContainerSize GetSize()
	{
		return m_Size;
	}
}
