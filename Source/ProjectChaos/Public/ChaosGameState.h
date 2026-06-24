// Project Chaos - GameState: ChaosSubsystem'in (ChainID beyni) evi.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ChaosGameState.generated.h"

class UChaosChainComponent;

/**
 * ChainID beyni (UChaosChainComponent) burada yaşar. Doküman 06: "ChaosSubsystem
 * GameState tarafında, server-authoritative." GameState tüm client'lara replike,
 * BeginPlay'de garanti hazır → combo/skor için doğru ev.
 */
UCLASS()
class PROJECTCHAOS_API AChaosGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AChaosGameState();

	UChaosChainComponent* GetChainComponent() const { return ChainComp; }

private:
	UPROPERTY(VisibleAnywhere, Category = "Chaos")
	TObjectPtr<UChaosChainComponent> ChainComp;
};
