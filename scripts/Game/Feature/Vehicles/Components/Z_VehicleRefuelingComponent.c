[ComponentEditorProps(category: "Z/Components", description: "")]
class Z_VehicleRefuelingComponentClass: ScriptComponentClass
{
};

class Z_VehicleRefuelingComponent: ScriptComponent
{
	IEntity m_NearestVehicle;
	
	void RequestRefuel(IEntity owner, IEntity user)
	{
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		if (!rplComponent)
		{
			Print("Owner of Z_VehicleRefuelingComponent does not have RplComponent", LogLevel.ERROR);
			
			return;
		}
		
		Rpc(RpcDo_RequestRefuel, rplComponent.Id());
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_RequestRefuel(RplId rplCompId)
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(rplCompId));
		if (!rplComp) return;
		
		IEntity owner = rplComp.GetEntity();
		
		IEntity vehicle = GetNearestVehicle(owner);
		
		if (! vehicle) return;
		
		Refuel(vehicle);
		Repair(vehicle);
	}
	
	void Repair(IEntity ent)
	{
		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(ent.FindComponent(SCR_VehicleDamageManagerComponent));
		
		if (! damageManager) return;
		
		damageManager.FullHeal();
	}
	
	void Refuel(IEntity ent)
	{
		FuelManagerComponent fuelManager = FuelManagerComponent.Cast(ent.FindComponent(FuelManagerComponent));
		
		if (! fuelManager) return;
		
		array<BaseFuelNode> fuelNodes();
		fuelManager.GetFuelNodesList(fuelNodes);
		
		foreach (BaseFuelNode node : fuelNodes)
		{
			if (node.GetFuel() == node.GetMaxFuel())
				continue;
			
			node.SetFuel(node.GetMaxFuel());
		}
	}
	
	IEntity GetNearestVehicle(IEntity owner)
	{
		GetGame().GetWorld().QueryEntitiesBySphere(owner.GetOrigin(), 7.5, CheckVehicleNearPump);
		
		return m_NearestVehicle;
	}
	
	bool CheckVehicleNearPump(IEntity ent)
	{
		Managed comp = ent.FindComponent(FuelManagerComponent);
		
		if (comp)
		{
			FuelManagerComponent fuelManager = FuelManagerComponent.Cast(comp);
			
			if (fuelManager.CanBeRefueled())
			{
				m_NearestVehicle = ent;
				
				return false;
			}
		}

		return true;
	}
}
