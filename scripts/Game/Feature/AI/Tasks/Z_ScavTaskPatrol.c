[BaseContainerProps(), Z_ScavTaskTitle()]
class Z_ScavTaskPatrol : Z_ScavTaskBase
{
	static const string NAME = "Patrol";
	
	override void Spawn(inout Z_PersistentScavTask task, inout map<IEntity, ref Z_ScavTaskEntityStub> entities)
	{
		Print("Spawning patrol task");
		
		if (task.m_EntityStubs.IsEmpty())
		{
			Print("Spawning new AI group");
			
			// TODO replace with attribute
			ResourceName res = "{E552DABF3636C2AD}Prefabs/Groups/OPFOR/Group_USSR_RifleSquad.et";
			Resource agentPrefab = Resource.Load(res);
			
			EntitySpawnParams spawnParams = EntitySpawnParams();
			spawnParams.TransformMode = ETransformMode.WORLD;
			spawnParams.Transform[3] = task.m_Origin;
			
			IEntity entGroup = GetGame().SpawnEntityPrefab(
				agentPrefab,
				GetGame().GetWorld(),
				spawnParams
			);
			
			if (! entGroup)
			{
				Print(string.Format("Z_ScavTaskPatrol could not spawn '%1'", agentPrefab), LogLevel.ERROR);
				
				return;
			}
			
			SCR_AIGroup group = SCR_AIGroup.Cast(entGroup);
			
			if (! group)
			{
				Print(string.Format("Z_ScavTaskPatrol spawned entity '%1' that is not of SCR_AIGroup type, deleting!", agentPrefab), LogLevel.ERROR);
				
				RplComponent.DeleteRplEntity(entGroup, false);
				
				delete task;
				
				return;
			}
			
			Z_ScavTaskEntityStub stub = new Z_ScavTaskEntityStub();
			stub.Fill(entGroup);
			stub.Fill(group);
			
			task.m_EntityStubs.Insert(stub);
			entities.Set(entGroup, stub);
			
			Print("Saved entity stubs: " + task.m_EntityStubs.Count());
			
			/*
			ScriptInvoker onAgentAdded = group.GetOnAgentAdded();
			ScriptInvoker onAgentRemoved = group.GetOnAgentRemoved();
			
			onAgentAdded.Insert(OnAgentAdded);
			onAgentRemoved.Insert(OnAgentRemoved);
			*/
		}
		else
		{
			Print("Spawning previous AI group using stubs");
			
			foreach (ref Z_ScavTaskEntityStub stub : task.m_EntityStubs)
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
				
				array<AIAgent> agents = new array<AIAgent>();
				group.GetAgents(agents);
				
				foreach (int i, AIAgent agent : agents)
				{
					if (! stub.items.IsIndexValid(i))
					{
						Print("Index invalid, killing agent", LogLevel.WARNING);
						
						SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(agent.GetControlledEntity());
		
						if (! char) continue;
		
						CharacterControllerComponent characterController = CharacterControllerComponent.Cast(char.FindComponent(CharacterControllerComponent));
						if (characterController) characterController.ForceDeath();
						
						continue;
					}
					
					Z_ScavTaskEntityStubItem item = stub.items.Get(i);
					
					agent.GetControlledEntity().SetOrigin(item.origin);
					agent.GetControlledEntity().SetYawPitchRoll(item.yawPitchRoll);
					
					if (! item.alive)
					{
						SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(agent.GetControlledEntity());
		
						if (! char) continue;
		
						CharacterControllerComponent characterController = CharacterControllerComponent.Cast(char.FindComponent(CharacterControllerComponent));
						if (characterController) characterController.ForceDeath();
					}
				}
				
				entities.Set(ent, stub);
			}
		}
	}
	
	void OnAgentAdded(AIGroup group, AIAgent agent)
	{}
	
	void OnAgentRemoved(AIGroup group, AIAgent agent)
	{}
};
