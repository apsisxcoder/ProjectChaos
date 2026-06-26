// Project Chaos - ChainID combo brain (06_Chaos_Scoring). Phase 5.1: iskelet + yaşam döngüsü.

#include "ChaosChainComponent.h"

#include "ChaosEventMessage.h"
#include "ChaosGameplayTags.h"
#include "ChaosPlayerState.h"
#include "ChaosScoringTuning.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"

DEFINE_LOG_CATEGORY_STATIC(LogChaosChain, Log, All);

// Puanlama: Tuning varsa ondan, yoksa standart varsayılan (06 §4-5 değerleri).
int32 UChaosChainComponent::ScorePlayerHit(int32 LinkCount) const
{
	const int32 Base       = Tuning ? Tuning->PlayerHitBase      : 5;
	const int32 Stagger    = Tuning ? Tuning->StaggerGivenPoints : 5;
	const int32 Escalation = Tuning ? Tuning->EscalationPerLink  : 3;
	return Base + Stagger + (Escalation * LinkCount);
}

float UChaosChainComponent::TierBonus(int32 LinkCount) const
{
	if (Tuning)
	{
		return Tuning->GetTierBonus(LinkCount);
	}
	// Fallback (DA varsayılanlarıyla aynı): 4+ →3.0, 3 →1.5, 2 →1.0
	if (LinkCount >= 4) return 3.0f;
	if (LinkCount >= 3) return 1.5f;
	return 1.0f;
}

UChaosChainComponent::UChaosChainComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // olay bazlı, tick yok
	SetIsReplicatedByDefault(false);           // 5.1: saf server mantığı, henüz replike yok
}

void UChaosChainComponent::BeginPlay()
{
	Super::BeginPlay();

	// Sadece server dinler (chaos mantığı server-authoritative).
	if (!HasServerAuthority())
	{
		return;
	}

	UGameplayMessageSubsystem& Bus = UGameplayMessageSubsystem::Get(this);
	SkillUsedListener = Bus.RegisterListener(TAG_Event_Skill_Used, this, &UChaosChainComponent::HandleSkillUsed);
	RecoveredListener = Bus.RegisterListener(TAG_Event_Penalty_Recovered, this, &UChaosChainComponent::HandleStaggerRecovered);
	ImpactListener = Bus.RegisterListener(TAG_Event_Impact_PlayerHit, this, &UChaosChainComponent::HandleImpact);
}

void UChaosChainComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SkillUsedListener.Unregister();
	RecoveredListener.Unregister();
	ImpactListener.Unregister();
	Super::EndPlay(EndPlayReason);
}

void UChaosChainComponent::HandleSkillUsed(FGameplayTag Channel, const FChaosEventMessage& Message)
{
	OnSkillUsed(Message.Instigator);
}

void UChaosChainComponent::HandleStaggerRecovered(FGameplayTag Channel, const FChaosEventMessage& Message)
{
	OnStaggerRecovered(Message.Instigator);
}

void UChaosChainComponent::HandleImpact(FGameplayTag Channel, const FChaosEventMessage& Message)
{
	OnImpact(Message.Instigator, Message.Target);
}

bool UChaosChainComponent::HasServerAuthority() const
{
	const AActor* Owner = GetOwner();
	return Owner && Owner->HasAuthority();
}

void UChaosChainComponent::OnSkillUsed(APlayerState* Player)
{
	if (!HasServerAuthority() || !Player)
	{
		return;
	}

	// Doküman 06 §2 kural 1: skill → YENİ ChainID. Oyuncu zaten bir zincirdeyse
	// önce oradan çıkar (re-tag): tek zincire aittir.
	RemovePlayerFromCurrentChain(Player);

	const int32 NewID = NextChainID++;
	FChaosChain Chain;
	Chain.ChainID = NewID;
	Chain.Owner = Player;
	Chain.StartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	Chain.ActiveStaggered.Add(Player); // skill sahibi de bu zincirde sersem

	ActiveChains.Add(NewID, Chain);
	PlayerCurrentChain.Add(Player, NewID);

	UE_LOG(LogChaosChain, Log, TEXT("[CHAIN] AÇILDI  ID=%d  Owner=%s  (aktif zincir: %d)"),
		NewID, *Player->GetPlayerName(), ActiveChains.Num());
}

