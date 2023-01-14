[ComponentEditorProps(category: "Z/Components", description: "")]
class Z_VehicleGameModeComponentClass: SCR_BaseGameModeComponentClass
{}

class Z_VehicleGameModeComponent: SCR_BaseGameModeComponent
{	
	[Attribute("{05C51EC369D5D562}Config/Z_VehicleConfig.conf", UIWidgets.ResourceNamePicker, "Vehicle config")]
	ResourceName m_VehicleConfig;
	
	ref Z_VehicleConfig m_VehicleConfigCache;
	
	ref array<Z_VehicleContainerEntity> m_VehicleContainerEntities = new ref array<Z_VehicleContainerEntity>();
	
	static Z_VehicleGameModeComponent GetInstance()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		
		if (gameMode)
			return Z_VehicleGameModeComponent.Cast(gameMode.FindComponent(Z_VehicleGameModeComponent));
		else
			return null;
	}
	
	ref Z_VehicleConfig GetVehicleConfig()
	{
		if (m_VehicleConfigCache)
		{
			return m_VehicleConfigCache;
		}
		
		Resource container = BaseContainerTools.LoadContainer(m_VehicleConfig);
					
		Z_VehicleConfig config = Z_VehicleConfig.Cast(
			BaseContainerTools.CreateInstanceFromContainer(container.GetResource().ToBaseContainer())
		);
					
		m_VehicleConfigCache = config;
		
		return m_VehicleConfigCache;
	}
	
	ref array<Z_VehicleContainerEntity> GetVehicleContainers()
	{
		return m_VehicleContainerEntities;
	}
	
	void RegisterVehicleContainer(Z_VehicleContainerEntity ent)
	{
		if (m_VehicleContainerEntities.Contains(ent))
			return;
		
		m_VehicleContainerEntities.Insert(ent);
	}
}
