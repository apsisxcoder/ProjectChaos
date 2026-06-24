// Project Chaos - Phase 0 launch/knockback sync prototype.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ChaosCharacter.generated.h"

/**
 * Phase 0 prototip karakteri.
 *
 * SORUMLULUK BÖLÜŞÜMÜ:
 *  - C++ (bu sınıf) = SADECE otoriter primitive'ler + replicated state.
 *      "Kim nereye savruldu" (kapsül + bStaggered) server'da hesaplanır.
 *  - Blueprint = input bağlama + GÖRSEL tepki (ragdoll/montage).
 *      Ragdoll görselini OnStaggerChanged event'inde SEN BP'de kurarsın
 *      → rebuild gerekmeden iterasyon.
 *
 * Otorite (Guide/_Route 01_Ana_Mimari §11): tüm launch/knockback server'da,
 * CMC kapsülü replike eder. Ragdoll görseli her client'ta lokaldir, gameplay'i
 * ETKİLEMEZ.
 */
UCLASS()
class PROJECTCHAOS_API AChaosCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AChaosCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//~ BP'DEN ÇAĞRILACAK PRIMITIVE'LER ======================================

	/** Karakteri Direction yönünde fırlat (+ yukarı bileşen). Client'tan çağrılınca
	 *  otomatik server'a gider. BP input event'inden çağır. Direction normalize edilir. */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Chaos|Launch")
	void Server_LaunchSelf(FVector Direction, float Force);

	/** Bir hedef karakteri bizden ona doğru fırlat (P1->P2 aktarım testi için elle de çağrılabilir). */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Chaos|Launch")
	void Server_ApplyKnockback(AChaosCharacter* Target, float Force);

	/** Stagger durumunu elle aç/kapat (test/debug için). */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Chaos")
	void Server_SetStaggered(bool bNewStaggered);

	UFUNCTION(BlueprintPure, Category = "Chaos")
	bool IsStaggered() const { return bStaggered; }

	//~ TUNING (BP'de ayarlanır) =============================================

	/** Yatay kuvvete oranla eklenecek yukarı bileşen (0.5 = yarısı kadar yukarı). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos|Tuning")
	float UpStrengthRatio = 0.5f;

	/** Sersem karakter başka birine çarpınca ona uygulanan kuvvet. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos|Tuning")
	float TransferForce = 1000.f;

	/** Stagger süresi (sn); sonunda otomatik recovery. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos|Tuning")
	float StaggerDuration = 1.8f;

protected:
	/** RAGDOLL GÖRSELİNİ BURADA, BP'DE KUR. Her client'ta + host'ta çağrılır.
	 *  C++ hiçbir mesh fiziği uygulamaz; görsel tamamen senin. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chaos")
	void OnStaggerChanged(bool bNewStaggered);

	UPROPERTY(ReplicatedUsing = OnRep_Staggered, BlueprintReadOnly, Category = "Chaos")
	bool bStaggered = false;

	UFUNCTION()
	void OnRep_Staggered();

	/** Sersem karakterin çarpışmasını server'da yakalar → aktarım. */
	UFUNCTION()
	void OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

private:
	/** SERVER: hız uygula + stagger'a sok + recovery timer. */
	void DoLaunch(const FVector& Velocity);

	/** SERVER: durum değiştir, OnRep'i host'ta da tetikle, timer yönet. */
	void SetStaggeredInternal(bool bNewStaggered);

	void RecoverFromStagger();

	FTimerHandle StaggerTimerHandle;

	/** Aynı temasın saniyede onlarca hit üretmesini engeller. */
	float LastHitTime = 0.f;
};
