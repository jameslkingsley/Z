enum Z_ScavDifficulty
{
	Low,
	Medium,
	High,
	Insane
};

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
	vector direction;
	ECharacterStance stance;
};

class Z_ScavTaskEntityStubInternal : Z_ScavTaskEntityStub
{
	IEntity ent;
	
	static Z_ScavTaskEntityStubInternal FromStub(Z_ScavTaskEntityStub stub, IEntity entity)
	{
		Z_ScavTaskEntityStubInternal instance();
		
		instance.ent = entity;
		instance.resource = stub.resource;
		instance.origin = stub.origin;
		instance.direction = stub.direction;
		instance.stance = stub.stance;
		
		return instance;
	}
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

[Z_ScavTask(Z_ScavTaskBase, "Base"), Z_ScavTaskTitle(), BaseContainerProps()]
class Z_ScavTaskBase
{
	[Attribute("100", UIWidgets.Auto, "Attrition cost of this task")]
	int m_AttritionCost;
	
	[Attribute("0", UIWidgets.ComboBox, "AI faction to spawn for this task", "", ParamEnumArray.FromEnum(Z_ScavFaction))]
	Z_ScavFaction m_Faction;
	
	ref array<ref Z_ScavTaskEntityStubInternal> SpawnEntityStubs(vector origin, array<ref Z_ScavTaskEntityStub> stubs);
	
	ref array<ref Z_ScavTaskEntityStub> GetEntityStubs(Z_PersistentScavTask task);
	
	string GetFactionKey()
	{
		return Z_ScavGameModeComponent.GetInstance().GetConfig().GetFactionKey();
	}
};
