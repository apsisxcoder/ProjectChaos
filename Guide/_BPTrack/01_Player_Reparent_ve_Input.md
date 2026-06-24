# 01 — Player Reparent ve Input Bağlama

> Konu: `BP_ChaosCharacter`'ı C++ `AChaosCharacter`'a bağlamak ve launch input'unu kurmak.
> Kod karşılığı: `AChaosCharacter::Server_LaunchSelf` (BlueprintCallable, Server RPC).

---

## 1. Reparent
- **BP:** `Content/Blueprints/BP_ChaosCharacter`
- **Parent Class:** `AChaosCharacter` (C++) olarak değiştirildi.
- Sonuç: template'in tüm hareket/kamera/input'u korunur, üstüne Chaos primitive'leri gelir.

## 2. Input Action
- **IA_Launch** adında Input Action (Digital/bool) oluşturuldu.
- Input Mapping Context'e bir tuşa bağlandı (tuş: _[doldurulacak — ekran görüntüsü gelince]_).

## 3. Event Graph — Launch bağlama
```
Enhanced Input Action IA_Launch (Started)
   → Server_LaunchSelf
        Direction = Get Actor Forward Vector
        Force     = _[değer: ~500–1200, BP'de tuning]_
```
- Mantık C++'ta; BP sadece input'u primitive'e bağlar.

## Açık / doldurulacak
- [ ] IA_Launch hangi tuşa bağlı? (ekran görüntüsü)
- [ ] Class Defaults → Chaos|Input alanları atandı mı? (LaunchAction, ChaosMappingContext) — veya doğrudan event node mu kullanıldı?
- [ ] Tuning değerleri: UpStrengthRatio, Force kaç?
