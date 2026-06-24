# 04 — GameState Kurulumu (ChaosSubsystem evi)

> Konu: ChainID beynini (`UChaosChainComponent`) barındıran `AChaosGameState`'i devreye almak.
> Kod karşılığı: `AChaosGameState` (GameState) + `UChaosChainComponent` (component).
> Mimari gerekçe: WorldSubsystem multiplayer'da replike olmaz → GameState doğru ev (araştırma + 06 doküman).

---

## BP'de yapılacak (tek seferlik)
1. `Content/ThirdPerson/Blueprints/BP_ThirdPersonGameMode` aç.
2. **Class Defaults → Classes → GameState Class = `AChaosGameState`** (veya ondan türetilmiş BP).
3. Kaydet.

> Alternatif: C++ GameMode yoksa BP GameMode'da set etmek yeterli. DefaultEngine.ini'ye
> global yazmak da olur ama proje BP GameMode kullanıyor, oradan set etmek en temizi.

## Doğrulama (5.1 — log)
PIE 3 oyuncu / Listen Server, Output Log açık (`LogChaosChain` filtrele):
- Launch'a bas → `[CHAIN] AÇILDI ID=1 Owner=...`
- Ayılınca → `[CHAIN] ... zincirden çıktı` → `[CHAIN] KAPANDI ID=1`
- Birkaç oyuncu aynı anda → ID'ler karışmamalı, her zincir kendi owner'ında.

## Henüz YOK (sonraki adımlar)
- Impact linkleme / eskalasyon / victim re-tag → 5.2
- Puan / tier / hero çarpanı + replike skor → 5.3
- Combo popup (BP görsel) → 5.4
