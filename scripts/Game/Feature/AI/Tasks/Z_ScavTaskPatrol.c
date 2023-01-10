[Z_ScavTask(Z_ScavTaskPatrol, "Patrol"), Z_ScavTaskTitle(), BaseContainerProps()]
class Z_ScavTaskPatrol : Z_ScavTaskBase
{
	[Attribute("{000CD338713F2B5A}Prefabs/AI/Groups/Group_Base.et", UIWidgets.ResourceNamePicker, "Group prefab to use (should be base)")]
	ResourceName m_GroupPrefab;
	
	[Attribute("{22A875E30470BD4F}Prefabs/AI/Waypoints/AIWaypoint_Patrol.et", UIWidgets.ResourceNamePicker, "Waypoint prefab to use")]
	ResourceName m_WaypointPrefab;
	
	[Attribute("2", UIWidgets.Auto, "Minimum number of soldiers in the group")]
	int m_GroupSizeMin;
	
	[Attribute("8", UIWidgets.Auto, "Maximum number of soldiers in the group")]
	int m_GroupSizeMax;
	
	override ref map<IEntity, ref Z_ScavTaskEntityStub> SpawnEntityStubs(vector origin, array<ref Z_ScavTaskEntityStub> stubs)
	{
		ref map<IEntity, ref Z_ScavTaskEntityStub> watchers();
		
		// Create AI group
		EntitySpawnParams grpParams = EntitySpawnParams();
		grpParams.TransformMode = ETransformMode.WORLD;
		grpParams.Transform[3] = origin;
		
		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(Resource.Load(m_GroupPrefab), GetGame().GetWorld(), grpParams));
		
		FactionManager facManager = GetGame().GetFactionManager();
		
		Faction faction = facManager.GetFactionByKey(GetFactionKey());
		
		if (! faction)
		{
			Print("Failed to find faction with key:" + GetFactionKey(), LogLevel.ERROR);
			
			return watchers;
		}
		
		group.SetFaction(faction);
		
		foreach (ref Z_ScavTaskEntityStub stub : stubs)
		{
			// Spawn soldier and add to group
			EntitySpawnParams params = EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			params.Transform[3] = stub.origin;
			
			int index = group.GetAgentsCount();
			
			IEntity ai = GetGame().SpawnEntityPrefab(Resource.Load(stub.resource), GetGame().GetWorld(), params);
			
			if (!ai) continue;
			
			SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(ai);
			
			if (char)
			{
				CharacterControllerComponent charController = char.GetCharacterController();
				
				if (charController)
				{
					if (stub.direction) charController.SetMovementDirWorld(stub.direction);
					if (stub.stance) charController.SetStanceChange(ConvertStanceToInt(stub.stance));
				}
			}
			
			group.AddAIEntityToGroup(ai, index);
			
			watchers.Set(ai, stub);
		}
		
		AddWaypoints(group, origin);
		
		return watchers;
	}
	
	void AddWaypoints(SCR_AIGroup group, vector origin)
	{
		array<AIWaypoint> queueOfWaypoints = new array<AIWaypoint>();
		
		for (int i = 0; i < Math.RandomInt(3, 6); i++)
		{
			vector wpPos = GetWaypointOrigin(origin);
			
			if (wpPos == vector.Zero) continue;
			
			EntitySpawnParams wpParams = EntitySpawnParams();
			wpParams.TransformMode = ETransformMode.WORLD;
			wpParams.Transform[3] = wpPos;
			
			AIWaypoint wp = AIWaypoint.Cast(GetGame().SpawnEntityPrefab(Resource.Load(m_WaypointPrefab), null, wpParams));
			
			if (wp) queueOfWaypoints.Insert(wp);
		}
		
		queueOfWaypoints.Insert(queueOfWaypoints[0]);
		
		EntitySpawnParams wpParams = new EntitySpawnParams();
		wpParams.TransformMode = ETransformMode.WORLD;
		wpParams.Transform[3] = origin;
	
		Resource res = Resource.Load("{35BD6541CBB8AC08}Prefabs/AI/Waypoints/AIWaypoint_Cycle.et");
		
		AIWaypointCycle wp = AIWaypointCycle.Cast(GetGame().SpawnEntityPrefab(res, null, wpParams));
		
		wp.SetWaypoints(queueOfWaypoints);
		wp.SetRerunCounter(-1);
		
		group.AddWaypointAt(wp, 0);
	}
	
	override ref array<ref Z_ScavTaskEntityStub> UpdateEntityStubs(map<IEntity, ref Z_ScavTaskEntityStub> watchers)
	{
		ref array<ref Z_ScavTaskEntityStub> stubs();
		
		foreach (IEntity ent, ref Z_ScavTaskEntityStub stub : watchers)
		{
			if (! ent) continue;
			
			SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(ent);
			
			if (! char) continue;
			
			CharacterControllerComponent charController = char.GetCharacterController();
			
			if (! charController) continue;
			
			if (charController.IsDead()) continue;
			
			ref Z_ScavTaskEntityStub newStub();
			
			newStub.resource = stub.resource;
			newStub.origin = ent.GetOrigin();
			newStub.direction = charController.GetMovementDirWorld();
			newStub.stance = charController.GetStance();
			
			stubs.Insert(newStub);
		}
		
		return stubs;
	}
	
	override ref array<ref Z_ScavTaskEntityStub> GetEntityStubs(Z_PersistentScavTask task)
	{
		ref array<ref ResourceName> prefabs = Z_ScavGameModeComponent.GetInstance().GetConfig().GetSoldierPrefabs(m_Faction);
		
		if (prefabs.IsEmpty())
		{
			Debug.Error("Soldier prefabs in scav config is empty");
		}
		
		ref array<ref Z_ScavTaskEntityStub> stubs();
		
		int groupSize = Math.RandomInt(m_GroupSizeMin, m_GroupSizeMax);
		
		for (int i = 0; i < groupSize; i++)
		{
			ref Z_ScavTaskEntityStub stub();
		
			stub.resource = prefabs.GetRandomElement();
			stub.origin = task.m_Origin + Vector(i, 0, 0);
			stub.direction = Vector(0, 0, 0);
			stub.stance = ECharacterStance.STAND;
		
			stubs.Insert(stub);
		}
		
		return stubs;
	}
	
	protected int ConvertStanceToInt(ECharacterStance stance)
	{
		switch (stance)
		{
			case ECharacterStance.STAND:
				return ECharacterStanceChange.STANCECHANGE_TOERECTED;
			case ECharacterStance.CROUCH:
				return ECharacterStanceChange.STANCECHANGE_TOCROUCH;
			case ECharacterStance.PRONE:
				return ECharacterStanceChange.STANCECHANGE_TOPRONE;
		}
		return 0;
	}
	
	protected vector GetWaypointOrigin(vector from, int iterations = 20)
	{
		if (iterations <= 0)
		{
			return vector.Zero;
		}
		
		RandomGenerator gen();
		vector pos = gen.GenerateRandomPointInRadius(10, 300, from);
		
		vector finalPos;
		bool posFound = SCR_WorldTools.FindEmptyTerrainPosition(finalPos, pos, 50);
		
		if (posFound)
		{
			posFound = ! Z_Core.IsUnderwater(finalPos);
		}
		
		if (! posFound)
		{
			return GetWaypointOrigin(from, iterations - 1);
		}
		
		return finalPos;
	}
};
