[ComponentEditorProps(category: "Z/Components", description: "")]
class Z_VehicleContainerComponentClass: ScriptComponentClass
{
}

class Z_VehicleContainerComponent: ScriptComponent
{
	protected override void OnPostInit(IEntity owner)
	{
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		if (rplComponent && rplComponent.Role() == RplRole.Authority)
		{
		}
	}
}
