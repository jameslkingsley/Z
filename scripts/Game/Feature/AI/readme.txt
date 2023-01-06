AI

- Split map up into regions (possibly re-use loot regions?)
- Each region defines AI characteristics (difficulty, what tier loot, attrition etc)
- Each region contains various entities that define activities that can be done in this area
	- A radius attribute can help randomise where things spawn (SCR_WorldTools.FindEmptyTerrainPosition)
	- A forest might have activities such as "patrol"
	- A military base might have "patrol" and "defend"
- Spawning works similar to loot
	- Players get near a region and it then:
		- checks if there's already enough AI in the region
		- if not, spawn some in but far enough away so players dont see
		- spawn position is chosen based on heat map
			- encounters are persisted
				- on AI death / player kill by AI:
					- store an encounter event containing the location and attrition impact
					- encounter events expire after a while (so heat map doesn't become stale)
					- aggregate these encounters into a centroid coordinate
					- these form the heat map (hotter areas of denser encounters and attrition impact)
			- spawns are higher chance on hot areas, lower on cold
		- amount spawned is based on how many players in region and attrition

attrition:

- attrition governs how much the AI can achieve
- player actions can increase/decrease AI attrition
	- destroy patrol = decrease
	- die in military base (giving them your gear) = increase
- wouldn't be that complex but need good event handling on AI groups
- essentially need to know who killed who and adjust attrition based on that
- AI would have a constant base supply of attrition per hour
- AI have a max attrition level
- when AI have lowest attrition possible, it'll recover naturally based on server pop

player skills:

- similar to tarkov
- metabolism (slower metabolism, faster energy recovery etc)
- resistance to illness (unknown atm)
- stamina (thresholds for encumbrance, carry capacity, drain/s)
- weapon handling (sway, recoil, hold breath duration)
- skills take a lot of time to level up - some number of weeks to achieve max skills
- all skills reset to zero upon death
