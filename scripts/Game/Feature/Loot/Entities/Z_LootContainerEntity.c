class Z_LootContainerLootable
{
	ResourceName resource;
	Z_LootTier tier;
	vector origin;
	vector yawPitchRoll;
	bool usePhysics = false;
	
	// TODO Damage states (magazine filled)
}

[EntityEditorProps(category: "Z/Entities", description: "")]
class Z_LootContainerEntityClass: GenericEntityClass
{}

class Z_LootContainerEntity: GenericEntity
{
	IEntity m_LootableEntity;
	
	ref Z_LootContainerLootable m_Lootable;
	
	int m_OriginalInteractionLayer;
	
	int m_LootedAtTimestampInSeconds;
	
	void Z_LootContainerEntity(IEntitySource src, IEntity parent)
	{
		// SetEventMask(EntityEvent.INIT);
		
		if (! parent)
			return;
		
		parent.AddChild(this, -1, EAddChildFlags.RECALC_LOCAL_TRANSFORM);
	}
	
	override event protected void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		IEntity child = GetChildren();
		
		if (! child)
			return;
		
		Print("Hydrating container from child entity");
		m_LootableEntity = child;
	}
	
	bool HasSpawned()
	{
		return m_LootableEntity != null;
	}
	
	bool HasLootable()
	{
		return m_Lootable != null;
	}
	
	bool IsInCooldown()
	{
		if (! m_LootedAtTimestampInSeconds)
			return false;
		
		int timestampInSeconds = System.GetTickCount() / 1000;
		int elapsedTime = timestampInSeconds - m_LootedAtTimestampInSeconds;
		int cooldown = Z_LootGameModeComponent.GetInstance().GetLootVolumeCooldown();
		
		return elapsedTime < cooldown;
	}
	
	bool Clear()
	{
		if (! m_LootableEntity)
			return false;
		
		if (m_LootableEntity.GetOrigin() != m_Lootable.origin) {
			m_LootedAtTimestampInSeconds = System.GetTickCount() / 1000;
			
			// EnablePersistenceComponent(m_LootableEntity);
			// RemoveChild(m_LootableEntity);
			
			Print("Loot container was looted - setting cooldown", LogLevel.DEBUG);
			
			return false;
		}
		
		delete m_LootableEntity;
		
		return true;
	}
	
	void SpawnLootTable(Z_LootTable table, Z_LootVolumeEntity volume)
	{
		Z_LootContainerLootable lootable = new Z_LootContainerLootable();
		
		lootable.resource = table.m_Resource;
		lootable.tier = table.m_Tier;
		lootable.origin = GetOrigin() + Vector(0, Math.RandomFloat(0.5, 1), 0);
		lootable.yawPitchRoll = Vector(Math.RandomInt(-180, 180), Math.RandomInt(-90, 90), Math.RandomInt(-90, 90));
		lootable.usePhysics = true;
		
		m_Lootable = lootable;
		
		SpawnOwnLootable(volume);
	}
	
	void SpawnOwnLootable(Z_LootVolumeEntity volume)
	{
		if (m_LootableEntity) {
			delete m_LootableEntity;
		}
		
		if (! m_Lootable)
			return;
		
		EntitySpawnParams spawnParams = EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[3] = m_Lootable.origin;
		
		m_LootableEntity = GetGame().SpawnEntityPrefab(
			Resource.Load(m_Lootable.resource),
			GetGame().GetWorld(),
			spawnParams
		);
		
		m_LootableEntity.SetYawPitchRoll(m_Lootable.yawPitchRoll);
		
		m_LootableEntity.Update();
		
		// AddChild(m_LootableEntity, -1, EAddChildFlags.RECALC_LOCAL_TRANSFORM);
		
		// DisablePersistenceComponent(m_LootableEntity);
		
		if (m_Lootable.usePhysics) {
			ActivatePhysics(m_LootableEntity);
		}
		
		GetGame().GetCallqueue().CallLaterByName(this, "AfterEntityFinishedDrop", 5000, false, volume);
	}
	
	void DisablePersistenceComponent(IEntity ent)
	{
		EL_PersistenceComponent persistence = EL_PersistenceComponent.Cast(ent.FindComponent(EL_PersistenceComponent));
		
		if (! persistence)
			return;
		
		EL_PersistenceManagerInternal persistenceManager = EL_PersistenceManagerInternal.GetInternalInstance();

		persistenceManager.UnregisterSaveRoot(persistence);
	}
	
	void EnablePersistenceComponent(IEntity ent)
	{
		EL_PersistenceComponent persistence = EL_PersistenceComponent.Cast(ent.FindComponent(EL_PersistenceComponent));
		
		if (! persistence)
			return;
		
		EL_PersistenceManagerInternal persistenceManager = EL_PersistenceManagerInternal.GetInternalInstance();

		persistenceManager.RegisterSaveRoot(persistence, true);
	}
	
	void ActivatePhysics(IEntity ent)
	{
		if (! ent.GetPhysics())
		{
			// Print("Lootable is missing physics: " + m_Lootable.resource.GetPath(), LogLevel.WARNING);
			
			return;
		}
		
		if (! ent.GetPhysics().IsDynamic())
		{
			// Print("Lootable physics is not dynamic: " + m_Lootable.resource.GetPath(), LogLevel.WARNING);
			
			float massInKilos = ent.GetPhysics().GetMass();
			
			if (massInKilos <= 0) massInKilos = 10;
			
			ent.GetPhysics().Destroy();
			
			Physics.CreateDynamic(ent, massInKilos, -1);
			
			if (! ent.GetPhysics())
			{
				Print("Failed to create dynamic physics for lootable", LogLevel.ERROR);
				
				return;
			}
		}
		
		m_OriginalInteractionLayer = GetParent().GetPhysics().GetInteractionLayer();
		GetParent().GetPhysics().SetInteractionLayer(EPhysicsLayerDefs.Dynamic);
		
		ent.GetPhysics().SetActive(ActiveState.ACTIVE);
		ent.GetPhysics().ChangeSimulationState(SimulationState.SIMULATION);
		ent.GetPhysics().EnableGravity(true);
		ent.GetPhysics().SetInteractionLayer(EPhysicsLayerDefs.Dynamic);
	}
	
	void AfterEntityFinishedDrop(notnull Z_LootVolumeEntity volume)
	{
		if (! m_LootableEntity)
			return;
		
		m_LootableEntity.GetPhysics().ChangeSimulationState(SimulationState.COLLISION);
		m_LootableEntity.GetPhysics().SetInteractionLayer(EPhysicsLayerDefs.None);
		
		if (m_OriginalInteractionLayer) {
			GetParent().GetPhysics().SetInteractionLayer(m_OriginalInteractionLayer);
		}
		
		m_Lootable.origin = m_LootableEntity.GetOrigin();
		m_Lootable.yawPitchRoll = m_LootableEntity.GetYawPitchRoll();
		m_Lootable.usePhysics = false;
	}
}
