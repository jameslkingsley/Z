[WorkbenchPluginAttribute("Highlight missing loot volumes", "Sinks all buildings and objects with loot volumes and containers into the ground", "", "", {"WorldEditor"}, "", 0xf1c0)]
class Z_HighlightMissingLootVolumes: WorkbenchPlugin
{
	[Attribute("false", UIWidgets.CheckBox)]
	bool m_HideLootVolumes;
	
	[Attribute("false", UIWidgets.CheckBox)]
	bool m_HideLootContainers;

	[ButtonAttribute("OK")]
	void OkButton()	{}
	
	override void Run()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		
		if (! worldEditor || worldEditor.IsPrefabEditMode()) return;
		
		WorldEditorAPI api = worldEditor.GetApi();
		
		if (! api) return;

		BaseWorld world = api.GetWorld();
		
		if (! world) return;
		
		if (! Workbench.ScriptDialog("Configure loot volume highlighting", "", this)) return;

		if (! api.BeginEntityAction()) return;

		Print("--- Start highlighting missing loot volumes");

		// Progress dialog
		int approximateCount = api.GetEditorEntityCount();
		WBProgressDialog progress = new WBProgressDialog("Highlighting", worldEditor);

		EditorEntityIterator iter(api);
		IEntitySource src = iter.GetNext();
		
		while (src)
		{
			ProcessEntity(api, src);
			
			src = iter.GetNext();

			progress.SetProgress(iter.GetCurrentIdx() / approximateCount);
		}

		api.EndEntityAction();

		Print("--- Done highlighting missing loot volumes");
	}
	
	private void ProcessEntity(WorldEditorAPI api, IEntitySource src)
	{
		int childCount = src.GetNumChildren();
		
		if (src.GetClassName() == "Z_LootVolumeEntity")
		{
			if (m_HideLootVolumes)
			{
				Z_LootVolumeEntity vol = Z_LootVolumeEntity.Cast(api.SourceToEntity(src));
				
				if (! vol.m_Categories.IsEmpty() && ! vol.m_Locations.IsEmpty())
				{
					SetEntityVisibility(api, src, ! m_HideLootVolumes);
				}
			}
			else
			{
				SetEntityVisibility(api, src, ! m_HideLootVolumes);
			}
		}
		else if (src.GetClassName() == "Z_LootContainerEntity")
		{
			SetEntityVisibility(api, src, ! m_HideLootContainers);
		}
		else
		{
			for (int i = 0; i < childCount; ++i)
			{
				ProcessEntity(api, src.GetChild(i));
			}
		}
	}
	
	private void SetEntityVisibility(WorldEditorAPI api, IEntitySource src, bool visible)
	{
		IEntitySource parent = src.GetParent();
		
		if (! parent)
		{
			Print("Loot volume or container without parent", LogLevel.ERROR);
			
			return;
		}
		
		IEntity ent = api.SourceToEntity(parent);
		
		if (! ent) return;
		
		api.SetEntityVisible(ent, visible, false);
	}
};
