[BaseContainerProps(description: "Vehicle Spawn Config")]
class Z_VehicleRegionSpawnConfig
{
	[Attribute("0", UIWidgets.ComboBox, "Tier of vehicle to spawn", "", ParamEnumArray.FromEnum(Z_VehicleTier))]
	Z_VehicleTier m_Tier;
	
	[Attribute("1", UIWidgets.Auto, "Number of spawns")]
	int m_Amount;
}

[ComponentEditorProps(category: "Z/Components", description: "")]
class Z_VehicleRegionComponentClass : ScriptComponentClass
{}

class Z_VehicleRegionComponent : ScriptComponent
{
	[Attribute("", UIWidgets.Object, category: "Vehicles")]
	ref array<ref Z_VehicleRegionSpawnConfig> m_VehicleSpawnConfigs;
	
	ref array<string> m_SpawnedVehicles = new ref array<string>();
	
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
			GetGame().GetCallqueue().CallLater(InitializeVehicles, 2500);
		}
	}
	
	int GetMaximumVehiclesInTier(Z_VehicleTier tier)
	{
		if (! m_VehicleSpawnConfigs) return 0;
		
		int count = 0;
		
		foreach (Z_VehicleRegionSpawnConfig conf : m_VehicleSpawnConfigs)
		{
			if (conf.m_Tier == tier)
			{
				count += conf.m_Amount;
			}
		}
		
		return count;
	}
	
	ref array<IEntity> GetSpawnedVehicleEntities()
	{
		ref array<IEntity> entities();
		
		EL_PersistenceManager manager = EL_PersistenceManager.GetInstance();
		
		if (! manager) return entities;
		
		array<string> idsToRemove();
		
		foreach (string persistentId : m_SpawnedVehicles)
		{
			IEntity ent = manager.FindEntityByPersistentId(persistentId);
			
			if (ent)
			{
				entities.Insert(ent);
			}
			else
			{
				idsToRemove.Insert(persistentId);
			}
		}
		
		foreach (string id : idsToRemove)
		{
			int index = m_SpawnedVehicles.Find(id);
			
			if (index > -1)
			{
				m_SpawnedVehicles.Remove(index);
			}
		}
		
		return entities;
	}
	
	Z_VehicleTier GetVehicleTier(IEntity ent)
	{
		ResourceName resource = ent.GetPrefabData().GetPrefabName();
		
		if (! resource) return -1;
		
		Z_Vehicle vehicle = Z_VehicleGameModeComponent.GetInstance().GetVehicleConfig().FindVehicle(resource);
		
		if (! vehicle) return -1;
		
		return vehicle.GetTier();
	}
	
	ref Z_Vehicle GetRandomVehicleInTier(Z_VehicleTier tier)
	{
		array<ref Z_Vehicle> vehicles = Z_VehicleGameModeComponent.GetInstance().GetVehicleConfig().FilterByTier(tier);
		
		if (vehicles.IsEmpty()) return null;
		
		return vehicles.GetRandomElement();
	}
	
	void InitializeVehicles()
	{
		ref map<Z_VehicleTier, int> tiersSpawned();
		
		tiersSpawned.Set(Z_VehicleTier.Tier1, 0);
		tiersSpawned.Set(Z_VehicleTier.Tier2, 0);
		tiersSpawned.Set(Z_VehicleTier.Tier3, 0);
		
		ref array<IEntity> spawned = GetSpawnedVehicleEntities();
		
		foreach (IEntity ent : spawned)
		{
			Z_VehicleTier tier = GetVehicleTier(ent);
			
			if (tier > -1)
			{
				tiersSpawned.Set(tier, tiersSpawned.Get(tier) + 1);
			}
		}
		
		foreach (Z_VehicleTier tier, int amountSpawned : tiersSpawned)
		{
			int amountToSpawn = GetMaximumVehiclesInTier(tier) - amountSpawned;
			
			if (amountToSpawn <= 0) continue;
			
			for (int i = 0; i < amountToSpawn; i++)
			{
				Z_Vehicle vehicle = GetRandomVehicleInTier(tier);
				
				if (! vehicle)
				{
					Print(string.Format("Cannot find vehicle in tier %1", tier), LogLevel.ERROR);
					
					break;
				}
				
				Z_VehicleContainerEntity container = GetRandomVehicleContainerBySize(vehicle.m_Size);
				
				if (! container)
				{
					Print(string.Format("Cannot find vehicle container that supports size %1", vehicle.m_Size), LogLevel.ERROR);
					
					continue;
				}
				
				IEntity vehicleEntity = container.SpawnVehicle(vehicle);
				
				EL_PersistenceComponent persistenceComponent = EL_PersistenceComponent.Cast(vehicleEntity.FindComponent(EL_PersistenceComponent));
				
				if (! persistenceComponent)
				{
					Print("Vehicle does not have persistence component", LogLevel.ERROR);
					
					RplComponent.DeleteRplEntity(vehicleEntity, false);
					
					continue;
				}
				
				persistenceComponent.Save();
				
				if (! m_SpawnedVehicles.Contains(persistenceComponent.GetPersistentId()))
				{
					m_SpawnedVehicles.Insert(persistenceComponent.GetPersistentId());
				}
			}
		}
		
		Print(string.Format("Initialized %1 vehicles for region: %2", m_SpawnedVehicles.Count(), GetOwner().GetName()));
	}
	
	Z_VehicleContainerEntity GetRandomVehicleContainerBySize(Z_VehicleContainerSize size)
	{
		array<Z_VehicleContainerEntity> filtered();
		
		foreach (Z_VehicleContainerEntity container : GetVehicleContainersInRegion())
		{
			if (container.GetSize() == size)
			{
				filtered.Insert(container);
			}
		}
		
		if (filtered.IsEmpty()) return null;
		
		return filtered.GetRandomElement();
	}
	
	ref array<Z_VehicleContainerEntity> GetVehicleContainersInRegion()
	{
		ref array<Z_VehicleContainerEntity> containers();
		
		PolylineArea area = GetPolylineArea();
		
		if (! area) return containers;
		
		foreach (Z_VehicleContainerEntity ent : Z_VehicleGameModeComponent.GetInstance().GetVehicleContainers())
		{
			if (area.IsEntityInside(ent) && ! ent.IsFilled())
			{
				containers.Insert(ent);
			}
		}
		
		return containers;
	}
	
	PolylineArea GetPolylineArea()
	{
		return PolylineArea.Cast(GetOwner());
	}
}
