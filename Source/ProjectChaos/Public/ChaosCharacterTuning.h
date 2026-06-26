// Project Chaos - Karakter tuning Data Asset (data-driven config, 01 §2.1).

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ChaosCharacterTuning.generated.h"

/**
 * Bir karakterin fizik/stagger ayarları. C++ MANTIĞI değil, CONFIG'i taşır.
 * Yeni karakter = yeni DA (ya da paylaş); değerleri tek yerden değiştir, recompile yok.
 *
 * Karaktere atanmazsa C++ güvenli varsayılanlara düşer (fallback) — asset unutulsa
 * bile oyun çalışır. Buradaki varsayılanlar = fallback ile aynı.
 */
UCLASS(BlueprintType)
class PROJECTCHAOS_API UChaosCharacterTuning : public UDataAsset
{
	GENERATED_BODY()

public:
	/** How much UP force is added relative to forward force (0.5 = half as much up).
	 *  Higher = more of a jump/arc; lower = flatter, more horizontal knockback. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Launch")
	float UpStrengthRatio = 0.5f;

	/** Force pushed onto a fresh victim when a staggered (flying) body hits them.
	 *  This is what spreads the chain. Higher = victims fly further. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Launch")
	float TransferForce = 1000.f;

	/** How long (seconds) a character stays staggered/ragdolled per hit.
	 *  Accumulates: getting hit twice = ~2x this. Higher = longer comedy ragdoll. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stagger")
	float StaggerDuration = 1.8f;
};
