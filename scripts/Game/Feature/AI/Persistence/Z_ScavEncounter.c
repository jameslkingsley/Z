enum Z_ScavEncounterImportance
{
	Low = 1,
	Medium = 2,
	High = 3
}

class Z_ScavEncounter
{
	int cell;
	vector origin;
	Z_ScavEncounterImportance importance;
}
