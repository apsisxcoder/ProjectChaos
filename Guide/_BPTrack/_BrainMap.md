# BRAIN MAP — Claude'un Sistem Zihin Haritası

> Bu, Claude'un Project Chaos'u nasıl modellediğidir. Karar/kod öncesi buraya bakılır.
> Canlı belge: anlayış değiştikçe güncellenir.

---

## 0. ÇALIŞMA KURALLARI (asla atlama)
```
Yeni adım → 1) mevcut çalışıyor mu DOĞRULA
            2) source code'a BAK
            3) interneti ARA
            4) gerekirse brain map kur
            5) gerekçe SUN
            6) ONAY al → ANCAK O ZAMAN kod yaz
Sorumluluk: oyunu KULLANICI kurar. Claude tasarım kararı vermez, seçenek+gerekçe sunar.
Backend = C++ BlueprintCallable primitive + state. Görsel/akış/tuning = BP (rebuild'siz).
```

---

## 1. KATMAN HARİTASI
```
                ┌─────────────────────────────────────┐
                │  C++  AChaosCharacter (OTORİTE)      │
                │  - Server_LaunchSelf (RPC)           │
                │  - Server_ApplyKnockback (RPC)       │
                │  - Server_SetStaggered (RPC)         │
                │  - bStaggered (replicated)           │
                │  - DoLaunch / SetStaggered / Recover │
                │  - OnCapsuleHit (solid → aktarım)    │
                │  - OnStaggerChanged() [BP'ye event]  │
                └───────────────┬─────────────────────┘
                                │ BlueprintImplementableEvent / Callable
                ┌───────────────▼─────────────────────┐
                │  BP_ThirdPersonCharacter (GÖRSEL/AKIŞ)│
                │  - IA_Launch → Server_LaunchSelf      │
                │  - OnStaggerChanged → ragdoll blend   │
                │  - Collision presetleri (mesh/capsule)│
                │  - Tuning (Force, UpStrengthRatio...) │
                └───────────────────────────────────────┘
```

## 2. COLLISION MODELİ (kilitli karar)
```
Capsule  : Pawn = BLOCK   → oyuncular solid, içinden geçmez, Hit ile aktarım algısı
Mesh     : Pawn = IGNORE  → ragdoll yalnız görsel, kimseyi fiziksel itmez
Aktarım  : OnComponentHit (solid çarpma), Overlap DEĞİL (overlap reddedildi)
```

## 3. STAGGER / RAGDOLL MODELİ (kullanıcı netleştirdi)
```
Stagger = AZALAN SÜRE sayacı (bool değil).
- Skill başlatıcı  : etkilenene +StaggerDuration EKLER (birikir; 2 skill = 2×süre)
- Ragdoll dağıtıcı : sersem beden SOLID çarparsa taze kişiye stagger uygular
- Süre delta-time ile azalır → 0'da ayılır
- TAŞIYICI çarpınca AYILMAZ → kendi süresi dolana kadar sersem (P2+P3 aynı anda olabilir)
- Teknik: timer = (kalan + StaggerDuration), per-frame tick YOK → §2.5 ile aynı
- StaggerDuration BP'den ayarlanır (şimdilik ~3 sn fikri)
```

## 4. ChainID'ye köprü (06_Chaos_Scoring — sonraki büyük adım)
```
Skill kullanımı      → YENİ ChainID (Owner = skill sahibi)
İstemsiz çarpışma    → mevcut ChainID'yi yayar (kurbana bulaştırır)  ← bizim OnCapsuleHit aktarımı
Zincir kapanır       → kendi sersemleri kalmayınca → puan Owner'a
Şu an: fiziksel temel kuruluyor (launch + ragdoll + aktarım).
Sonra: ChaosSubsystem (GameState, server) bu olayları zincire bağlar + puan.
```

## 5. PHASE 0 DURUM
```
[x] Launch (server-auth, 3 pencere sync)
[x] Kısmi ragdoll görseli (spine_01 blend, BP)
[x] Mesh collision (Pawn=Ignore)
[x] Solid kapsül + Hit aktarım (Block; taze kişiye bulaşır = option a; taşıyıcı ayılmaz)
[x] Stagger birikim (kalan+StaggerDuration; per-frame tick yok)
[ ] ChaosSubsystem / ChainID puanlama
```

## 6. KARARLAR
- [x] Sersem beden ZATEN sersem birine çarparsa → **(a) hiçbir şey.** "Sersemlik sersem
      DEĞİLSE bulaşıcı" — aktarım yalnız taze kişiye. (2026-06-25)
- [ ] Recovery yumuşatma (Timeline) — sonra.
