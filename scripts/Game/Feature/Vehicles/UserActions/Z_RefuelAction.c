class Z_RefuelAction : ScriptedUserAction
{
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
		Z_VehicleRefuelingComponent refuelingComponent = Z_VehicleRefuelingComponent.Cast(pOwnerEntity.FindComponent(Z_VehicleRefuelingComponent));
		
		if (!refuelingComponent)
		{
			Print("Owner is missing Z_VehicleRefuelingComponent", LogLevel.ERROR);
			
			return;
		}
			
		refuelingComponent.RequestRefuel(pOwnerEntity, pUserEntity);
	}
}
