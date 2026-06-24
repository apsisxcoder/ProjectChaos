// Project Chaos - Phase 0 launch/knockback sync prototype.

#include "ChaosCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

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
	const FVector Velocity = (Dir * Force) + (FVector::UpVector * Force * UpStrengthRatio);
	DoLaunch(Velocity);
}

void AChaosCharacter::Server_ApplyKnockback_Implementation(AChaosCharacter* Target, float Force)
{
	if (!Target || Target == this)
	{
		return;
	}
	const FVector Dir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	const FVector Velocity = (Dir * Force) + (FVector::UpVector * Force * UpStrengthRatio);
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

	bStaggered = bNewStaggered;
	OnRep_Staggered(); // listen host kendi penceresinde de görsün

	GetWorldTimerManager().ClearTimer(StaggerTimerHandle);
	if (bNewStaggered)
	{
		GetWorldTimerManager().SetTimer(StaggerTimerHandle, this,
			&AChaosCharacter::RecoverFromStagger, StaggerDuration, false);
	}
}

void AChaosCharacter::RecoverFromStagger()
{
	SetStaggeredInternal(false);
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
	const FVector Velocity = (Dir * TransferForce) + (FVector::UpVector * TransferForce * UpStrengthRatio);
	Other->DoLaunch(Velocity);
}

void AChaosCharacter::OnRep_Staggered()
{
	// Görsel tepki tamamen BP'de — C++ mesh fiziğine dokunmaz.
	OnStaggerChanged(bStaggered);
}
