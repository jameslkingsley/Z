class Z_DisassemblePrefabAction : ScriptedUserAction
{
	override bool CanBeShownScript(IEntity user)
	{
		return CanBePerformedScript(user);
	}

	override bool CanBePerformedScript(IEntity user)
	{
		SCR_UniversalInventoryStorageComponent storage = SCR_UniversalInventoryStorageComponent.Cast(GetOwner().FindComponent(SCR_UniversalInventoryStorageComponent));
		
		if (! storage)
		{
			Print("Missing storage component on disassemble prefab action", LogLevel.ERROR);
			
			return true;
		}
		
		return storage.GetTotalWeight() <= 0;
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		Z_AssemblyComponent assemblyComponent = Z_AssemblyComponent.Cast(pOwnerEntity.FindComponent(Z_AssemblyComponent));
		
		if (!assemblyComponent)
		{
			Print("Owner is missing Z_AssemblyComponent", LogLevel.ERROR);
			
			return;
		}
			
		assemblyComponent.RequestDisassembly(pOwnerEntity, pUserEntity);
	}
}
