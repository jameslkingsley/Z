[EntityEditorProps(category: "Z/Entities", description: "")]
class Z_LootContainerEntityClass: GenericEntityClass
{}

class Z_LootContainerEntity: GenericEntity
{
	bool m_IsHydrated = false;
	
	IEntity m_Lootable;
	
	int m_OriginalInteractionLayer;
	
	ResourceName m_CurrentResource;
	vector m_CurrentOrigin;
	vector m_CurrentYawPitchRoll;
	Z_LootVolumeEntity m_CurrentLootVolumeEntity;
	int m_EmptiedAtTimestampInSeconds;
	
	void Z_LootContainerEntity(IEntitySource src, IEntity parent)
	{
		if (! parent)
			return;
		
		parent.AddChild(this, -1, EAddChildFlags.RECALC_LOCAL_TRANSFORM);
	}
	
	bool IsHydrated()
	{
		return m_IsHydrated;
	}
	
	void SetHydrated(bool state)
	{
		m_IsHydrated = state;
	}
	
	ResourceName GetCurrentResource()
	{
		return m_CurrentResource;
	}
	
	Z_LootVolumeEntity FindChildLootVolumeEntity(IEntity ent, bool withNonEmptyLootVolumes = true)
	{
		if (! ent.GetChildren())
			return null;

		IEntity child = ent.GetChildren();
					
		while (child)
		{
			if (child.Type() == Z_LootVolumeEntity) {
				Z_LootVolumeEntity vol = Z_LootVolumeEntity.Cast(child);
				
				if (withNonEmptyLootVolumes) {
					if (vol.HasLootVolumes()) return vol;
				} else {
					return vol;
				}
			}
			
			child = child.GetChildren();
		}
		
		return null;
	}
	
	Z_LootVolumeEntity FindParentLootVolumeEntity(bool withNonEmptyLootVolumes = true)
	{
		IEntity parent = GetOwner().GetParent();
						
		while (parent)
		{
			Z_LootVolumeEntity child = FindChildLootVolumeEntity(parent, withNonEmptyLootVolumes);
			
			if (child) {
				if (withNonEmptyLootVolumes) {
					if (child.HasLootVolumes()) return child;
				} else {
					return child;
				}
			}
			
			parent = parent.GetParent();
		}
		
		return null;
	}
	
	Z_LootVolumeEntity NearestLootVolumeEntity(bool withNonEmptyLootVolumes = true)
	{
		ref array<Z_LootVolumeEntity> volumes = Z_LootGameModeComponent.GetInstance().GetLootVolumeEntities(withNonEmptyLootVolumes);
		
		vector containerOrigin = GetOwner().GetOrigin();
		
		float nearestDist = 9999999;
		Z_LootVolumeEntity nearest;
		
		foreach(Z_LootVolumeEntity volume : volumes)
		{
			if (!volume)
				continue;
			
			float distance = vector.Distance(volume.GetOrigin(), containerOrigin);
			
			if (distance < nearestDist) {
				nearestDist = distance;
				nearest = volume;
			}
		}
		
		return nearest;
	}
	
	Z_LootVolumeEntity GetRelevantLootVolumeEntity(bool withNonEmptyLootVolumes = true)
	{
		Z_LootVolumeEntity ent = FindParentLootVolumeEntity(withNonEmptyLootVolumes);
		
		if (ent) return ent;
		
		return NearestLootVolumeEntity(withNonEmptyLootVolumes);
	}
	
	ResourceName PickLootable()
	{
		Z_LootVolumeEntity lootVolumeEntity = GetRelevantLootVolumeEntity();
		
		if (! lootVolumeEntity)
			return string.Empty;
		
		Z_LootVolumeEntity lootVolumeEntityForDuplicates = GetRelevantLootVolumeEntity(false);
		
		for (int i = 0; i < Math.RandomInt(1, 10); i++)
		{
			Z_LootVolume lootVolume = lootVolumeEntity.GetRandomLootVolume(GetCategories(), Math.RandomFloat(0, 1));
			
			if (! lootVolume)
				continue;
			
			ref Z_LootTableDefinition table = Z_LootGameModeComponent.GetInstance().GetLootTableDefinition(lootVolume.GetCategory());
			
			if (! table)
				continue;
			
			if (table.IsEmpty())
				continue;
				// return string.Empty;
		
			ResourceName randomLootable = table.GetRandomLootable();
			
			Print(randomLootable.GetPath());
			
			if (
				lootVolumeEntityForDuplicates.HasReachedLimitForLootable(randomLootable)
				|| lootVolumeEntity.HasReachedLimitForLootable(randomLootable)
			) {
				Print("Reached limit for " + randomLootable.GetPath());
				continue;
			}
			
			m_CurrentLootVolumeEntity = lootVolumeEntity;
			
			return randomLootable;
		}
		
		return string.Empty;
	}
	
