# 05 — Tag Event Omurgası (GameplayMessageSubsystem)

> Konu: Component'ler ChaosSubsystem'i doğrudan çağırmak yerine **GameplayTag kanalına yayın** yapar; subsystem dinler. Ability ↔ Chaos/Objective decoupling (01 §2.3, 06 §10).

---

## Neden / hangi araç
- Doküman "GameplayTag event omurgası" şart koşuyor.
- UE 5.7'de **GameplayMessageRouter engine'de YOK**; `AsyncMessageSystem` Experimental (Mover gibi → reddedildi).
- Karar: Lyra'nın **GameplayMessageRouter** plugin'i (Epic, self-contained, tek bağımlılık `GameplayTags`, beta ama production-proven) projeye **taşındı**.

## Kurulum (yapıldı)
- Plugin: `Plugins/GameplayMessageRouter/` (Lyra'dan kopyalandı, Binaries/Intermediate temizlendi).
- `ProjectChaos.uproject` → Plugins'e `GameplayMessageRouter` eklendi.
- `Build.cs` → `GameplayTags`, `GameplayMessageRuntime` eklendi.

## Tag taksonomisi (native, C++)
`ChaosGameplayTags.h/.cpp`:
- `Event.Skill.Used` — skill kullanıldı → yeni ChainID
- `Event.Penalty.Recovered` — oyuncu ayıldı → zincirden çık
- (5.2'de: `Event.Impact.PlayerHit` vb.)

## Mesaj yükü
`FChaosEventMessage { APlayerState* Instigator; APlayerState* Target; }` (Lyra FVerbMessage deseni).

## Akış (yeni)
```
ChaosCharacter (server):
  Server_LaunchSelf   → BroadcastMessage(Event.Skill.Used, {Instigator=PS})
  RecoverFromStagger  → BroadcastMessage(Event.Penalty.Recovered, {Instigator=PS})

UChaosChainComponent (GameState, server) BeginPlay:
  RegisterListener(Event.Skill.Used → HandleSkillUsed → OnSkillUsed)
  RegisterListener(Event.Penalty.Recovered → HandleStaggerRecovered → OnStaggerRecovered)
  EndPlay: listener'ları Unregister
```
Davranış 5.1 ile **aynı** (loglar değişmez); sadece decoupled + tag'li.

## Teknik notlar
- `UGameplayMessageSubsystem` bir **GameInstanceSubsystem** (instance-local, replike değil).
  Server'da broadcast → server'daki listener çalışır. Chaos mantığı server-side → sorun yok.
- Skor replikasyonu ayrı (PlayerState/GameState), bus değil.

## Doğrulama
Rebuild + PIE: 5.1 logları **birebir aynı** çıkmalı (`AÇILDI/çıktı/KAPANDI`). Çıkıyorsa decoupling başarılı.

## Sonra
- 5.2 impact event'leri de bu omurgadan (`Event.Impact.*`).
- Objective sistemi (05_Route) aynı bus'ı kullanır → tekrar yazılmaz.
- GAS gelince abilities de aynı bus'a yayınlar (framework-agnostik).