void UChaosChainComponent::OnImpact(APlayerState* Instigator, APlayerState* Victim)
{
	if (!HasServerAuthority() || !Instigator || !Victim || Instigator == Victim)
	{
		return;
	}

	// Instigator hangi zincirde? (ChainID'yi DEĞERLE kopyala — sonra map'leri
	// değiştireceğiz, pointer dangling olmasın.)
	const int32* FoundChainID = PlayerCurrentChain.Find(Instigator);
	if (!FoundChainID)
	{
		// Sersem ama zincirsiz instigator (beklenmez). Orphan — şimdilik atla.
		UE_LOG(LogChaosChain, Warning, TEXT("[CHAIN] impact atlandı: %s zincirsiz"),
			*Instigator->GetPlayerName());
		return;
	}
	const int32 ChainID = *FoundChainID;

	// Re-tag: Victim başka bir zincirdeyse oradan çıkar (06 §2 kural 3). Bu, map'leri
	// değiştirebilir/zincir kapatabilir — bu yüzden Chain'i BUNDAN SONRA bul.
	RemovePlayerFromCurrentChain(Victim);

	FChaosChain* Chain = ActiveChains.Find(ChainID);
	if (!Chain)
	{
		return; // teorik: instigator'ın zinciri arada kapanmış
	}

	// Victim'i Instigator'ın zincirine ekle → zincir yayıldı.
	Chain->ActiveStaggered.Add(Victim);
	PlayerCurrentChain.Add(Victim, ChainID);
	Chain->LinkCount++;

	// Puan birikimi (06 §4): eskalasyonlu PlayerHit puanı.
	const int32 Points = ScorePlayerHit(Chain->LinkCount);
	Chain->AccumulatedPoints += Points;

	FChainLink Link;
	Link.Instigator = Instigator;
	Link.Victim = Victim;
	Link.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	Chain->Links.Add(Link);

	UE_LOG(LogChaosChain, Log, TEXT("[CHAIN] LINK   ID=%d  %s -> %s  LinkCount=%d  +%d puan (ham: %d)"),
		ChainID, *Instigator->GetPlayerName(), *Victim->GetPlayerName(),
		Chain->LinkCount, Points, Chain->AccumulatedPoints);
}

void UChaosChainComponent::OnStaggerRecovered(APlayerState* Player)
{
	if (!HasServerAuthority() || !Player)
	{
		return;
	}
	RemovePlayerFromCurrentChain(Player);
}

void UChaosChainComponent::RemovePlayerFromCurrentChain(APlayerState* Player)
{
	const int32* FoundID = PlayerCurrentChain.Find(Player);
	if (!FoundID)
	{
		return;
	}
	const int32 ChainID = *FoundID;
	PlayerCurrentChain.Remove(Player);

	if (FChaosChain* Chain = ActiveChains.Find(ChainID))
	{
		Chain->ActiveStaggered.Remove(Player);

		UE_LOG(LogChaosChain, Log, TEXT("[CHAIN] %s zincirden çıktı  ID=%d  (kalan sersem: %d)"),
			*Player->GetPlayerName(), ChainID, Chain->ActiveStaggered.Num());

		// Doküman 06 §2 kural 4: kendi sersemleri kalmayınca zincir kapanır.
		if (Chain->ActiveStaggered.Num() == 0)
		{
			CloseChain(ChainID);
		}
	}
}

void UChaosChainComponent::CloseChain(int32 ChainID)
{
	const FChaosChain* Chain = ActiveChains.Find(ChainID);
	if (!Chain)
	{
		return;
	}

	// Final skor (06 §3-6): ham birikim × tier bonus × hero çarpan (şimdilik 1.0).
	const float HeroMultiplier = 1.0f; // TODO: DA_HeroProfile.ComboMultiplier
	const float Bonus = TierBonus(Chain->LinkCount);
	const int32 Final = FMath::RoundToInt(Chain->AccumulatedPoints * Bonus * HeroMultiplier);
	const FText TierLabel = Tuning ? Tuning->GetTierLabel(Chain->LinkCount) : FText::GetEmpty();

	// Owner'ın PlayerState'ine yaz (replike → herkes görür).
	if (AChaosPlayerState* PS = Cast<AChaosPlayerState>(Chain->Owner))
	{
		PS->AddChaosScore(Final);
	}

	UE_LOG(LogChaosChain, Log, TEXT("[CHAIN] KAPANDI ID=%d  Owner=%s  %s  ham=%d ×%.1f = %d puan  (aktif zincir: %d)"),
		ChainID, Chain->Owner ? *Chain->Owner->GetPlayerName() : TEXT("?"),
		*TierLabel.ToString(), Chain->AccumulatedPoints, Bonus, Final, ActiveChains.Num() - 1);

	ActiveChains.Remove(ChainID);
}
