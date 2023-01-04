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
};

class Z_ScavTask
{
	protected static ref map<typename, string> s_mMapping;

	static void Set(typename taskType, string name)
	{
		if (!s_mMapping) s_mMapping = new map<typename, string>();
		s_mMapping.Set(taskType, name);
	}

	static string Get(typename taskType)
	{
		if (!taskType) return string.Empty;

		if (!s_mMapping) s_mMapping = new map<typename, string>();

		string result = s_mMapping.Get(taskType);

		if (result.IsEmpty())
		{
			result = taskType.ToString();
			s_mMapping.Set(taskType, result);
		}

		return result;
	}

	static typename GetTypeByName(string name)
	{
		if (!s_mMapping) s_mMapping = new map<typename, string>();

		typename result = s_mMapping.GetKeyByValue(name);

		if (!result)
		{
			result = name.ToType();
			s_mMapping.Set(result, name);
		}

		return result;
	}

	void Z_ScavTask(typename taskType, string name)
	{
		Set(taskType, name);
	}
};

class Z_ScavTaskFactory
{
	static Z_ScavTaskBase Make(string type)
	{
		typename taskType = Z_ScavTask.GetTypeByName(type);
		
		return Z_ScavTaskBase.Cast(taskType.Spawn());
	}
}

[BaseContainerProps(), Z_ScavTaskTitle()]
class Z_ScavTaskBase
{
	[Attribute("100", UIWidgets.Auto, "Attrition cost of this task")]
	int m_AttritionCost;
	
	ref map<IEntity, ref Z_ScavTaskEntityStub> SpawnEntityStubs(array<ref Z_ScavTaskEntityStub> stubs);
	
	ref array<ref Z_ScavTaskEntityStub> UpdateEntityStubs(map<IEntity, ref Z_ScavTaskEntityStub> watchers);
	
	ref array<ref Z_ScavTaskEntityStub> GetEntityStubs(Z_PersistentScavTask task);
};
