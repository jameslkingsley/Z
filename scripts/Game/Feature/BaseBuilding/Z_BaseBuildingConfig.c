[BaseContainerProps(configRoot: true)]
class Z_BaseBuildingConfig
{
	[Attribute(defvalue: "{69866F7F642722D4}Prefabs/BaseBuilding/Z_SmallConstructionTool.et", uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Construction tool", params: "et")]
	ResourceName m_ConstructionTool;
	
	[Attribute(defvalue: "0 -0.010 0", uiwidget: UIWidgets.Coords, desc: "Construction tool transform offset")]
	vector m_ConstructionToolOffset;
	
	[Attribute(defvalue: "0 90 0", uiwidget: UIWidgets.Coords, desc: "Construction tool yaw pitch roll")]
	vector m_ConstructionToolYawPitchRoll;
	
	[Attribute("", UIWidgets.Object, category: "Buildables")]
	ref array<ref Z_Buildable> m_Buildables;
	
	ResourceName GetConstructionToolResource()
	{
		return m_ConstructionTool;
	}
	
	vector GetConstructionToolOffset()
	{
		return m_ConstructionToolOffset;
	}
	
	vector GetConstructionToolYawPitchRoll()
	{
		return m_ConstructionToolYawPitchRoll;
	}
	
	ref array<ref Z_Buildable> GetBuildables()
	{
		return m_Buildables;
	}
}
