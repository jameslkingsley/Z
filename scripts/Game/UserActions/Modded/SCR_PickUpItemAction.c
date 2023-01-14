modded class SCR_PickUpItemAction : SCR_InventoryAction
{
	InventoryItemComponent m_InventoryItemComponent;
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		
		m_InventoryItemComponent = InventoryItemComponent.Cast(pOwnerEntity.FindComponent(InventoryItemComponent));
	}
	
	override bool GetActionNameScript(out string outName)
	{
		if (! m_InventoryItemComponent)
		{
			outName = "#AR-Inventory_PickUp";
			
			return true;
		}
		
		UIInfo uiInfo = m_InventoryItemComponent.GetUIInfo();
		
		if (! uiInfo || uiInfo.GetName().IsEmpty())
		{
			outName = "#AR-Inventory_PickUp";
			
			return true;
		}
		
		outName = string.Format("#AR-Inventory_PickUp %1", uiInfo.GetName());
		
		return true;
	}
}
