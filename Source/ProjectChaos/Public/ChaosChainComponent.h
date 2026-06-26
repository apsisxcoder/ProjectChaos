// Project Chaos - ChainID combo brain (06_Chaos_Scoring). Phase 5.1: iskelet + yaşam döngüsü.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "ChaosChainComponent.generated.h"

class APlayerState;
struct FChaosEventMessage;

/** Zincirdeki tek bir halka: kim kime çarptı (06 §2 veri yapısı). */
USTRUCT()
struct FChainLink
{
	GENERATED_BODY()

	TObjectPtr<APlayerState> Instigator = nullptr; // bu halkayı üreten
	TObjectPtr<APlayerState> Victim = nullptr;     // çarpılan
	float Timestamp = 0.f;
};

/**
 * Bir kaos zinciri (combo). 5.2: link + LinkCount eklendi. Puan/tier 5.3'te.
 */
USTRUCT()
struct FChaosChain
{
	GENERATED_BODY()

	int32 ChainID = 0;
	TObjectPtr<APlayerState> Owner = nullptr;   // zinciri açan (skill sahibi)
	TSet<TObjectPtr<APlayerState>> ActiveStaggered; // bu zincire ait HÂLÂ sersem olanlar
	TArray<FChainLink> Links;                   // üretilen halkalar
	int32 LinkCount = 0;                         // player-hit sayısı (eskalasyon)
	int32 AccumulatedPoints = 0;                 // birikmiş ham puan (kapanışta × tier)
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
	void HandleImpact(FGameplayTag Channel, const FChaosEventMessage& Message);

	FGameplayMessageListenerHandle SkillUsedListener;
	FGameplayMessageListenerHandle RecoveredListener;
	FGameplayMessageListenerHandle ImpactListener;

	//~ Çekirdek zincir mantığı ------------------------------------------------
	/** YENİ ChainID aç, owner'ı sersem üye yap. */
	void OnSkillUsed(APlayerState* Player);

	/** Oyuncu ayıldı → ait olduğu zincirden çıkar, zincir boşaldıysa kapat. */
	void OnStaggerRecovered(APlayerState* Player);

	/** Sersem Instigator taze Victim'e çarptı → Victim'i Instigator'ın zincirine ekle (yay). */
	void OnImpact(APlayerState* Instigator, APlayerState* Victim);

	/** Oyuncuyu mevcut zincirinden çıkar (varsa), o zincir boşaldıysa kapat. */
	void RemovePlayerFromCurrentChain(APlayerState* Player);

	void CloseChain(int32 ChainID);

	//~ Puanlama (fallback'li; Tuning atanmazsa standart varsayılan) -----------
	int32 ScorePlayerHit(int32 LinkCount) const;  // 06 §4: Base + StaggerGiven + Escalation×LinkCount
	float TierBonus(int32 LinkCount) const;        // 06 §5

	/** Skor sayıları (tier, puan). Boşsa fallback. BP_ChaosGameState'te atanır. */
	UPROPERTY(EditDefaultsOnly, Category = "Chaos")
	TObjectPtr<class UChaosScoringTuning> Tuning;

	UPROPERTY()
	TMap<int32, FChaosChain> ActiveChains;

	UPROPERTY()
	TMap<TObjectPtr<APlayerState>, int32> PlayerCurrentChain;

	int32 NextChainID = 1;
};
