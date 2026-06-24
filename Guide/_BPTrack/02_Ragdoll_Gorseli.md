# 02 — Ragdoll Görseli (OnStaggerChanged)

> Konu: Sersem (stagger) durumunda kısmi ragdoll blend'i. Tamamen BP'de — C++ mesh fiziğine dokunmaz.
> Kod karşılığı: `AChaosCharacter::OnStaggerChanged(bool)` (BlueprintImplementableEvent).
> Tasarım: `01_Ana_Mimari §11.4` — "kontrollü ragdoll + hızlı recovery, tam Gang Beasts değil".

---

## Neden kısmi blend (full ragdoll değil)
Full `Set Simulate Physics` → mesh kapsülden kopar, yere batar (yaşadığımız bug).
Kısmi blend → `spine_01`'den yukarısı floppy, pelvis/bacaklar kapsülle gider → kopma yok.

## Event Graph — OnStaggerChanged
```
Event OnStaggerChanged (New Staggered : bool)
  → Branch (New Staggered)
     TRUE (sersem):
        1. Mesh → Set All Bodies Below Simulate Physics
              In Bone Name = spine_01 ; New Simulate = ✔ ; Include Self = ✔
        2. Mesh → Set All Bodies Below Physics Blend Weight
              In Bone Name = spine_01 ; Physics Blend Weight = 0.75  (tuning, 1.0'a kadar çıkılabilir)
     FALSE (recovery):
        1. Mesh → Set All Bodies Below Physics Blend Weight
              In Bone Name = spine_01 ; Physics Blend Weight = 0.0
        2. Mesh → Set All Bodies Below Simulate Physics
              In Bone Name = spine_01 ; New Simulate = ✗
```
- Bone adı bir **değişken** (tek yerden değiştirilir): değer = `spine_01`.

## Önemli ön koşul
- Mesh'in collision'ı fiziğe izin vermeli (bkz. [03_Collision_Ayarlari.md](03_Collision_Ayarlari.md)).
  Aksi halde `Set All Bodies Below Simulate Physics` sessizce çalışmaz.

## Açık / sonra
- [ ] Recovery'yi yumuşatma (Timeline ile blend 1→0 geçişi). Şu an sert (anında).
- [ ] Floppy seviyesi yeterli mi, yoksa farklı bone mu (pelvis = daha çok ragdoll)?
