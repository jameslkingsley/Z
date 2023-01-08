enum Z_ScavEncounterImportance
{
	Low = 3,
	Medium = 8,
	High = 13
}

enum Z_ScavEncounterExpiryTime
{
	Low = 12,
	Medium = 48,
	High = 168
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

[EL_DbName(Z_ScavEncounterSaveData, "Z_ScavEncounter")]
class Z_ScavEncounterSaveData : EL_ScriptedStateSaveDataBase
{
    string cell;
	vector origin;
	Z_ScavEncounterImportance importance;
	int createdAtInHours;
}

[EL_PersistentScriptedStateSettings(Z_ScavEncounter, Z_ScavEncounterSaveData, autoSave: false, shutDownSave: false, selfDelete: false)]
class Z_ScavEncounter : EL_PersistentScriptedStateBase
{
	string cell;
	vector origin;
	Z_ScavEncounterImportance importance;
	int createdAtInHours;
	
	static Z_ScavEncounter Create(vector o, Z_ScavEncounterImportance i)
	{
		Z_ScavEncounter instance();
		
		instance.cell = SCR_MapEntity.GetGridPos(o);
		instance.origin = o;
		instance.importance = i;
		instance.createdAtInHours = Z_Core.GetCurrentTimestampInHours();
		
		instance.Save();
		
		return instance;
	}
	
	bool HasExpired()
	{
		int current = Z_Core.GetCurrentTimestampInHours();
		
		int diff = current - createdAtInHours;
		
		int stalenessAgeInHours = GetExpiryTime();
		
		if (stalenessAgeInHours == -1) return false;
		
		return diff >= stalenessAgeInHours;
	}
	
	int GetExpiryTime()
	{
		if (importance == Z_ScavEncounterImportance.Low) return Z_ScavEncounterExpiryTime.Low;
		if (importance == Z_ScavEncounterImportance.Medium) return Z_ScavEncounterExpiryTime.Medium;
		if (importance == Z_ScavEncounterImportance.High) return Z_ScavEncounterExpiryTime.High;
		return -1;
	}
		
	int CalculateHeat()
	{
		return importance;
	}
}
