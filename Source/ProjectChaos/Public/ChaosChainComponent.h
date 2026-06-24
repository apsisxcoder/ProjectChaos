// Project Chaos - ChainID combo brain (06_Chaos_Scoring). Phase 5.1: iskelet + yaşam döngüsü.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "ChaosChainComponent.generated.h"

class APlayerState;
struct FChaosEventMessage;

/**
 * Bir kaos zinciri (combo). 5.1'de minimal: sadece sahip + aktif sersemler.
 * Link/puan/eskalasyon 5.2-5.3'te eklenir.
 */
USTRUCT()
struct FChaosChain
{
	GENERATED_BODY()

	int32 ChainID = 0;
	TObjectPtr<APlayerState> Owner = nullptr;   // zinciri açan (skill sahibi)
	TSet<TObjectPtr<APlayerState>> ActiveStaggered; // bu zincire ait HÂLÂ sersem olanlar
	float StartTime = 0.f;
};

/**
 * ChaosSubsystem'in BEYNİ. GameState'e takılır (AChaosGameState), server-authoritative.
 * Tüm oyuncuları gören merkez — combo cross-player olduğu için tek noktada.
 * Olay bazlı, per-frame tick YOK (05/06 ile tutarlı).
 *
 * Mimari notu: WorldSubsystem multiplayer'da replike olmaz → GameState doğru ev
 * (araştırma teyitli; doküman 06 da "GameState tarafında" diyor).
 */
UCLASS(ClassGroup = (Chaos), meta = (BlueprintSpawnableComponent))
class PROJECTCHAOS_API UChaosChainComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UChaosChainComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool HasServerAuthority() const;

	//~ Tag omurgası dinleyicileri (GameplayMessageSubsystem) -------------------
	void HandleSkillUsed(FGameplayTag Channel, const FChaosEventMessage& Message);
	void HandleStaggerRecovered(FGameplayTag Channel, const FChaosEventMessage& Message);

	FGameplayMessageListenerHandle SkillUsedListener;
	FGameplayMessageListenerHandle RecoveredListener;

	//~ Çekirdek zincir mantığı ------------------------------------------------
	/** YENİ ChainID aç, owner'ı sersem üye yap. */
	void OnSkillUsed(APlayerState* Player);

	/** Oyuncu ayıldı → ait olduğu zincirden çıkar, zincir boşaldıysa kapat. */
	void OnStaggerRecovered(APlayerState* Player);

	/** Oyuncuyu mevcut zincirinden çıkar (varsa), o zincir boşaldıysa kapat. */
	void RemovePlayerFromCurrentChain(APlayerState* Player);

	void CloseChain(int32 ChainID);

	UPROPERTY()
	TMap<int32, FChaosChain> ActiveChains;

	UPROPERTY()
	TMap<TObjectPtr<APlayerState>, int32> PlayerCurrentChain;

	int32 NextChainID = 1;
};
