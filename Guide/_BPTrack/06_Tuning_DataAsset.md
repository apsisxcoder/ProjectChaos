# 06 — Karakter Tuning Data Asset

> Konu: Karakter fizik/stagger ayarlarını C++/BP-default yerine **Data Asset**'e taşımak (data-driven, 01 §2.1).
> Kod: `UChaosCharacterTuning : UDataAsset` + `AChaosCharacter::Tuning` referansı (fallback'li getter'lar).

---

## Neden
- Yeni karakter = yeni DA (ya da paylaş) → genel değişkenleri **tek yerden** değiştir, recompile yok.
- Config editörde görünür → debugging kolaylaşır.
- Üç katman: C++ mantık / **Data Asset config** / BP görsel.

## Yapı
`UChaosCharacterTuning` alanları (varsayılan = fallback ile aynı):
- `UpStrengthRatio` = 0.5
- `TransferForce` = 1000
- `StaggerDuration` = 1.8

`AChaosCharacter`:
- `Tuning` (EditDefaultsOnly) referansı.
- `GetUpStrengthRatio()/GetTransferForce()/GetStaggerDuration()` → Tuning varsa ondan, **yoksa standart varsayılan** (fallback). Asset unutulsa bile çalışır.

## BP'de yapılacak (tek seferlik)
1. Content Browser → sağ tık → **Miscellaneous → Data Asset** → sınıf: **ChaosCharacterTuning** → adı: `DA_DefaultCharacter`.
2. İçinde değerleri gir (ya da varsayılan bırak).
3. `BP_ChaosCharacter` → Class Defaults → **Chaos|Tuning → Tuning = DA_DefaultCharacter**.

## Doğrulama
- Rebuild → davranış **aynı** kalmalı (varsayılanlar eşit). DA atamasan bile fallback ile çalışır.
- DA'da bir değeri değiştir (örn. StaggerDuration=3) → recompile olmadan etkisini gör.

## Sonra
- 5.3'te aynı desen `DA_ChaosTuning` (tier eşiği + puan) ile tekrarlanır.
- İleride LaunchForce + hero ComboMultiplier da DA'ya (DA_HeroProfile).
