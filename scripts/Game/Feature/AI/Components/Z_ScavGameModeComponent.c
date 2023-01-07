[ComponentEditorProps(category: "Z/Components", description: "")]
class Z_ScavGameModeComponentClass: SCR_BaseGameModeComponentClass
{}

class Z_ScavGameModeComponent: SCR_BaseGameModeComponent
{
	[Attribute("{19994566F9D13227}Config/Z_ScavConfig.conf", UIWidgets.ResourceNamePicker, "Scav config")]
	ResourceName m_ScavConfig;
	
	ref array<Z_ScavRegionComponent> m_ScavRegions = {};
	
	ref Z_ScavConfig m_ScavConfigCache;
	
	static Z_ScavGameModeComponent GetInstance()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		
		if (gameMode)
			return Z_ScavGameModeComponent.Cast(gameMode.FindComponent(Z_ScavGameModeComponent));
		else
			return null;
	}
	
	ref Z_ScavConfig GetConfig()
	{
		if (m_ScavConfigCache)
		{
			return m_ScavConfigCache;
		}
		
		Resource container = BaseContainerTools.LoadContainer(m_ScavConfig);
					
		m_ScavConfigCache = Z_ScavConfig.Cast(
			BaseContainerTools.CreateInstanceFromContainer(container.GetResource().ToBaseContainer())
		);
		
		return m_ScavConfigCache;
	}
	
	override void OnWorldPostProcess(World world)
	{
		if (! Replication.IsServer() || ! GetGame().InPlayMode()) return;
		
		if (! Z_Core.GetInstance().HasSeededScavEncounters())
		{
			GetGame().GetCallqueue().CallLater(FinishSeedingScavEncounters, 5000);
			GetGame().GetCallqueue().CallLater(InitializeHeatMap, 6000);
			GetGame().GetCallqueue().CallLater(InitializeTasks, 7000);
		}
		else
		{
			GetGame().GetCallqueue().CallLater(InitializeHeatMap, 1000);
			GetGame().GetCallqueue().CallLater(InitializeTasks, 2000);
		}
		
		GetGame().GetCallqueue().CallLater(InitializeHeatMap, 1800 * 1000);
		GetGame().GetCallqueue().CallLater(InitializeTasks, 1810 * 1000);
		
		Print("---- ReforgerZ Scav OnWorldPostProcess Complete ----");
	}
	
	override void OnPlayerKilled(int playerId, IEntity player, IEntity killer)
	{
		if (! Replication.IsServer() || ! GetGame().InPlayMode()) return;
		
		Z_ScavEncounter.Create(player.GetOrigin(), Z_ScavEncounterImportance.Medium);
		
		Print("Player died, creating encounter: " + player.GetOrigin());
	}
	
	void FinishSeedingScavEncounters()
	{
		Z_Core.GetInstance().SetHasSeededScavEncounters(true);
		
		Print("Seeded scav encounters - SHOULD ONLY HAPPEN ONCE ON FRESH WIPE", LogLevel.WARNING);
	}
	
	void InitializeHeatMap()
	{
		Z_HeatMap.Clear();
		
		Z_HeatMap.LoadEncounters();
		
		Z_HeatMap.LoadWeights();
		
		Z_HeatMap.LoadProbabilities();
	}
	
	void InitializeTasks()
	{
		vector worldMin, worldMax;
		GetGame().GetWorld().GetBoundBox(worldMin, worldMax);
		
		for (float x = 0; x < worldMax[0]; x += Z_HeatMap.CELL_SIZE)
		{
			for (float z = 0; z < worldMax[2]; z += Z_HeatMap.CELL_SIZE)
			{
				vector origin = Vector(x, 0, z);
				string cell = SCR_MapEntity.GetGridPos(origin);
				
				float prob = Z_HeatMap.GetProbability(cell);
				
				// Random number needs to be below probability for us to consider spawning any tasks
				if (Math.RandomFloat(0, 1) > prob) continue;
				
				Z_ScavRegionComponent region = GetScavRegionThatSurroundsOrigin(origin);
				
				// We can only spawn tasks that are inside a defined region
				if (! region) continue;
				
				region.InitializeTasks(cell, origin);
			}
		}
	}
	
	Z_ScavRegionComponent GetScavRegionThatSurroundsOrigin(vector origin)
	{
		foreach (Z_ScavRegionComponent region : GetScavRegions())
		{
			PolylineArea area = region.GetPolylineArea();
			
			if (! area) continue;
			
			if (region.GetPolylineArea().IsPosInside(origin))
				return region;
		}
		
		return null;
	}
	
	ref array<Z_ScavRegionComponent> GetScavRegions()
	{
		return m_ScavRegions;
	}
	
	void RegisterScavRegion(Z_ScavRegionComponent region)
	{
		if (m_ScavRegions.Contains(region))
			return;
		
		m_ScavRegions.Insert(region);
	}
}
