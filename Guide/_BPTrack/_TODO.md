# TODO — Gelecekteki İyileştirmeler / Ertelenen İşler

> Şu an çalışan ama "sonra daha iyisini yapacağız" denilen her şey buraya yazılır.
> Tek kaynak: bir iş ertelenince buraya eklenir, yapılınca işaretlenir/silinir.
> Şimdi yapma sebebi: prototip akışını bölmemek; çekirdek önce.

---

## Fizik / Stagger (Phase 0 cilası)
- [ ] **Movement kapama** — stagger sırasında karakterin input/hareketini kıs/kapat
      (şu an sersemken hâlâ yürüyebiliyor olabilir). Sersem = kontrol yok hissi.
- [ ] **Tuning geçişi** — Force, UpStrengthRatio, TransferForce, StaggerDuration (~3 sn),
      blend weight (0.75) değerleri playtest ile kalibre edilecek. Hepsi BP'den ayarlı.
- [ ] **Recovery yumuşatma** — ragdoll blend 1→0 geçişini Timeline ile yumuşat
      (şu an sert/anında). bkz. [02_Ragdoll_Gorseli.md](02_Ragdoll_Gorseli.md)
- [ ] **Launch yöntemi** — client'ta rubber-band görülürse LaunchCharacter yerine
      RootMotionSource (FRootMotionSource_ConstantForce). Şimdilik gerek yok (sync ✓).

## Collision / His
- [ ] Solid kapsül "his"i: 8 kişi dar alanda — itişme dengesi playtest'te bakılacak.

## Sonraki büyük adım (erteleme değil, sıradaki iş)
- [ ] **ChaosSubsystem / ChainID puanlama** (06_Chaos_Scoring) — fiziksel temel bitti,
      olayları zincire bağlayıp puanlama buraya biner.

---
> Not (Claude): yeni "sonra yaparız" çıktığında BURAYA ekle, [00_Index.md](00_Index.md)'ten linkli tut.
