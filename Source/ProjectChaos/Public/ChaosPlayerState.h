// Project Chaos - PlayerState: replike Chaos skoru (06 §0).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ChaosPlayerState.generated.h"

/**
 * Oyuncunun Chaos skorunu taşır. PlayerState doğru ev: pawn ölse/respawn olsa da
 * kalır, herkese replike (scoreboard). Skor SERVER'da yazılır, client'lara replike.
 */
UCLASS()
class PROJECTCHAOS_API AChaosPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** SERVER: skora ekle (zincir kapanınca Owner'a yazılır). */
	void AddChaosScore(int32 Delta);

	UFUNCTION(BlueprintPure, Category = "Chaos")
	int32 GetChaosScore() const { return ChaosScore; }

protected:
	UPROPERTY(ReplicatedUsing = OnRep_ChaosScore, BlueprintReadOnly, Category = "Chaos")
	int32 ChaosScore = 0;

	UFUNCTION()
	void OnRep_ChaosScore();

	/** Skor değişince (server + client) — BP'de UI/scoreboard güncelle. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chaos")
	void OnChaosScoreChanged(int32 NewScore);
};
