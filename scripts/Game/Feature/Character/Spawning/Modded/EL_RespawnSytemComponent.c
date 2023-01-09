modded class EL_RespawnSytemComponent : SCR_RespawnSystemComponent
{
	protected override GenericEntity RequestSpawn(int playerId)
	{
		GenericEntity player = super.RequestSpawn(playerId);
		
		FactionAffiliationComponent affiliationComp = FactionAffiliationComponent.Cast(player.FindComponent(FactionAffiliationComponent));
		
		if (affiliationComp)
		{
			Faction faction = affiliationComp.GetAffiliatedFaction();
			
			FactionManager factionManager = GetGame().GetFactionManager();
			
			if (! factionManager)
			{
				Print("Missing FactionManager", LogLevel.ERROR);
				
				return player;
			}
			
			int index = factionManager.GetFactionIndex(faction);
			
			if (faction)
				DoSetPlayerFaction(playerId, index);
		}
		else
		{
			Print("Player has no FactionAffiliationComponent", LogLevel.ERROR);
		}
		
		return player;
	}
}
