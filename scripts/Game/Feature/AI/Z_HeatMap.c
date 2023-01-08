class Z_HeatMap
{
	const int CELL_SIZE = 100;
	
	const int CELL_SIZE_DIAGONAL = 141;
	
	static ref map<string, int> m_HeatMap = new map<string, int>();
	
	static ref map<string, float> m_HeatMapProbabilities = new map<string, float>();
	
	static ref Tuple2<int, int> m_HeatWeights;
	
	static float GetProbability(string cell)
	{
		if (! m_HeatMapProbabilities.Contains(cell))
			return 0;
		
		return m_HeatMapProbabilities.Get(cell);
	}
	
	static void Clear()
	{
		m_HeatMap.Clear();
		m_HeatMapProbabilities.Clear();
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
				
				int heat = 0;
				
				if (m_HeatMap.Contains(cell))
				{
					heat = m_HeatMap.Get(cell);
				}
				
				float heatPercent = Math.InverseLerp(m_HeatWeights.param1, m_HeatWeights.param2, heat);
				
				m_HeatMapProbabilities.Set(cell, Math.Clamp(heatPercent, 0, 1));
			}
		}
	}
	
	static void LoadWeights()
	{
		int max = Z_ScavEncounterImportance.Low;
		
		foreach (string _, int heat : m_HeatMap)
		{
			if (heat > max) max = heat;
		}
		
		m_HeatWeights = new Tuple2<int, int>(0, max);
		
		Print("Loaded heat map weights: 0 - " + max);
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
		
		EL_ScriptedStateLoaderCallbackInvokerMultiple<Z_ScavEncounter> callbackInvoker(callback);
		EL_ScriptedStateLoaderProcessorCallbackMultiple processorCallback();
		processorCallback.Setup(callbackInvoker);
		EL_PersistenceManager.GetInstance().GetDbContext().FindAllAsync(settings.m_tSaveDataType, callback: processorCallback);
	}
}

class Z_ScavEncounterCallback : EL_ScriptedStateLoaderCallbackMultiple<Z_ScavEncounter>
{
	override void OnComplete(Managed context, array<ref Z_ScavEncounter> data)
	{
		foreach (ref Z_ScavEncounter encounter : data)
		{
			if (encounter.HasExpired())
			{
				encounter.Delete();
				
				Print("Scav encounter expired, deleted");
				
				continue;
			}
			
			int heat = 0;
		
			if (Z_HeatMap.m_HeatMap.Contains(encounter.cell))
			{
				heat = Z_HeatMap.m_HeatMap.Get(encounter.cell);
			}
			
			heat += encounter.CalculateHeat();
			
			Z_HeatMap.m_HeatMap.Set(encounter.cell, heat);
		}
	}
}
