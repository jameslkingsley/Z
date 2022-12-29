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
	void Z_LootContainerEntity(IEntitySource src, IEntity parent)
	{
		if (! parent) return;
		
		parent.AddChild(this, -1, EAddChildFlags.RECALC_LOCAL_TRANSFORM);
	}
	
	notnull Z_LootContainerLootable CreateLootableFromTable(Z_LootTable table)
	{
		Z_LootContainerLootable lootable = new Z_LootContainerLootable();
		
		lootable.resource = table.m_Resource;
		lootable.tier = table.m_Tier;
		lootable.origin = GetOrigin() + Vector(0, Math.RandomFloat(0.5, 1), 0);
		lootable.yawPitchRoll = Vector(Math.RandomInt(-180, 180), Math.RandomInt(-90, 90), Math.RandomInt(-90, 90));
		lootable.usePhysics = true;
		
		return lootable;
	}
	
	void SpawnLootable(Z_LootContainerLootable lootable)
	{
		IEntity ent;
		
		EntitySpawnParams spawnParams = EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[3] = lootable.origin;
		
		ent = GetGame().SpawnEntityPrefab(
			Resource.Load(lootable.resource),
			GetGame().GetWorld(),
			spawnParams
		);
		
		ent.SetYawPitchRoll(lootable.yawPitchRoll);
		
		Z_LootableComponent lootableComponent = Z_LootableComponent.Cast(ent.FindComponent(Z_LootableComponent));
		
		if (! lootableComponent)
		{
			delete ent;
			
			Print("Lootable entity does not have lootable component: " + lootable.resource.GetPath(), LogLevel.ERROR);
			
			return;
		}
		
		if (lootable.usePhysics) {
			ActivatePhysics(ent);
		}
	}
	
	void ActivatePhysics(IEntity ent)
	{
		if (! ent.GetPhysics())
		{
			return;
		}
		
		if (! ent.GetPhysics().IsDynamic())
		{
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
		
		int originalInteractionLayer = GetParent().GetPhysics().GetInteractionLayer();
		GetParent().GetPhysics().SetInteractionLayer(EPhysicsLayerDefs.Dynamic);
		
		ent.GetPhysics().SetActive(ActiveState.ACTIVE);
		ent.GetPhysics().ChangeSimulationState(SimulationState.SIMULATION);
		ent.GetPhysics().EnableGravity(true);
		ent.GetPhysics().SetInteractionLayer(EPhysicsLayerDefs.Dynamic);
		
		GetGame().GetCallqueue().CallLaterByName(this, "RestPhysicsAfterPlacement", 5000, false, ent, originalInteractionLayer);
	}
	
	void RestPhysicsAfterPlacement(IEntity ent, int originalInteractionLayer)
	{
		if (! ent) return;
		
		ent.GetPhysics().ChangeSimulationState(SimulationState.COLLISION);
		ent.GetPhysics().SetInteractionLayer(EPhysicsLayerDefs.None);
		
		if (originalInteractionLayer) {
			GetParent().GetPhysics().SetInteractionLayer(originalInteractionLayer);
		}
		
		Z_LootableComponent lootableComponent = Z_LootableComponent.Cast(ent.FindComponent(Z_LootableComponent));
		
		if (! lootableComponent) return;
		
		lootableComponent.SetInitialSpawnState();
	}
}
