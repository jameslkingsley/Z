enum Z_ScavEncounterImportance
{
	Low = 2,
	Medium = 5,
	High = 13
}

// Iterate over grid positions of map and for each cell lookup the encounters matching grid
// Iteration starts with mins of world bounds and increments Z until at maxs[2] 
// Then Increments X until at maxs[0]
// Increment amount is close to grid cell size
// Each position can be converted into grid cell using SCR_MapEntity.GetGridPos()
// Can print whether cell was already processed - find increment size until this never happens
// With grid cell + matched encounters inside cell we can then calculate the cell heat
// Heat of cell is determined by the sum of encounter importance + age (older encounters weigh less)
// With all cells weighed, they'll each have an integer value
// These values can then be Lerp'd into a percentage (using lowest cell weight as min, highest as max)
// The resulting percentage is one component of the probability of spawning a task in that cell
// On top of this, each cell determines its available attrition (find region it's inside of)

// Simple solution for now + testing:
// Using attrition and the cost of allowed tasks within region, iterate downwards and create the highest cost but affordable task in the highest weighted cell
// Repeat process until region is filled. It will have hot areas as focused on high cost tasks, cold areas will be cheap tasks or nothing (if attrition ran out)

// Once region is filled, store timestamp
// Region will not re-fill until timestamp has elapsed X time (same setup as loot staleness)

// Location of task within cell can be determined using SCR_WorldTools.FindEmptyTerrainPosition()

// Might need 2 passes - first to weigh cells (using higher resolution) - second to spawn tasks (lower resolution (greater distances between tasks))

class Z_HeatMap
{
	const int CELL_SIZE = 100;
	
	static ref map<string, int> m_HeatMap = new map<string, int>();
	
	static ref map<string, float> m_HeatOriginProbabilities = new map<string, float>();
	
	static ref Tuple2<int, int> m_HeatWeights;
	
	static float GetProbability(string cell)
	{
		if (! m_HeatOriginProbabilities.Contains(cell))
			return 0;
		
		return m_HeatOriginProbabilities.Get(cell);
	}
	
	static void RecalculateProbability(string cell, Z_ScavRegionComponent region)
	{
		int heat = 0;
		
		if (m_HeatMap.Contains(cell))
		{
			heat = m_HeatMap.Get(cell);
		}
		
		float heatPercent = Math.InverseLerp(m_HeatWeights.param1, m_HeatWeights.param2, heat);
		
		heatPercent += region.GetAttritionProbability();
		
		heatPercent = Math.Clamp(heatPercent, 0, 1);
		
		m_HeatOriginProbabilities.Set(cell, heatPercent);
	}
	
	static void LoadProbabilities()
	{
		vector worldMin, worldMax;
		GetGame().GetWorld().GetBoundBox(worldMin, worldMax);
		
		for (float x = 0; x < worldMax[0]; x += Z_HeatMap.CELL_SIZE)
		{
			for (float z = 0; z < worldMax[2]; z += Z_HeatMap.CELL_SIZE)
			{
				vector origin = Vector(x, 0, z);
				string cell = SCR_MapEntity.GetGridPos(origin);
				
				// Default cell heat
				int heat = 0;
				
				if (m_HeatMap.Contains(cell))
				{
					heat = m_HeatMap.Get(cell);
				}
				
				float heatPercent = Math.InverseLerp(m_HeatWeights.param1, m_HeatWeights.param2, heat);
				
				Z_ScavRegionComponent region = GetScavRegionThatSurroundsOrigin(origin);
				
				if (region)
				{
					heatPercent += region.GetAttritionProbability();
				}
				
				heatPercent = Math.Clamp(heatPercent, 0, 1);
				
				m_HeatOriginProbabilities.Set(cell, heatPercent);
			}
		}
	}
	
	static void LoadWeights()
	{
		int min = Z_ScavEncounterImportance.Low;
		int max = Z_ScavEncounterImportance.Low;
		
		foreach (string _, int heat : m_HeatMap)
		{
			if (heat < min) min = heat;
			if (heat > max) max = heat;
		}
		
		m_HeatWeights = new Tuple2<int, int>(min, max);
	}
	
	static void LoadEncounters()
	{
		Z_ScavEncounterCallback callback();
		
		EL_PersistentScriptedStateSettings settings = EL_PersistentScriptedStateSettings.Get(Z_ScavEncounter);
		
		if (!settings || !settings.m_tSaveDataType)
		{
			Debug.Error(string.Format("Scripted state type '%1' needs to have no save-data configured to be loaded!", Z_ScavEncounter));
			return;
		}
		
		EL_ScriptedStateLoaderCallbackInvokerSingle<Z_ScavEncounter> callbackInvoker(callback);
		EL_ScriptedStateLoaderProcessorCallbackSingle processorCallback();
		processorCallback.Setup(callbackInvoker, true, Z_ScavEncounter);
		EL_PersistenceManager.GetInstance().GetDbContext().FindAllAsync(settings.m_tSaveDataType, callback: processorCallback);
	}
	
	static Z_ScavRegionComponent GetScavRegionThatSurroundsOrigin(vector origin)
	{
		foreach (Z_ScavRegionComponent region : Z_ScavGameModeComponent.GetInstance().GetScavRegions())
		{
			PolylineArea area = region.GetPolylineArea();
			
			if (! area) continue;
			
			if (region.GetPolylineArea().IsPosInside(origin))
				return region;
		}
		
		return null;
	}
}

class Z_ScavEncounterCallback : EL_ScriptedStateLoaderCallbackSingle<Z_ScavEncounter>
{
	override void OnComplete(Managed context, Z_ScavEncounter data)
	{
		int heat = 0;
		
		if (Z_HeatMap.m_HeatMap.Contains(data.cell))
		{
			heat = Z_HeatMap.m_HeatMap.Get(data.cell);
		}
		
		heat += data.CalculateHeat();
		
		Z_HeatMap.m_HeatMap.Set(data.cell, heat);
	}
}

[EL_DbName(Z_ScavEncounterSaveData, "Z_ScavEncounter")]
class Z_ScavEncounterSaveData : EL_ScriptedStateSaveDataBase
{
    string cell;
	vector origin;
	Z_ScavEncounterImportance importance;
}

[EL_PersistentScriptedStateSettings(Z_ScavEncounter, Z_ScavEncounterSaveData, autoSave: false, shutDownSave: false, selfDelete: false)]
class Z_ScavEncounter : EL_PersistentScriptedStateBase
{
	string cell;
	vector origin;
	Z_ScavEncounterImportance importance;
	
	static Z_ScavEncounter Create(vector o, Z_ScavEncounterImportance i)
	{
		Z_ScavEncounter instance();
		
		instance.cell = SCR_MapEntity.GetGridPos(o);
		instance.origin = o;
		instance.importance = i;
		
		// Encounters don't need to be kept alive in memory, so just save it straight away
		instance.Save();
		
		return instance;
	}
	
	int CalculateHeat()
	{
		return importance;
	}
}
