[ComponentEditorProps(category: "Z/Components", description: "")]
class Z_ScavGameModeComponentClass: SCR_BaseGameModeComponentClass
{}

class Z_ScavGameModeComponent: SCR_BaseGameModeComponent
{
	[Attribute("{19994566F9D13227}Config/Z_ScavConfig.conf", UIWidgets.ResourceNamePicker, "Scav config")]
	ResourceName m_ScavConfig;
	
	[Attribute("30", UIWidgets.Auto, "Minutes between heat map updates")]
	int m_HeatMapGenerationInterval;
	
	[Attribute("10", UIWidgets.Auto, "Interval in seconds for when nearby scav tasks are spawned")]
	int m_ScavTaskIntervalInSeconds;
	
	[Attribute("2500", UIWidgets.Auto, "Player distance to scav tasks before spawning")]
	int m_ScavTaskSpawningDistance;
	
	[Attribute("10", UIWidgets.Auto, "Impact on attrition when player is killed inside region")]
	int m_PlayerDeathAttritionImpact;
	
	[Attribute("-10", UIWidgets.Auto, "Impact on attrition when scav is killed inside region")]
	int m_ScavDeathAttritionImpact;
	
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
		
		GetGame().GetCallqueue().CallLater(ManageTasks, m_ScavTaskIntervalInSeconds * 1000, true);
		
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
		
		// Possibly just do this on server restarts?
		GetGame().GetCallqueue().CallLater(InitializeHeatMap, m_HeatMapGenerationInterval * 60 * 1000);
		GetGame().GetCallqueue().CallLater(InitializeTasks, ((m_HeatMapGenerationInterval * 60) + 10) * 1000);
		
		Print("---- ReforgerZ Scav OnWorldPostProcess Complete ----");
	}
	
	void ManageTasks()
	{
		foreach (Z_ScavRegionComponent region : GetScavRegions())
		{
			foreach (string gridCell, Z_PersistentScavTask task : region.GetTasks())
			{
				if (IsPlayerNear(task.GetOrigin(), m_ScavTaskSpawningDistance))
				{
					if (! task.HasSpawned())
					{
						task.Spawn(region.GetOwner());
						Print("Spawning task (player near)");
					}
				}
				else
				{
					if (task.HasSpawned())
					{
						task.DespawnOnNextUpdate(true);
						Print("Despawning task (player not near)");
					}
				}
			}
		}
	}
	
	bool IsPlayerNear(vector pos, float distance)
	{
		array<int> players();
		GetGame().GetPlayerManager().GetPlayers(players);
		
		if (! players) return false;
		
		foreach (int playerId : players)
		{
			IEntity playerEnt = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			
			if (! playerEnt) continue;
			
			if (vector.Distance(playerEnt.GetOrigin(), pos) <= distance)
			{
				return true;
			}
		}
		
		return false;
	}
	
	override void OnControllableDestroyed(IEntity entity, IEntity instigator)
	{
		if (! Replication.IsServer() || ! GetGame().InPlayMode()) return;
		
		if (EntityUtils.IsPlayer(entity)) return;
		
		Z_ScavEncounter.Create(entity.GetOrigin(), Z_ScavEncounterImportance.High);
		
		Z_ScavRegionComponent region = GetScavRegionThatSurroundsOrigin(entity.GetOrigin());
		
		if (region)
		{
			region.ImpactAttrition(m_ScavDeathAttritionImpact);
		}
		
		Print("Scav died, creating encounter: " + entity.GetOrigin());
	}
	
	override void OnPlayerKilled(int playerId, IEntity player, IEntity killer)
	{
		if (! Replication.IsServer() || ! GetGame().InPlayMode()) return;
		
		Z_ScavEncounter.Create(player.GetOrigin(), Z_ScavEncounterImportance.Medium);
		
		Z_ScavRegionComponent region = GetScavRegionThatSurroundsOrigin(player.GetOrigin());
		
		if (region)
		{
			region.ImpactAttrition(m_PlayerDeathAttritionImpact);
		}
		
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
