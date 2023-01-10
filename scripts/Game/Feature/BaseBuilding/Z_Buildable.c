[BaseContainerProps(description: "Buildable")]
class Z_Buildable
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Prefab to spawn", "et")]
	ResourceName m_Resource;
	
	[Attribute("0 0 0", UIWidgets.Coords, "Transform offset the item should have when spawned in")]
	vector m_TransformOffset;
	
	[Attribute("0 0 0", UIWidgets.Coords, "Yaw pitch roll the item should have when spawned in")]
	vector m_YawPitchRoll;
	
	ResourceName GetResource()
	{
		return m_Resource;
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
