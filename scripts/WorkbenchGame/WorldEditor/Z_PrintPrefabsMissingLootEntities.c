[WorkbenchPluginAttribute("Print prefabs missing loot entities", "Print the prefabs that are missing loot entities (volumes and containers) in the console", "", "", {"WorldEditor", "ResourceManager"}, "", 0xf1c0)]
class Z_PrintPrefabsMissingLootEntities: WorkbenchPlugin
{
	override void Run()
	{
		array<string> paths = {"Prefabs/Props", "Prefabs/Structures"};
		array<ResourceName> prefabs = {};
		array<ResourceName> prefabsMissingLootEntities = {};
		
		Workbench.SearchResources(prefabs.Insert, {"et"}, null, "Prefabs");
		
		foreach (ResourceName prefab : prefabs)
		{
			Resource res = Resource.Load(prefab);
			
			if (! res) continue;
			
			BaseResourceObject obj = res.GetResource();
			
			if (! obj) continue;
			
			BaseContainer con = obj.ToBaseContainer();
			
			array<string> addonNames = {};
			con.GetSourceAddons(addonNames);
			
			if (addonNames.Contains("Z"))
			{
				if (! prefab.GetPath().Contains("Prefabs/Props/") && ! prefab.GetPath().Contains("Prefabs/Structures/"))
				{
					continue;
				}
				
				if (! FileIO.FileExist("C:\\Users\\james\\Documents\\My Games\\ArmaReforgerWorkbench\\addons\\Z\\" + prefab.GetPath()))
				{
					Print("File does not exist", LogLevel.ERROR);
					
					continue;
				}
				
				if (! HasEntityType(con, "Z_LootContainerEntity") && ! HasEntityType(con, "Z_LootVolumeEntity"))
				{
					if (! prefabsMissingLootEntities.Contains(prefab))
						prefabsMissingLootEntities.Insert(prefab);
				}
			}
		}
		
		foreach (ResourceName prefab : prefabsMissingLootEntities)
		{
			// Print(prefab.GetPath(), LogLevel.WARNING);
			PrintFormat("@\"%1\"", prefab.GetPath());
		}
	}
	
	private bool HasEntityType(BaseContainer container, string type)
	{
		if (container.GetClassName() == type)
			return true;
		
		for (int i = 0; i < container.GetNumChildren(); i++)
		{
			if (HasEntityType(container.GetChild(i), type))
			{
				return true;
			}
		}
		
		return false;
	}
};
