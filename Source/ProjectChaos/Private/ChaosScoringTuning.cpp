// Project Chaos - Chaos puanlama tuning Data Asset (06 §4-5).

#include "ChaosScoringTuning.h"

#define LOCTEXT_NAMESPACE "ChaosScoring"

UChaosScoringTuning::UChaosScoringTuning()
{
	// Varsayılan tier tablosu (06 §5). Fresh asset bile doğru gelir.
	auto MakeTier = [](int32 MinLinks, float Bonus, const FText& Label)
	{
		FChaosTier Tier;
		Tier.MinLinks = MinLinks;
		Tier.Bonus = Bonus;
		Tier.Label = Label;
		return Tier;
	};

	Tiers.Add(MakeTier(2, 1.0f, LOCTEXT("Combo", "COMBO")));
	Tiers.Add(MakeTier(3, 1.5f, LOCTEXT("BigChaos", "BIG CHAOS")));
	Tiers.Add(MakeTier(4, 3.0f, LOCTEXT("MegaChaos", "MEGA CHAOS")));
}

float UChaosScoringTuning::GetTierBonus(int32 LinkCount) const
{
	// Eşiği geçen EN YÜKSEK tier'in bonusu (Tiers artan sırada varsayılır).
	float Bonus = 1.0f;
	for (const FChaosTier& Tier : Tiers)
	{
		if (LinkCount >= Tier.MinLinks)
		{
			Bonus = Tier.Bonus;
		}
	}
	return Bonus;
}

FText UChaosScoringTuning::GetTierLabel(int32 LinkCount) const
{
	FText Label = FText::GetEmpty();
	for (const FChaosTier& Tier : Tiers)
	{
		if (LinkCount >= Tier.MinLinks)
		{
			Label = Tier.Label;
		}
	}
	return Label;
}

#undef LOCTEXT_NAMESPACE
