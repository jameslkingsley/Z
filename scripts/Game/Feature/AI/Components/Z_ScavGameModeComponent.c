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
		
		// GetGame().GetCallqueue().CallLater(TestTemp, 2000);
		
		Print("---- ReforgerZ Scav OnWorldPostProcess Complete ----");
		
		GetGame().GetCallqueue().CallLater(InitializeHeatMap, 10000, true);
	}
	
	void InitializeHeatMap()
	{
		Z_HeatMap.LoadEncounters();
		
		Z_HeatMap.LoadWeights();
		
		Z_HeatMap.LoadProbabilities();
	}
	
	void WorldGridTest()
	{
		Z_ScavEncounter.Create(Vector(1347.174, 37.803, 2969.852), Z_ScavEncounterImportance.High);
	}
	
	ref array<Z_ScavRegionComponent> GetScavRegions()
	{
		return m_ScavRegions;
	}
	
	ref Tuple2<int, int> GetScavRegionAttritionWeights()
	{
		int min = 0;
		int max = 0;
		
		foreach (Z_ScavRegionComponent region : GetScavRegions())
		{
			if (region.m_Attrition < min) min = region.m_Attrition;
			if (region.m_Attrition > max) max = region.m_Attrition;
		}
		
		return new Tuple2<int, int>(min, max);
	}
	
	void RegisterScavRegion(Z_ScavRegionComponent region)
	{
		if (m_ScavRegions.Contains(region))
			return;
		
		m_ScavRegions.Insert(region);
	}
}
