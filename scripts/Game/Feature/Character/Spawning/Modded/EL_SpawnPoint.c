modded class EL_SpawnPoint
{
	override static EL_SpawnPoint GetDefaultSpawnPoint()
	{
		if (s_aSpawnPoints.IsEmpty()) return null;
		
		return s_aSpawnPoints.GetRandomElement();
	}
}
