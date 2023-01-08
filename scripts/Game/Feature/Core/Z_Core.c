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
