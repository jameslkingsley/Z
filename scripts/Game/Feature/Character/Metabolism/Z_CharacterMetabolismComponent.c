class Z_EnergyModifier
{
	// Persisted props
	// These change each tick
	float m_Amount;
	int m_DurationInSeconds;
	
	void Z_EnergyModifier(float amount, int durationInSeconds)
	{
		m_Amount = amount;
		m_DurationInSeconds = durationInSeconds;
	}
	
	float Consume()
	{
		if (m_Amount <= 0)
		{
			return 0;
		}
		
		float perSecond = m_Amount / m_DurationInSeconds;
		
		m_Amount -= perSecond;
		m_DurationInSeconds--;
		
		return perSecond;
	}
}

[ComponentEditorProps(category: "Z/Components", description: "")]
class Z_CharacterMetabolismComponentClass: ScriptComponentClass
{
};

class Z_CharacterMetabolismComponent : ScriptComponent
{
	[Attribute("10000", UIWidgets.Auto, "Maximum energy the character can attain")]
	int m_EnergyMaximum;
	
	[Attribute("1100", UIWidgets.Auto, "Default energy the character spawns with")]
	int m_EnergyDefault;
	
	[Attribute("1", UIWidgets.Auto, "Base energy drain per second")]
	float m_BaseEnergyDrainPerSecond;
	
	[Attribute("0.5", UIWidgets.Auto, "Percentage to apply to the base energy drain when character movement is slowest")]
	float m_EnergyMovementPenaltyMinimum;
	
	[Attribute("3.5", UIWidgets.Auto, "Percentage to apply to the base energy drain when character movement is fastest")]
	float m_EnergyMovementPenaltyMaximum;
	
	[Attribute("0.1", UIWidgets.Auto, "Percentage of maximum energy where character can fall unconscious")]
	float m_UnconsciousThreshold;
	
	// TODO Persisted props below
	
	float m_Energy;
	
	int m_LastUnconsciousAtInSeconds;
	
	ref array<ref Z_EnergyModifier> m_EnergyModifiers = new ref array<ref Z_EnergyModifier>();
	
	private CharacterControllerComponent m_CharacterController;
	
	private SCR_CharacterDamageManagerComponent m_DamageManager;
	
	override protected void OnPostInit(IEntity owner)
	{
		/*
		m_CharacterController = CharacterControllerComponent.Cast(owner.FindComponent(CharacterControllerComponent));
		
		if (! m_CharacterController)
		{
			Debug.Error("Metabolism component owner must have a character controller component");
			
			return;
		}
		
		m_DamageManager = SCR_CharacterDamageManagerComponent.Cast(owner.FindComponent(SCR_CharacterDamageManagerComponent));
		
		if (! m_Energy)
		{
			m_Energy = m_EnergyDefault;
		}
		
		// TODO Check if uncon and if should be con
		
		GetGame().GetCallqueue().CallLater(UpdateMetabolism, 1000, true);
		*/
	}
	
	void AddModifier(float amount, int durationInSeconds)
	{
		m_EnergyModifiers.Insert(new ref Z_EnergyModifier(amount, durationInSeconds));
	}
	
	void UpdateMetabolism()
	{
		m_Energy += CalculateEnergyDrain();
		
		if (m_Energy <= 0)
		{
			// Kill character
			return;
		}
		
		// Print("Current energy: " + m_Energy, LogLevel.WARNING);
	}
	
	void UpdateCharacterController()
	{
		float energyPercent = Math.InverseLerp(0, m_EnergyMaximum, m_Energy);
		
		// TODO proper character movement controls (cant find code to do this)
	}
	
	bool IsBelowUnconsciousThreshold()
	{
		return GetEnergyCoefficient() <= m_UnconsciousThreshold;
	}
	
	bool IsUnconsciousInCooldown()
	{
		if (! m_LastUnconsciousAtInSeconds) return false;
		
		int timestampInSeconds = System.GetTickCount() / 1000;
		int elapsedTime = timestampInSeconds - m_LastUnconsciousAtInSeconds;
		int cooldown = Math.Lerp(5, 60, GetEnergyCoefficient());
		
		return elapsedTime < cooldown;
	}
	
	int CalculateUnconsciousDuration()
	{
		return Math.Lerp(20, 5, GetEnergyCoefficient()) * 1000;
	}
	
	void SetUnconscious()
	{
		m_CharacterController.SetUnconscious(true);
		
		int duration = CalculateUnconsciousDuration();
		
		GetGame().GetCallqueue().CallLater(UnsetUnconscious, duration);
		
		Print("Going uncon for " + duration / 1000 + " seconds");
	}
	
	void UnsetUnconscious()
	{
		m_CharacterController.SetUnconscious(false);
		
		m_LastUnconsciousAtInSeconds = System.GetTickCount() / 1000;
	}
	
	float GetEnergyCoefficient()
	{
		return Math.InverseLerp(0, m_EnergyMaximum, m_Energy);
	}
	
	float CalculateEnergyDrain()
	{
		float drain = m_BaseEnergyDrainPerSecond * -1;
		
		drain += ConsumeModifiers();
		
		drain -= CalculateMovementPenalty();
		
		return drain;
	}
	
	float CalculateMovementPenalty()
	{
		float moveSpeedPercent = Math.InverseLerp(0, 3, m_CharacterController.GetMovementSpeed());
		
		if (moveSpeedPercent == 0) return 0;
		
		float penaltyPercent = Math.Lerp(m_EnergyMovementPenaltyMinimum, m_EnergyMovementPenaltyMaximum, moveSpeedPercent);
		
		return penaltyPercent * m_BaseEnergyDrainPerSecond;
	}
	
	float ConsumeModifiers()
	{
		float modifiers = 0;
		
		array<Z_EnergyModifier> consumedModifiers = new array<Z_EnergyModifier>();
		
		foreach (Z_EnergyModifier mod : m_EnergyModifiers)
		{
			float amount = mod.Consume();
			
			if (amount == 0)
			{
				consumedModifiers.Insert(mod);
				
				continue;
			}
			
			modifiers += amount;
		}
		
		foreach (Z_EnergyModifier mod : consumedModifiers)
		{
			int index = m_EnergyModifiers.Find(mod);
			
			if (index > -1)
			{
				m_EnergyModifiers.Remove(index);
			}
		}
		
		return modifiers;
	}
}
