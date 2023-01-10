class Z_AssemblePrefabAction : ScriptedUserAction
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Buildable to assemble", params: "et")]
	protected ResourceName m_Buildable;
	
	override bool CanBeShownScript(IEntity user)
	{
		return true;
	}

	override bool CanBePerformedScript(IEntity user)
	{
		return true;
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		Z_AssemblyComponent assemblyComponent = Z_AssemblyComponent.Cast(pOwnerEntity.FindComponent(Z_AssemblyComponent));
		
		if (!assemblyComponent)
		{
			Print("Owner is missing Z_AssemblyComponent", LogLevel.ERROR);
			
			return;
		}
			
		assemblyComponent.RequestAssembly(pOwnerEntity, pUserEntity, m_Buildable);
	}
}
