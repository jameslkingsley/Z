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
