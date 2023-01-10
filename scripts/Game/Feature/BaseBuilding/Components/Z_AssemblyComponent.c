[ComponentEditorProps(category: "Z/Components", description: "")]
class Z_AssemblyComponentClass: ScriptComponentClass
{
};

class Z_AssemblyComponent: ScriptComponent
{
	void RequestAssembly(IEntity owner, IEntity user, ResourceName resource)
	{
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		if (!rplComponent)
		{
			Print("Owner of Z_AssemblyComponent does not have RplComponent", LogLevel.ERROR);
			
			return;
		}
		
		vector direction = user.GetYawPitchRoll().AnglesToVector();
		vector forwardVector = owner.GetOrigin() + (direction * 1.2);
		forwardVector[1] = owner.GetWorld().GetSurfaceY(forwardVector[0], forwardVector[2]);
		
		Rpc(RpcDo_RequestAssemble, rplComponent.Id(), resource, forwardVector);
	}
	
	void RequestDisassembly(IEntity owner, IEntity user)
	{
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		if (!rplComponent)
		{
			Print("Owner of Z_AssemblyComponent does not have RplComponent", LogLevel.ERROR);
			
			return;
		}
		
		Rpc(RpcDo_RequestDisassemble, rplComponent.Id());
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_RequestDisassemble(RplId rplCompId)
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(rplCompId));
		if (!rplComp) return;
		
		IEntity owner = rplComp.GetEntity();
		
		IEntity ent = Z_BaseBuildingGameModeComponent.GetInstance().SpawnConstructionTool(owner.GetOrigin());
		
		rplComp.DeleteRplEntity(owner, false);
		
		EL_PersistenceComponent persistenceComponent = EL_PersistenceComponent.Cast(ent.FindComponent(EL_PersistenceComponent));
		
		if (! persistenceComponent) return;
		
		persistenceComponent.Save();
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_RequestAssemble(RplId rplCompId, ResourceName res, vector forwardVector)
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(rplCompId));
		if (!rplComp) return;
		
		IEntity owner = rplComp.GetEntity();
		
		Z_Buildable buildable = Z_BaseBuildingGameModeComponent.GetInstance().FindBuildable(res);
		
		if (! buildable)
		{
			Print(string.Format("No buildable config entry for %1", res), LogLevel.ERROR);
			
			return;
		}
		
		IEntity ent = buildable.Spawn(forwardVector);
		
		rplComp.DeleteRplEntity(owner, false);
		
		EL_PersistenceComponent persistenceComponent = EL_PersistenceComponent.Cast(ent.FindComponent(EL_PersistenceComponent));
		
		if (! persistenceComponent) return;
		
		persistenceComponent.Save();
	}
}
