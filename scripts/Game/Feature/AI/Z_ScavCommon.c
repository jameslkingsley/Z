enum Z_ScavDifficulty
{
	Low,
	Medium,
	High,
	Insane
};

enum Z_ScavEncounterImportance
{
	//
};

class Z_ScavEncounter
{};

class Z_ScavTaskTitle : SCR_ContainerActionTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = source.GetClassName();
		title.Replace("Z_ScavTask", "");
		string sOriginal = title;
		SplitStringByUpperCase(sOriginal, title);
		return true;
	}
};

class Z_ScavTaskEntityStub
{
	ResourceName resource;
	vector origin;
	vector yawPitchRoll;
	IEntity ent;
};

class Z_ScavTaskFactory
{
	static Z_ScavTaskBase Make(string type)
	{
		switch (type)
		{
			case Z_ScavTaskPatrol.NAME:
				return new Z_ScavTaskPatrol();
		}
		
		return null;
	}
}

[BaseContainerProps(), Z_ScavTaskTitle()]
class Z_ScavTaskBase
{
	[Attribute("100", UIWidgets.Auto, "Attrition cost of this task")]
	int m_AttritionCost;
	
	void Spawn(inout Z_PersistentScavTask task);
};
