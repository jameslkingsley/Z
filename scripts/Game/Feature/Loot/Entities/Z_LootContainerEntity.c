class Z_LootContainerLootable
{
	ResourceName resource;
	Z_LootTier tier;
	vector origin;
	vector yawPitchRoll;
	bool dropFromHeight = false;
	
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
		if (! parent)
			return;
		
		parent.AddChild(this, -1, EAddChildFlags.RECALC_LOCAL_TRANSFORM);
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
			
			Print("Loot container was looted - setting cooldown");
			
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
		lootable.origin = GetOrigin();
		lootable.yawPitchRoll = Vector(40, GetYawPitchRoll()[1], 90);
		lootable.dropFromHeight = true;
		
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
		
		Z_LootContainerLootable lootable = m_Lootable;
		
		vector origin = lootable.origin;
		
		if (lootable.dropFromHeight)
			origin += Vector(0, 0.25, 0);
		
		EntitySpawnParams spawnParams = EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[3] = origin;
		
		m_LootableEntity = GetGame().SpawnEntityPrefab(
			Resource.Load(lootable.resource),
			GetGame().GetWorld(),
			spawnParams
		);
		
		m_LootableEntity.SetYawPitchRoll(lootable.yawPitchRoll);
		
		if (! m_LootableEntity.GetPhysics()) {
			return;
		}
		
		if (lootable.dropFromHeight) {
			m_OriginalInteractionLayer = GetParent().GetPhysics().GetInteractionLayer();
			GetParent().GetPhysics().SetInteractionLayer(EPhysicsLayerDefs.Static);
			
			m_LootableEntity.GetPhysics().SetActive(ActiveState.ALWAYS_ACTIVE);
			m_LootableEntity.GetPhysics().ChangeSimulationState(SimulationState.SIMULATION);
			m_LootableEntity.GetPhysics().EnableGravity(true);
			m_LootableEntity.GetPhysics().SetInteractionLayer(EPhysicsLayerDefs.Static);
		}
		
		GetGame().GetCallqueue().CallLaterByName(this, "AfterEntityFinishedDrop", 3000, false, volume);
	}
	
	void AfterEntityFinishedDrop(notnull Z_LootVolumeEntity volume)
	{
		if (! m_LootableEntity)
			return;
		
		m_LootableEntity.GetPhysics().SetActive(ActiveState.INACTIVE);
		m_LootableEntity.GetPhysics().ChangeSimulationState(SimulationState.NONE);
		m_LootableEntity.GetPhysics().SetInteractionLayer(EPhysicsLayerDefs.None);
		
		if (m_OriginalInteractionLayer) {
			GetParent().GetPhysics().SetInteractionLayer(m_OriginalInteractionLayer);
		}
		
		m_Lootable.origin = m_LootableEntity.GetOrigin();
		m_Lootable.yawPitchRoll = m_LootableEntity.GetYawPitchRoll();
		m_Lootable.dropFromHeight = false;
	}
}
