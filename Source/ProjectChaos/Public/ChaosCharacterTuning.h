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
	/** Yatay kuvvete oranla eklenecek yukarı bileşen (0.5 = yarısı kadar yukarı). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Launch")
	float UpStrengthRatio = 0.5f;

	/** Sersem karakter başka birine çarpınca ona uygulanan kuvvet. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Launch")
	float TransferForce = 1000.f;

	/** Stagger süresi (sn); sonunda otomatik recovery. Birikimli (kalan + bu). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stagger")
	float StaggerDuration = 1.8f;
};
