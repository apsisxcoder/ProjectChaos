# 07 — Chaos Puanlama (5.3)

> Konu: Zincir kapanınca puan hesaplanıp Owner'ın PlayerState'ine yazılır (replike). Sayılar DA'da.
> Kod: `AChaosPlayerState` (replike skor), `UChaosScoringTuning` (DA), ChainComponent puanlama.

---

## Akış (06 §3-6)
```
OnImpact   → puan = PlayerHitBase + StaggerGiven + Escalation×LinkCount  → AccumulatedPoints'e ekle
CloseChain → Final = Accumulated × TierBonus(LinkCount) × HeroÇarpan(1.0) → Owner PlayerState'e yaz
```
Tier: LinkCount 2→×1.0 (COMBO), 3→×1.5 (BIG CHAOS), 4+→×3.0 (MEGA CHAOS).

## Sayılar = DataAsset (kafadan atıldı, playtest'le kalibre)
`UChaosScoringTuning` (DA): PlayerHitBase=5, StaggerGiven=5, EscalationPerLink=3 + Tier dizisi.
Atanmazsa C++ **fallback** (aynı değerler) → çalışır.

## Replikasyon
`AChaosPlayerState.ChaosScore` (Replicated + OnRep). Server yazar, client'lara replike.
`OnChaosScoreChanged` (BlueprintImplementableEvent) → ileride scoreboard/UI.

---

## BP'de yapılacaklar
**1. (ZORUNLU) PlayerState Class ata:**
- `BP_ChaosGameMode` → Class Defaults → **PlayerState Class = AChaosPlayerState**.
- ⚠️ Bu olmazsa Cast başarısız, skor YAZILMAZ (log yine hesabı gösterir ama saklanmaz).

**2. (OPSİYONEL — tweak için) DA + GameState:**
- Content Browser → Data Asset → sınıf **ChaosScoringTuning** → `DA_ChaosTuning`.
- `BP_ChaosGameState` (AChaosGameState child) oluştur → içindeki **ChaosChainComponent → Tuning = DA_ChaosTuning**.
- `BP_ChaosGameMode` → GameState Class = BP_ChaosGameState.
- Atamasan fallback ile aynı değerler çalışır; sadece tweak istersen gerekli.

## Doğrulama (log)
3 oyuncu combo → 
```
[CHAIN] LINK ID=1 P1->P2 LinkCount=1 +13 puan (ham: 13)
[CHAIN] LINK ID=1 P2->P3 LinkCount=2 +16 puan (ham: 29)
[CHAIN] KAPANDI ID=1 Owner=P1 COMBO ham=29 ×1.0 = 29 puan
```
+ client penceresinde Owner'ın ChaosScore'u replike olmalı (BP'de Get Chaos Score ile bak ya da OnChaosScoreChanged log).

## Sonra (5.4 / ileri)
- Combo cue/popup (BP görsel: "MEGA CHAOS! +N").
- Fall/WorldHit impact'leri (şu an sadece PlayerHit puanlanıyor).
- Hero ComboMultiplier (DA_HeroProfile).
