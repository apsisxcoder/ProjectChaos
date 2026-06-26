// Project Chaos - PlayerState: replike Chaos skoru (06 §0).

#include "ChaosPlayerState.h"

#include "Net/UnrealNetwork.h"

void AChaosPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AChaosPlayerState, ChaosScore);
}

void AChaosPlayerState::AddChaosScore(int32 Delta)
{
	if (!HasAuthority() || Delta == 0)
	{
		return;
	}
	ChaosScore += Delta;
	OnRep_ChaosScore(); // RepNotify sadece client'ta tetiklenir → host için elle çağır
}

void AChaosPlayerState::OnRep_ChaosScore()
{
	OnChaosScoreChanged(ChaosScore);
}