	void SpawnLootable(IEntity ent)
	{
		ResourceName lootable;
		
		if (m_CurrentResource) {
			lootable = m_CurrentResource;
		} else {
			lootable = PickLootable();
		}
		
		if (! lootable) {
			return;
		}
		
		m_CurrentResource = lootable;
		
		if (m_Lootable) {
			delete m_Lootable;
		}
		
		vector origin;
		
		if (m_CurrentOrigin) {
			origin = m_CurrentOrigin;
		} else {
			origin = ent.GetOrigin() + Vector(0, 0.25, 0);
		}
		
		EntitySpawnParams spawnParams = EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[3] = origin;
		
		m_Lootable = GetGame().SpawnEntityPrefab(
			Resource.Load(lootable),
			GetGame().GetWorld(),
			spawnParams
		);
		
		vector yawPitchRoll;
		
		if (m_CurrentYawPitchRoll) {
			yawPitchRoll = m_CurrentYawPitchRoll;
		} else {
			yawPitchRoll = Vector(40, ent.GetYawPitchRoll()[1], 90);
		}
		
		m_Lootable.SetYawPitchRoll(yawPitchRoll);
		// m_Lootable.SetYawPitchRoll(Vector(Math.RandomInt(-180, 180), Math.RandomInt(-45, 45), Math.RandomInt(-180, 180)));
		
		m_CurrentLootVolumeEntity.AddSpawnedLootable(lootable);
		
		if (! m_Lootable.GetPhysics()) {
			return;
		}
		
		if (! m_CurrentOrigin && ! m_CurrentYawPitchRoll) {
			m_OriginalInteractionLayer = ent.GetParent().GetPhysics().GetInteractionLayer();
			ent.GetParent().GetPhysics().SetInteractionLayer(EPhysicsLayerDefs.Static);
			
			m_Lootable.GetPhysics().SetActive(ActiveState.ALWAYS_ACTIVE);
			m_Lootable.GetPhysics().ChangeSimulationState(SimulationState.SIMULATION);
			m_Lootable.GetPhysics().EnableGravity(true);
			m_Lootable.GetPhysics().SetInteractionLayer(EPhysicsLayerDefs.Static);
		}
		
		GetGame().GetCallqueue().CallLater(OnPlacement, 5000);
	}
	
	void Hydrate(IEntity ent)
	{
		if (IsHydrated())
			return;
		
		if (! ent.GetParent()) {
			return;
		}
		
		if (! ent.GetParent().GetPhysics()) {
			return;
		}
		
		int timestampInSeconds = System.GetTickCount() / 1000;
		int elapsedTime = timestampInSeconds - m_EmptiedAtTimestampInSeconds;
		int cooldown = Z_LootGameModeComponent.GetInstance().GetLootContainerCooldown();
		
		if (elapsedTime < cooldown)
			return;
		
		SpawnLootable(ent);
		
		SetHydrated(true);
	}
	
	void OnPlacement()
	{
		if (! m_Lootable)
			return;
		
		m_Lootable.GetPhysics().SetActive(ActiveState.INACTIVE);
		m_Lootable.GetPhysics().ChangeSimulationState(SimulationState.NONE);
		m_Lootable.GetPhysics().SetInteractionLayer(EPhysicsLayerDefs.None);
		
		if (m_OriginalInteractionLayer) {
			GetOwner().GetParent().GetPhysics().SetInteractionLayer(m_OriginalInteractionLayer);
		}
		
		m_CurrentOrigin = m_Lootable.GetOrigin();
		m_CurrentYawPitchRoll = m_Lootable.GetYawPitchRoll();
	}
	
	void Dehydrate(IEntity ent)
	{
		if (m_Lootable) {
			if (m_Lootable.GetOrigin() == m_CurrentOrigin) {
				delete m_Lootable;
			} else {
				if (m_CurrentLootVolumeEntity && m_CurrentResource) {
					m_CurrentLootVolumeEntity.RemoveSpawnedLootable(m_CurrentResource);
				}
				
				m_EmptiedAtTimestampInSeconds = System.GetTickCount() / 1000;
				m_CurrentResource = ResourceName.Empty;
				m_CurrentOrigin = Vector(0, 0, 0);
				m_CurrentYawPitchRoll = Vector(0, 0, 0);
				m_CurrentLootVolumeEntity = null;
				m_Lootable = null;
			}
		}
		
		SetHydrated(false);
	}
}
