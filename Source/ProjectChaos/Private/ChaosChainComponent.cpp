// Project Chaos - ChainID combo brain (06_Chaos_Scoring). Phase 5.1: iskelet + yaşam döngüsü.

#include "ChaosChainComponent.h"

#include "ChaosEventMessage.h"
#include "ChaosGameplayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"

DEFINE_LOG_CATEGORY_STATIC(LogChaosChain, Log, All);

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
}

void UChaosChainComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SkillUsedListener.Unregister();
	RecoveredListener.Unregister();
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

	// 5.1: puan YOK. Sadece kapanışı logla. (Puan 5.3'te Owner'a yazılır.)
	UE_LOG(LogChaosChain, Log, TEXT("[CHAIN] KAPANDI ID=%d  Owner=%s  (aktif zincir: %d)"),
		ChainID, Chain->Owner ? *Chain->Owner->GetPlayerName() : TEXT("?"), ActiveChains.Num() - 1);

	ActiveChains.Remove(ChainID);
}
