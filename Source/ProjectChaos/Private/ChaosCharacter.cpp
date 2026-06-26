// Project Chaos - Phase 0 launch/knockback sync prototype.

#include "ChaosCharacter.h"

#include "ChaosCharacterTuning.h"
#include "ChaosEventMessage.h"
#include "ChaosGameplayTags.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

// Fallback'li tuning okuyucular: Tuning atanmamışsa standart varsayılan.
float AChaosCharacter::GetUpStrengthRatio() const { return Tuning ? Tuning->UpStrengthRatio : 0.5f; }
float AChaosCharacter::GetTransferForce() const   { return Tuning ? Tuning->TransferForce   : 1000.f; }
float AChaosCharacter::GetStaggerDuration() const { return Tuning ? Tuning->StaggerDuration : 1.8f; }

AChaosCharacter::AChaosCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// Çarpışma aktarımını server'da yakalamak için kapsül hit event'i.
	GetCapsuleComponent()->SetNotifyRigidBodyCollision(true);
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AChaosCharacter::OnCapsuleHit);
}

void AChaosCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AChaosCharacter, bStaggered);
}

//~ Primitive'ler ============================================================

void AChaosCharacter::Server_LaunchSelf_Implementation(FVector Direction, float Force)
{
	// SERVER. Verilen yön + yukarı bileşen.
	const FVector Dir = Direction.GetSafeNormal();
	const FVector Velocity = (Dir * Force) + (FVector::UpVector * Force * GetUpStrengthRatio());
	DoLaunch(Velocity);

	// Skill = yeni ChainID başlatıcı (06 §2 kural 1). Tag omurgasına yayınla;
	// ChaosSubsystem dinler (decoupled — doğrudan çağrı yok).
	FChaosEventMessage Msg;
	Msg.Instigator = GetPlayerState();
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(TAG_Event_Skill_Used, Msg);
}

void AChaosCharacter::Server_ApplyKnockback_Implementation(AChaosCharacter* Target, float Force)
{
	if (!Target || Target == this)
	{
		return;
	}
	const FVector Dir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	const FVector Velocity = (Dir * Force) + (FVector::UpVector * Force * GetUpStrengthRatio());
	Target->DoLaunch(Velocity);
}

void AChaosCharacter::Server_SetStaggered_Implementation(bool bNewStaggered)
{
	SetStaggeredInternal(bNewStaggered);
}

//~ Çekirdek (SERVER) ========================================================

void AChaosCharacter::DoLaunch(const FVector& Velocity)
{
	if (!HasAuthority())
	{
		return;
	}

	// Kapsül seviyesinde itme — gameplay sonucu. CMC bunu client'lara replike eder.
	LaunchCharacter(Velocity, /*bXYOverride=*/true, /*bZOverride=*/true);

	SetStaggeredInternal(true);
}

void AChaosCharacter::SetStaggeredInternal(bool bNewStaggered)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bNewStaggered)
	{
		// BİRİKİM (§2.5): yeni süre = kalan + StaggerDuration. Ne kadar çok yerse o kadar
		// uzun sersem kalır; süre timer ile delta-time azalır (per-frame tick yok).
		float Remaining = bStaggered
			? GetWorldTimerManager().GetTimerRemaining(StaggerTimerHandle) : 0.f;
		if (Remaining < 0.f)
		{
			Remaining = 0.f;
		}

		const bool bWasStaggered = bStaggered;
		bStaggered = true;
		// Görseli yalnız ilk girişte tetikle; birikimde (true→true) ragdoll'u yeniden açma.
		if (!bWasStaggered)
		{
			OnRep_Staggered(); // listen host kendi penceresinde de görsün
		}

		GetWorldTimerManager().SetTimer(StaggerTimerHandle, this,
			&AChaosCharacter::RecoverFromStagger, Remaining + GetStaggerDuration(), false);
	}
	else
	{
		bStaggered = false;
		OnRep_Staggered();
		GetWorldTimerManager().ClearTimer(StaggerTimerHandle);
	}
}

void AChaosCharacter::RecoverFromStagger()
{
	SetStaggeredInternal(false);

	// Oyuncu ayıldı → tag omurgasına yayınla; ChaosSubsystem zincirden çıkarır.
	FChaosEventMessage Msg;
	Msg.Instigator = GetPlayerState();
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(TAG_Event_Penalty_Recovered, Msg);
}

void AChaosCharacter::OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority() || !bStaggered)
	{
		return; // sadece server + sadece sersem karakter aktarır
	}

	AChaosCharacter* Other = Cast<AChaosCharacter>(OtherActor);
	if (!Other || Other == this || Other->bStaggered)
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastHitTime < 0.2f)
	{
		return; // spam koruması
	}
	LastHitTime = Now;

	// Bizden ötekine doğru fırlat (Server_ApplyKnockback ile aynı mantık).
	const FVector Dir = (Other->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	const FVector Velocity = (Dir * GetTransferForce()) + (FVector::UpVector * GetTransferForce() * GetUpStrengthRatio());
	Other->DoLaunch(Velocity);

	// İstemsiz çarpışma = zinciri yay (06 §2 kural 2). Tag omurgasına yayınla;
	// ChaosSubsystem kurbanı bizim zincirimize ekler (re-tag + LinkCount).
	FChaosEventMessage Msg;
	Msg.Instigator = GetPlayerState();
	Msg.Target = Other->GetPlayerState();
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(TAG_Event_Impact_PlayerHit, Msg);
}

void AChaosCharacter::OnRep_Staggered()
{
	// Görsel tepki tamamen BP'de — C++ mesh fiziğine dokunmaz.
	OnStaggerChanged(bStaggered);
}
