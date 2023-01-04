[Z_ScavTask(Z_ScavTaskPatrol, "Patrol"), Z_ScavTaskTitle(), BaseContainerProps()]
class Z_ScavTaskPatrol : Z_ScavTaskBase
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Prefab to spawn")]
	ResourceName m_AIGroupPrefab;
	
	override ref map<IEntity, ref Z_ScavTaskEntityStub> SpawnEntityStubs(array<ref Z_ScavTaskEntityStub> stubs)
	{
		ref map<IEntity, ref Z_ScavTaskEntityStub> watchers();
		
		foreach (ref Z_ScavTaskEntityStub stub : stubs)
		{
			EntitySpawnParams spawnParams = EntitySpawnParams();
			spawnParams.TransformMode = ETransformMode.WORLD;
			spawnParams.Transform[3] = stub.origin;
			
			IEntity ent = GetGame().SpawnEntityPrefab(
				Resource.Load(stub.resource),
				GetGame().GetWorld(),
				spawnParams
			);
			
			SCR_AIGroup group = SCR_AIGroup.Cast(ent);
			
			Resource wpPrefab = Resource.Load("{22A875E30470BD4F}Prefabs/AI/Waypoints/AIWaypoint_Patrol.et");
			
			AIWaypoint wp = AIWaypoint.Cast(GetGame().SpawnEntityPrefab(wpPrefab));
			wp.SetOrigin(stub.origin + Vector(Math.RandomInt(200, 500), 0, Math.RandomInt(200, 500)));
			
			Print("Distance to waypoint: " + vector.Distance(stub.origin, wp.GetOrigin()));
			
			group.AddWaypointToGroup(wp);
			
			// Register event listeners on SCR_AIGroup
			// - Record encounters for when AI are killed
			
			watchers.Set(ent, stub);
		}
		
		return watchers;
	}
	
	override ref array<ref Z_ScavTaskEntityStub> UpdateEntityStubs(map<IEntity, ref Z_ScavTaskEntityStub> watchers)
	{
		ref array<ref Z_ScavTaskEntityStub> stubs();
		
		foreach (IEntity ent, ref Z_ScavTaskEntityStub stub : watchers)
		{
			if (! ent) continue;
			
			ref Z_ScavTaskEntityStub newStub();
			
			newStub.resource = stub.resource;
			newStub.origin = ent.GetOrigin();
			
			stubs.Insert(newStub);
		}
		
		return stubs;
	}
	
	override ref array<ref Z_ScavTaskEntityStub> GetEntityStubs(Z_PersistentScavTask task)
	{
		ref array<ref Z_ScavTaskEntityStub> stubs();
		
		ref Z_ScavTaskEntityStub stub();
		
		stub.resource = m_AIGroupPrefab;
		stub.origin = task.m_Origin;
		
		stubs.Insert(stub);
		
		return stubs;
	}
};
