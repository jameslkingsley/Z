[BaseContainerProps(description: "Vehicle")]
class Z_Vehicle
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Prefab to spawn", "et")]
	ResourceName m_Resource;
	
	[Attribute("0", UIWidgets.ComboBox, "Tier rating of this vehicle (higher = better)", "", ParamEnumArray.FromEnum(Z_VehicleTier))]
	Z_VehicleTier m_Tier;
	
	[Attribute("0 0 0", UIWidgets.Coords, "Transform offset the item should have when spawned in")]
	vector m_TransformOffset;
	
	[Attribute("0 0 0", UIWidgets.Coords, "Yaw pitch roll the item should have when spawned in")]
	vector m_YawPitchRoll;
	
	ResourceName GetResource()
	{
		return m_Resource;
	}
	
	Z_VehicleTier GetTier()
	{
		return m_Tier;
	}
	
	vector GetTransformOffset()
	{
		return m_TransformOffset;
	}
	
	vector GetYawPitchRoll()
	{
		return m_YawPitchRoll;
	}
	
	IEntity Spawn(vector origin)
	{
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = origin + GetTransformOffset();
		
		IEntity ent = GetGame().SpawnEntityPrefab(Resource.Load(GetResource()), null, params);
	
		ent.SetYawPitchRoll(GetYawPitchRoll());
		
		return ent;
	}
}
