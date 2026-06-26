# ROADMAP — Build Order (yeniden sıralanmış)

> Kaynak: `Guide/_Route/01_Ana_Mimari §9`. Ragdoll/fizik, risk + kafa karışıklığı
> nedeniyle öne çekildi. Güncel sıra bu dosyada tutulur.

```
1. ScenarioSubsystem + DA_Scenario iskeleti      [ertelendi]
2. ObjectiveComponent (görev atama, OwnerOnly)    [ertelendi]
3. Trambolin Scenario (asıl "klibe gülüyor mu" testi) [ertelendi]
4. Fizik (push / launch / ragdoll / aktarım / stagger)  ✅ BİTTİ
5. ChaosSubsystem + ChainID scoring               ◀ ŞİMDİ BURADAYIZ
6. Hero / Trait / World Event                     [sonra]
```

## #5 — ChainID alt adımları (her biri ayrı doğrulanır)
- [x] **5.1** ChaosSubsystem iskeleti + zincir aç/kapat yaşam döngüsü (puan YOK, sadece log)
       → ✅ log testi geçti: aç→çık→kapat temiz, ID'ler artıyor, leak yok. (2026-06-25)
- [x] **5.1b** Tag event omurgası (GameplayMessageRouter taşındı) → component'ler broadcast, ChainComp dinler.
       → ✅ davranış aynı + eşzamanlı 2 zincir (aktif zincir: 2) kanıtlandı. bkz. [05_Tag_Event_Omurgasi.md](05_Tag_Event_Omurgasi.md)
- [x] **5.2** Impact linkleme + victim re-tag (Event.Impact.PlayerHit → OnImpact, LinkCount++)
       → ✅ kanıtlandı: relay (A→B→C aynı ID), bowling (A→B, A→C), re-tag (skill ile zincir değiştirme),
         eşzamanlı zincirler. LinkCount doğru artıyor. (2026-06-25)
- [ ] **5.3** Scoring (puan + tier + hero çarpanı) + PlayerScore replicate
- [ ] **5.4** Combo cue / popup (BP görsel: "COMBO! / MEGA CHAOS")

## Prototip eşlemesi (mevcut C++ → ChainID olayları)
```
Server_LaunchSelf   → OnSkillUsed     (yeni ChainID açar)
OnCapsuleHit aktarım → OnImpact(PlayerHit)  (zinciri yayar)
RecoverFromStagger  → OnStaggerRecovered    (zincir kapanışını tetikler)
```
Yani ChainID'nin tükettiği olaylar zaten kurulu; üstüne biniyoruz.

## Sonra
- #4 ertelenenler `_TODO.md`'de (movement kapama, tuning, recovery yumuşatma).
- #5 bitince → #1-3'e (Scenario/Objective) temiz dönüş.
