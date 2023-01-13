[EL_DbName(Z_CoreSaveData, "Z_ScavCore")]
class Z_CoreSaveData : EL_ScriptedStateSaveDataBase
{
    bool m_HasSeededScavEncounters;
}

[EL_PersistentScriptedStateSettings(Z_Core, Z_CoreSaveData, autoSave: true, shutDownSave: true, selfDelete: false)]
class Z_Core : EL_PersistentScriptedStateBase
{
	bool m_HasSeededScavEncounters;
	
	static Z_Core GetInstance()
	{
		return EL_PersistentScriptedStateLoader<Z_Core>.LoadSingleton();
	}
	
	static bool IsPlayerNear(vector pos, float distance)
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
	
	static bool IsPlayerInsideCell(string cell)
	{
		array<int> players();
		GetGame().GetPlayerManager().GetPlayers(players);
		
		if (! players) return false;
		
		foreach (int playerId : players)
		{
			IEntity playerEnt = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			
			if (! playerEnt) continue;
			
			string playerCell = SCR_MapEntity.GetGridPos(playerEnt.GetOrigin());
			
			if (IsCellInsideCell(playerCell, cell))
			{
				return true;
			}
		}
		
		return false;
	}
	
	static bool IsCellInsideCell(string innerCell, string outerCell)
	{
		return innerCell == outerCell;
		
		/*
		if (! buffer)
		{
			return innerCell == outerCell;
		}
		
		array<string> coords();
		outerCell.Split(" ", coords, true);
				
		int x = coords[0].ToInt();
		int y = coords[1].ToInt();
		
		array<string> buffered();
		
		buffered.Insert(string.Format("%1 %2", (x - 1).ToString(3), (y + 1).ToString(3)));
		buffered.Insert(string.Format("%1 %2", (x).ToString(3), (y + 1).ToString(3)));
		buffered.Insert(string.Format("%1 %2", (x + 1).ToString(3), (y + 1).ToString(3)));
		buffered.Insert(string.Format("%1 %2", (x - 1).ToString(3), (y).ToString(3)));
		buffered.Insert(string.Format("%1 %2", (x + 1).ToString(3), (y).ToString(3)));
		buffered.Insert(string.Format("%1 %2", (x - 1).ToString(3), (y - 1).ToString(3)));
		buffered.Insert(string.Format("%1 %2", (x).ToString(3), (y - 1).ToString(3)));
		buffered.Insert(string.Format("%1 %2", (x + 1).ToString(3), (y - 1).ToString(3)));
		
		foreach (string bCell : buffered)
		{
			if (innerCell == bCell)
			{
				return true;
			}
		}
		
		return false;
		*/
	}
	
	static int GetCurrentTimestampInHours()
	{
		int epochYear = 2021;
		int year, month, day, hour, minute, second;
		
		System.GetYearMonthDayUTC(year, month, day);
		System.GetHourMinuteSecondUTC(hour, minute, second);
		
		year = (year - epochYear) * 8760;
		month = month * 730;
		day = day * 24;
		
		return year + month + day + hour;
	}
	
	static bool IsUnderwater(vector origin)
	{
		vector outWaterSurfacePoint;
		EWaterSurfaceType outType;
		vector transformWS[4];
		vector obbExtents;
		
		float y = GetGame().GetWorld().GetSurfaceY(origin[0], origin[2]);
                    
		if (y > 0)
		{
		    origin[1] = y;
		}
		
		return ChimeraWorldUtils.TryGetWaterSurface(GetGame().GetWorld(), origin, outWaterSurfacePoint, outType, transformWS, obbExtents);
	}
	
	bool HasSeededScavEncounters()
	{
		return m_HasSeededScavEncounters;
	}
	
	void SetHasSeededScavEncounters(bool state)
	{
		m_HasSeededScavEncounters = state;
		
		Save();
	}
}
