[ComponentEditorProps(category: "Z/Components", description: "")]
class Z_BaseBuildingGameModeComponentClass: SCR_BaseGameModeComponentClass
{}

class Z_BaseBuildingGameModeComponent: SCR_BaseGameModeComponent
{
	[Attribute("{3B270A7C30B41C62}Config/Z_BaseBuildingConfig.conf", UIWidgets.ResourceNamePicker, "Base building config")]
	ResourceName m_ConfigResource;
	
	ref Z_BaseBuildingConfig m_ConfigCache;
	
	static Z_BaseBuildingGameModeComponent GetInstance()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		
		if (gameMode)
			return Z_BaseBuildingGameModeComponent.Cast(gameMode.FindComponent(Z_BaseBuildingGameModeComponent));
		else
			return null;
	}
	
	ref Z_BaseBuildingConfig GetConfig()
	{
		if (m_ConfigCache)
		{
			return m_ConfigCache;
		}
		
		Resource container = BaseContainerTools.LoadContainer(m_ConfigResource);
					
		m_ConfigCache = Z_BaseBuildingConfig.Cast(
			BaseContainerTools.CreateInstanceFromContainer(container.GetResource().ToBaseContainer())
		);
		
		return m_ConfigCache;
	}
	
	ref Z_Buildable FindBuildable(ResourceName resource)
	{
		foreach (ref Z_Buildable buildable : GetConfig().GetBuildables())
		{
			if (buildable.GetResource() == resource)
			{
				return buildable;
			}
		}
		
		return null;
	}
	
	IEntity SpawnConstructionTool(vector origin)
	{
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = origin + GetConfig().GetConstructionToolOffset();
		
		IEntity ent = GetGame().SpawnEntityPrefab(Resource.Load(GetConfig().GetConstructionToolResource()), null, params);
		
		ent.SetYawPitchRoll(GetConfig().GetConstructionToolYawPitchRoll());
		
		return ent;
	}
}
