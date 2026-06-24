// Project Chaos - Tag event omurgasının mesaj yükü (Lyra FVerbMessage deseni).

#pragma once

#include "CoreMinimal.h"
#include "ChaosEventMessage.generated.h"

class APlayerState;

/**
 * Chaos olaylarının ortak mesaj yapısı. GameplayMessageSubsystem ile yayınlanır.
 * Instigator = olayı üreten; Target = (varsa) etkilenen (impact'te kurban).
 */
USTRUCT(BlueprintType)
struct FChaosEventMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Chaos")
	TObjectPtr<APlayerState> Instigator = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Chaos")
	TObjectPtr<APlayerState> Target = nullptr;
};
