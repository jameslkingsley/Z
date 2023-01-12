[ComponentEditorProps(category: "Z/Components", description: "")]
class Z_VehicleRegionComponentClass : ScriptComponentClass
{}

class Z_VehicleRegionComponent : ScriptComponent
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Vehicles to spawn", "et")]
	ref array<ref ResourceName> m_Vehicles;
	
	ref array<string> m_SpawnedVehicles = new ref array<string>();
	
	ref array<IEntity> m_VehicleContainers = new ref array<IEntity>();
	
	protected override void OnPostInit(IEntity owner)
	{
		if (! GetGame().InPlayMode()) return;
		
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		if (! rplComponent)
		{
			Debug.Error("Vehicle region owner entity is missing RplComponent");
		}
		
		EL_PersistenceComponent persistence = EL_PersistenceComponent.Cast(owner.FindComponent(EL_PersistenceComponent));
		
		if (! persistence)
		{
			Debug.Error("Vehicle region must have a persistence component");
		}
		
		if (rplComponent.Role() == RplRole.Authority)
		{
			GetGame().GetCallqueue().CallLater(InitializeVehicles, 500);
		}
	}
	
	int GetMaximumVehicleCount()
	{
		if (! m_Vehicles) return 0;
		
		return m_Vehicles.Count();
	}
	
	void InitializeVehicles()
	{
		if (m_SpawnedVehicles.Count() == GetMaximumVehicleCount())
		{
			return;
		}
	}
	
	PolylineArea GetPolylineArea()
	{
		return PolylineArea.Cast(GetOwner());
	}
}
