// Project Chaos - Chaos puanlama tuning Data Asset (06 §4-5, tüm sayılar burada).

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ChaosScoringTuning.generated.h"

/** Tek bir tier: LinkCount eşiği → çarpan + etiket (06 §5). */
USTRUCT(BlueprintType)
struct FChaosTier
{
	GENERATED_BODY()

	/** Minimum chain links (player-hits) needed to reach this tier. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tier")
	int32 MinLinks = 0;

	/** Multiplier applied to the chain's whole raw score when it reaches this tier (1.0 = no bonus). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tier")
	float Bonus = 1.f;

	/** Text shown in the combo popup for this tier (e.g. "MEGA CHAOS"). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tier")
	FText Label;
};

/**
 * Chaos skorunun TÜM sayıları. Kafadan atılan değerler — playtest'le kalibre edilecek,
 * o yüzden data'da (hardcode değil). Atanmazsa ChainComponent fallback'e düşer.
 */
UCLASS(BlueprintType)
class PROJECTCHAOS_API UChaosScoringTuning : public UDataAsset
{
	GENERATED_BODY()

public:
	UChaosScoringTuning();

	// Points for one player-hit = PlayerHitBase + StaggerGivenPoints + (EscalationPerLink * LinkCount).

	/** Base score for hitting any player. The flat reward for each hit in a chain. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Points")
	int32 PlayerHitBase = 5;

	/** Extra score for knocking the victim into stagger (added on top of PlayerHitBase). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Points")
	int32 StaggerGivenPoints = 5;

	/** Bonus per chain link: longer chains score more. Hit #N earns this * N extra.
	 *  (1st hit +3, 2nd +6, 3rd +9 ... when this = 3.) Raise it to reward long combos harder. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Points")
	int32 EscalationPerLink = 3;

	/** Combo tiers. When a chain reaches a link threshold it gets a score multiplier + a label
	 *  for the popup (COMBO / BIG CHAOS / MEGA CHAOS). Keep ordered by MinLinks ascending. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tiers")
	TArray<FChaosTier> Tiers;

	/** LinkCount → tier bonus çarpanı (eşiği geçen en yüksek tier). */
	float GetTierBonus(int32 LinkCount) const;

	/** LinkCount → tier etiketi (cue için). */
	FText GetTierLabel(int32 LinkCount) const;
};
