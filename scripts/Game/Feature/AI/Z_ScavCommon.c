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

class Z_ScavTaskEntityStubItem
{
	vector origin;
	vector yawPitchRoll;
	bool alive = true;
	
	void Fill(IEntity ent)
	{
		origin = ent.GetOrigin();
		yawPitchRoll = ent.GetYawPitchRoll();
		
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(ent);

		if (! char) return;

		CharacterControllerComponent characterController = CharacterControllerComponent.Cast(char.FindComponent(CharacterControllerComponent));
		
		if (! characterController) return;
		
		alive = ! characterController.IsDead();
	}
	
	void Fill(AIAgent agent)
	{
		//
	}
};

// refactor this to base class so it's simpler to define how an AI group is get/set
class Z_ScavTaskEntityStub
{
	ResourceName resource;
	vector origin;
	
	ref array<ref Z_ScavTaskEntityStubItem> items = new array<ref Z_ScavTaskEntityStubItem>();
	
	void Fill(IEntity ent)
	{
		resource = ent.GetPrefabData().GetPrefabName();
		origin = ent.GetOrigin();
		
		SCR_AIGroup group = SCR_AIGroup.Cast(ent);
		if (group) Fill(group);
	}
	
	void Fill(AIGroup group)
	{
		array<AIAgent> agents = new array<AIAgent>();
		group.GetAgents(agents);
		
		items.Clear();
		
		foreach (AIAgent agent : agents)
		{
			IEntity agentEnt = agent.GetControlledEntity();
			
			Z_ScavTaskEntityStubItem item = new Z_ScavTaskEntityStubItem();
			item.Fill(agentEnt);
			item.Fill(agent);
			
			items.Insert(item);
		}
	}
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
	
	void Spawn(inout Z_PersistentScavTask task, inout map<IEntity, ref Z_ScavTaskEntityStub> entities);
};
