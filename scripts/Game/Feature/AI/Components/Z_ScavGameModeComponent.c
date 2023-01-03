[ComponentEditorProps(category: "Z/Components", description: "")]
class Z_ScavGameModeComponentClass: SCR_BaseGameModeComponentClass
{}

class Z_ScavGameModeComponent: SCR_BaseGameModeComponent
{
	ref array<Z_ScavRegionComponent> m_ScavRegions = {};
	
	static Z_ScavGameModeComponent GetInstance()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		
		if (gameMode)
			return Z_ScavGameModeComponent.Cast(gameMode.FindComponent(Z_ScavGameModeComponent));
		else
			return null;
	}
	
	override void OnWorldPostProcess(World world)
	{
		if (! Replication.IsServer() || ! GetGame().InPlayMode()) return;
		
		GetGame().GetCallqueue().CallLater(TestTemp, 2000);
		
		Print("---- ReforgerZ Scav OnWorldPostProcess Complete ----");
	}
	
	void RegisterScavRegion(Z_ScavRegionComponent region)
	{
		if (m_ScavRegions.Contains(region))
			return;
		
		m_ScavRegions.Insert(region);
	}
	
	void TestTemp()
	{
		foreach (Z_ScavRegionComponent region : m_ScavRegions)
		{
			if (! region.m_Tasks.IsEmpty()) continue;
			
			Z_PersistentScavTask task = Z_PersistentScavTask.Create(Z_ScavTaskPatrol.NAME, Vector(1311.074, 39.011, 2746.414));
			
			region.RegisterTask(task);
			
			task.Spawn();
			
			EL_PersistenceComponent persistence = EL_PersistenceComponent.Cast(region.GetOwner().FindComponent(EL_PersistenceComponent));
			
			persistence.Save();
		}
	}
}
