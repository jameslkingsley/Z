[EntityEditorProps(category: "Z/Entities", description: "")]
class Z_VehicleContainerEntityClass: GenericEntityClass
{}

class Z_VehicleContainerEntity: GenericEntity
{
	[Attribute("0", UIWidgets.ComboBox, "Size of vehicle this container can fit", "", ParamEnumArray.FromEnum(Z_VehicleContainerSize))]
	Z_VehicleContainerSize m_Size;
	
	bool m_IsFilled = false;
	
	void Z_VehicleContainerEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
	
	override protected void EOnInit(IEntity owner)
	{
		if (! Replication.IsServer() || ! GetGame().InPlayMode()) return;
		
		if (! owner) return;
		
		if (! Z_VehicleGameModeComponent.GetInstance()) return;
		
		GetGame().GetCallqueue().CallLater(RegisterVehicleContainerIfEmpty, 100);
	}
	
	bool IsFilled()
	{
		return m_IsFilled;
	}
	
	void RegisterVehicleContainerIfEmpty()
	{
		GetGame().GetWorld().QueryEntitiesBySphere(GetOrigin(), 5, CheckEntityNearVehicleContainer);
		
		if (! m_IsFilled)
		{
			Z_VehicleGameModeComponent.GetInstance().RegisterVehicleContainer(this);
		}
	}
	
	bool CheckEntityNearVehicleContainer(IEntity ent)
	{
		if (ent.FindComponent(FuelManagerComponent))
		{
			m_IsFilled = true;
			
			return false;
		}

		return true;
	}
	
	IEntity SpawnVehicle(Z_Vehicle vehicle)
	{
		IEntity ent;
		
		EntitySpawnParams spawnParams = EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[3] = GetOrigin();
		
		ent = GetGame().SpawnEntityPrefab(
			Resource.Load(vehicle.GetResource()),
			GetGame().GetWorld(),
			spawnParams
		);
		
		ent.SetYawPitchRoll(GetYawPitchRoll());
		
		SetFuel(ent);
		SetDamage(ent);
		
		m_IsFilled = true;
		
		return ent;
	}
	
	void SetDamage(IEntity ent)
	{
		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(ent.FindComponent(SCR_VehicleDamageManagerComponent));
		
		if (! damageManager) return;
		
		array<HitZone> hitzones();
		damageManager.GetAllHitZones(hitzones);
		
		foreach (HitZone hitZone : hitzones)
		{
			SCR_WheelHitZone wheel = SCR_WheelHitZone.Cast(hitZone);
			
			if (! wheel) continue;
			
			if (Math.RandomFloat01() > 0.5) continue;
			
			wheel.SetHealth(Math.RandomFloat(0, 0.5));
		}
	}
	
	void SetFuel(IEntity ent)
	{
		FuelManagerComponent fuelManager = FuelManagerComponent.Cast(ent.FindComponent(FuelManagerComponent));
		
		if (! fuelManager) return;
		
		array<BaseFuelNode> fuelNodes();
		fuelManager.GetFuelNodesList(fuelNodes);
		
		foreach (BaseFuelNode node : fuelNodes)
		{
			node.SetFuel(Math.RandomFloat(0.05, 0.5));
		}
	}
	
	Z_VehicleContainerSize GetSize()
	{
		return m_Size;
	}
}
