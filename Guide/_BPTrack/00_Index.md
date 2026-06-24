# PROJECT CHAOS — BLUEPRINT TAKİP (BPTrack)

> Bu klasör, **kod'da görünmeyen** Blueprint kurulumunu insan-okur şekilde belgeler.
> `.uasset` ikili dosya → git'te diff okunmaz. Burası "BP'de elle ne kuruldu" defteridir.
> Kural: BP'de bir şey kurulunca/değişince buraya not düşülür (ekran görüntüsüyle gelir).

> İlgili kod tarafı: `Source/ProjectChaos/` (C++ primitive'ler).
> İlgili tasarım: `Guide/_Route/` (00–06 doküman seti).

---

## Dosya seti (adım adım, net başlıklar)

| # | Dosya | Konu | Durum |
|---|-------|------|-------|
| 01 | [01_Player_Reparent_ve_Input.md](01_Player_Reparent_ve_Input.md) | BP_ChaosCharacter → AChaosCharacter reparent + IA_Launch input bağlama | ✅ kuruldu |
| 02 | [02_Ragdoll_Gorseli.md](02_Ragdoll_Gorseli.md) | OnStaggerChanged event'inde kısmi ragdoll blend (spine_01) | ✅ kuruldu |
| 03 | [03_Collision_Ayarlari.md](03_Collision_Ayarlari.md) | Mesh ve Capsule collision preset/response ayarları | ✅ mesh + capsule |
| 04 | [04_GameState_Kurulumu.md](04_GameState_Kurulumu.md) | BP GameMode → GameState Class = AChaosGameState | 🔶 BP'de set edilecek |
| — | [_BrainMap.md](_BrainMap.md) | Claude'un sistem zihin haritası (C++ ↔ BP ↔ tasarım) | canlı |
| — | [_TODO.md](_TODO.md) | Gelecekteki iyileştirmeler / ertelenen işler (movement kapama, tuning, recovery yumuşatma) | canlı |

---

## Genel durum (Phase 0 fizik temeli)
- ✅ Launch (server-authoritative, 3 pencere sync)
- ✅ Kısmi ragdoll görseli (spine_01 blend)
- ✅ Mesh collision (ragdoll kimseyi itmez)
- ✅ Solid kapsül + Hit ile aktarım (Block, OnCapsuleHit; taze kişiye bulaşır, taşıyıcı ayılmaz)
- ✅ Stagger birikim modeli (kalan + StaggerDuration; rebuild gerekir)
- ⬜ ChaosSubsystem / ChainID puanlama (sonraki büyük adım)
