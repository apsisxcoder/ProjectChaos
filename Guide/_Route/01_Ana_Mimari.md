# PROJECT CHAOS — ANA MİMARİ (Architecture Overview)

> Çalışma adı: **Project Chaos**
> Engine: **UE5.7** · Framework: **GAS** · Network: **Listen Server (şimdilik)**
> Bu doküman üst seviye iskelettir. Detaylar alt dokümanlara dağıtılmıştır.

---

## 0. Doküman Seti (nasıl okunur)

```
01_Ana_Mimari.md      <-- buradasın (genel mimari, otorite, sistem haritası)
02_Scenario.md        <-- Scenario data yapısı, görev havuzları, A->B yaşam döngüsü
03_Player.md          <-- 3 katmanlı player, iki fazlı init, GAS
04_Trait.md           <-- karma scope (Global/HeroLocked), pasif slot, GAS uygulama
05_Objective.md       <-- ObjectiveBase, 5 arketip, GameplayTag event omurgası, progress
06_Chaos_Scoring.md   <-- ChainID combo, skor, awards (puansız), reveal — OYUNUN BEYNİ
07_World_Events.md    <-- (gelecek) event cluster, klip-değeri filtresi
```

Kural: bir şey kodlanmadan önce ilgili doküman güncel olmalı. Doküman ile kod
çelişirse **doküman düzeltilir veya kod düzeltilir** — sessizce sapılmaz.

---

## 1. Vizyon (1 paragraf)

8 oyunculu, emergent chaos party oyunu. Oyuncular birbirini öldürmeye değil,
**aynı dünya objesi üzerinde çelişen gizli görevleri** tamamlamaya çalışır.
İçerik kaynağı hero değil; **Scenario + Objective + Trait + World Event**
kombinasyonudur. Başarı = en çok kill değil; kazanma modeli **C**:
`Objectives + Chaos + End Match Awards`.

Tek metrik her şeyin üstünde: *oyunu hiç bilmeyen biri 10 sn'lik klibe gülüyor mu?*
Tüm tasarım kararları bu filtreden geçer.

---

## 2. Mimari İlkeler (değişmez)

1. **Data-driven.** Hero, Scenario, Objective, Trait, Event = Data Asset.
   Yeni içerik = yeni data, mümkün olduğunca yeni kod değil.
2. **Server-authoritative.** Tüm gameplay sonuçları (fizik itme, görev progress,
   skor) sunucuda hesaplanır. Client sadece input gönderir ve sonucu görür.
3. **GameplayTag event omurgası.** Objective mantığı ability mantığından
   ayrıdır. Ability bir tag broadcast eder; görev o tag'i dinler/sayar.
   (bkz. 05_Objective.md)
4. **Okunabilirlik.** Hero silüeti uzaktan okunur; Scenario tek objede toplanır;
   görünmeyen/klip üretmeyen sistem eklenmez.
5. **Scope kilidi.** 4 hero max. Battle pass / ranked / inventory / crafting yok.

---

## 3. Üst Seviye Sistem Haritası

```
                       ┌─────────────────────────────┐
                       │   GameMode (SERVER ONLY)     │
                       │   - Match akışı (start/end)  │
                       │   - Scenario SEÇİMİ          │
                       │   - Objective DAĞITIMI       │
                       └──────────────┬──────────────┘
                                      │ spawn / assign
                ┌─────────────────────┼─────────────────────┐
                │                     │                     │
   ┌────────────▼──────────┐  ┌───────▼────────┐  ┌─────────▼─────────┐
   │  GameState            │  │  Scenario      │  │  PlayerState (xN) │
   │  (REPLICATED → all)   │  │  (sahne aktörü)│  │  (per-player)     │
   │  - ChaosScore tablosu │  │  - Shared obj  │  │  - ObjectiveComp  │
   │  - Public scoreboard  │  │  - NPC         │  │    (OwnerOnly rep) │
   │  - ChaosSubsystem mant│  │  - A->B state  │  │  - Atanmış görevler│
   └───────────────────────┘  └────────────────┘  └─────────┬─────────┘
                                                            │ controls
                                                  ┌─────────▼─────────┐
                                                  │  Pawn / Character │
                                                  │  - HeroComponent  │
                                                  │  - TraitComponent │
                                                  │  - CosmeticComp   │
                                                  │  - GAS AbilitySys │
                                                  └───────────────────┘
```

Kritik nokta: **gizli görevler herkese replicate OLMAZ.** Her oyuncunun görevi
sadece sahibine gider (`COND_OwnerOnly`). Aksi halde oyuncular birbirinin
görevini görür ve oyunun çatışma çekirdeği çöker. (bkz. §5)

---

## 4. Otorite & Replikasyon Modeli

| Sistem                 | Nerede yaşar          | Otorite     | Kimle paylaşılır        |
|------------------------|-----------------------|-------------|-------------------------|
| Match akışı, scenario seçimi | GameMode        | Server only | Hiç (client görmez)     |
| Atanmış görevler       | PlayerState/ObjectiveComp | Server  | **Sadece sahibi** (OwnerOnly) |
| Görev progress         | ObjectiveComp         | Server      | Sadece sahibi           |
| Public chaos skor      | GameState             | Server      | Herkes (replicated)     |
| Combo detection        | ChaosSubsystem (GameState tarafı) | Server | Sonuç herkese |
| Fizik (push/launch/ragdoll) | Pawn / movement  | **Server**  | Herkese (simüle)        |
| Hero ability'leri      | AbilitySystemComp     | Server (GAS) | Standart GAS replikasyon |
| Visual mesh / kozmetik | CosmeticComp          | Server seçer | Herkese (cosmetic-only) |

**KARAR (kilitlendi): Hareket = CMC (Character Movement Component).**
BP_ThirdPerson template tabanlı (ACharacter + CMC hazır gelir), custom C++
extension ile genişletilir. **Mover REDDEDİLDİ** çünkü: UE5.7'de hâlâ
beta/experimental (production-ready ~5.8/5.9, ~18 ay sonra), API churn riski
solo dev'i framework'le uğraştırır; asıl gücü GASP/pürüzsüz locomotion ki bizim
karakterlerimiz bilerek bozuk — Mover'ın güçlü olduğu yer bizim umursamadığımız
yer. Marketteki her animasyon plugin'i CMC'ye göre yazılmış (solo dev için destek
ağı). Ragdoll/savrulma zaten Chaos physics işi (movement component'ten bağımsız),
server-authoritative çözülür. Mover bir sonraki projeye saklanır.

---

## 5. Katmanlı Player Modeli (overview → 03_Player.md)

Bir oyuncu üç bağımsız katmandan oluşur:

```
Player
 ├─ Visual Identity   = HeroArchetype.VisualPool içinden random mesh (silüet korunur)
 ├─ Hero Ability Set  = HeroArchetype (Rocket / Magnet / Teleport / Giant ...)
 └─ Match Modifiers   = Trait + atanmış Objective'ler + aktif World Event
```

- **Hero ≠ görünüş.** Rocket Hero'nun 5 farklı mesh'i var; biri random seçilir.
  Ama hepsi roket siluetini taşır → rakip "bu uçacak" der.
- Görünüş **gameplay bilgisini gizlemez**, sadece çeşitlilik verir.
- Detay (component'ler, replikasyon, init sırası) 03_Player.md'de.

---

## 6. Scenario Sistemi (overview → 02_Scenario.md)

İçerik birimi **Scenario**'dur. Bir Scenario = elle kurgulanmış iskelet +
random doldurulan görev havuzları.

```
DA_Scenario (örnek: "Trambolin")
 ├─ SharedObject     : paylaşılan obje (trambolin)
 ├─ NPC              : tek establishing shot'ın öznesi (ağlayan çocuk)
 ├─ Cinematic        : sabit kamera + NPC anim + text overlay (FİLM DEĞİL)
 ├─ Lifecycle        : A noktası (başlangıç) -> B noktası (çözüm)
 ├─ PrimaryPool[~20] : birbiriyle ÇELİŞEN ana görevler
 ├─ SidePool         : küçük yan görevler
 ├─ ChaosPool        : doğrudan sabotaj görevleri
 └─ SolvabilityFlag  : tüm görevler aynı anda mümkün mü? / "imkânsızlık = komedi"
```

Maç başında her oyuncuya dağıtılır:
**1 Primary (PrimaryPool'dan random) + 2 Side + 1 Chaos.**
Kimse ne çekeceğini bilmez; kimse başkasının görevini görmez.

Sinematik kuralı: tek shot, sonra ekrana **Primary** yazılır
(örn. "Çocuğun oyuncağına kavuşmasına yardım et"). Her görev için ayrı cutscene
YOK — sinematik bir *kamera + metin sistemidir*, animasyon filmi değil.

---

## 7. Objective Sistemi (overview → 05_Objective.md)

Her görev, bir GameplayTag event'ine bağlı bir koşuldur.

```
Ability "Jump"      --broadcast-->  Event.Trampoline.Jump
Görev "10 kez zıpla"  --listen-->   sayaç++  (server'da)
Görev "elindekini çal" --listen-->  Event.Item.Stolen
```

Sonuç: yeni görev eklemek = yeni tag + sayaç/koşul, çoğunlukla **kod değil**.
Görev tipleri: 5 arketip (Counter, Hold, Carry, Prevent, Transport). Detay 05_Objective.md'de.

---

## 8. Chaos & Scoring (overview → 06_Chaos_Scoring.md)

Combo detection **tek oyuncuda olamaz** — birden çok oyuncunun etkileşimini
görmek gerekir (Rocket launch → Magnet pull → wall hit zinciri). Bu yüzden
**ChaosSubsystem GameState tarafında, server-authoritative** çalışır.
Component'ler sadece "ben şu olayı ürettim" event'i atar; değerlendirme merkezde.

Skor = Objective + Chaos (model C). Awards **puansızdır** (overjustification —
06'da gerekçeli). "En Çok Sabote Edilen" gibi ödüller bilgilendirici tanımadır,
sıralamaya girmez → kaybeden bile güler. Detay 06'da.

---

## 9. Phase 0 — Build Order (kod sırası)

Bu sırayla kodlayacağız. Her adım bir öncekine yaslanır:

1. **ScenarioSubsystem + DA_Scenario iskeleti** — scenario seç, sahneyi spawn et.
   (GameMode server-only mantığı)
2. **ObjectiveComponent** — havuzdan görev atama (OwnerOnly rep), GameplayTag
   event dinleme, progress server'da.
3. **Tek gerçek Scenario: Trambolin** — 1 shared object, ~20 çelişen primary,
   birkaç side/chaos. Asıl test bu: yabancı klibe gülüyor mu?
4. **Fizik (push / launch)** — server-authoritative, kontrollü ragdoll + hızlı
   recovery (tam Gang Beasts ragdoll'u değil).
5. **ChaosSubsystem + scoring** üstüne biner.
6. **Hero/Trait/World Event** sistemleri sonra genişler.

Deliverable: aynı haritada 8 oyuncu, trambolin senaryosu, gizli görevler,
çalışan sabotaj.

---

## 10. Scope Protection (asla ekleme)

Battle pass · Ranked · 50 hero · Crafting · Inventory · MMO özellikleri.

**Altın kural:** bir özellik (1) kaos, (2) oyuncu etkileşimi, (3) komik hikaye
üretmiyorsa — yapılmaz.

---

## 11. Networking & Hareket Kurulumu (KİLİTLİ)

### 11.1 Topoloji: Listen Server (ship) + Dedicated test
- **Ship hedefi: Listen Server.** Host hem server hem oyuncu. 8 oyuncu, persistent
  save yok, her maç kapalı → tam listen server profili. Engine'de hazır, ayrı
  server build'i gerekmez. (Dedicated, persistent dünya/MMO için zorunludur —
  bizde değil; ayrıca source derleme + ayrı executable yükü getirir.)
- **Sonradan dedicated'a geçiş** (competitive integrity / host-advantage sorunu
  için ileride istenirse) authority-temiz kod sayesinde neredeyse sancısızdır.

### 11.2 "Sonradan multiplayer" YASAK — baştan server-authoritative
Tek oyuncu mantığıyla yazıp sonra multiplayer'a çevirmek = yeniden yazım.
Her gameplay sonucu **ilk günden** server'da hesaplanır. Tek oyuncuda kurulan
hiçbir davranış yoktur. (Sadece saf görsel/asset denemeleri — "bu ragdoll
komik mi" — tek pencerede yapılabilir; davranış değil.)

### 11.3 Geliştirme/test kurulumu
```
HEDEF (ship) : Listen Server
GÜNLÜK TEST  : PIE "Number of Players: 3" + Net Mode: Play As Listen Server
KRİTİK TEST  : ARA SIRA Net Mode "Play As Client" (dedicated) ile test et
               → host'un GİZLEDİĞİ authority bug'larını yakalar
KURAL        : tüm gameplay sonuçları HasAuthority() arkasında
```
**Listen server tuzağı:** host'ta her şey yerel olduğu için, client'ta
çalışmayacak yanlış kod host penceresinde "çalışıyormuş gibi" görünür ve bug'ı
gizler. Bu yüzden periyodik **dedicated modda test** + her yerde **`HasAuthority()`**
+ `GetNetMode()` farkındalığı zorunludur. Bu, sonradan-çevirme tuzağının ilacıdır.

### 11.4 Ragdoll/fizik senkronizasyonu (Phase 0 #1 risk)
Varsayılan UE ragdoll yalnız pelvis konumunu replicate eder → 8 oyuncuda her
client farklı ragdoll görür ve combo/chaos "kim nereye savruldu"ya bağlı olduğu
için desync sistemi kırar. Bu yüzden ragdoll **server-authoritative** olmalı
(server simüle eder, sonuç replicate). Test asla tek pencerede değil, **2+ client**
ile yapılır. Model: kontrollü ragdoll + hızlı recovery (tam Gang Beasts değil),
server-authoritative, kısa-orta süreli (1.5-2 sn). bkz. 06_Chaos_Scoring (ChainID
fizik temeli) ve 03_Player (stagger/recovery).

---

## 12. Açık Kararlar (karara bağlandıkça güncellenecek)

- [x] ~~Hareket/fizik: CMC vs Mover~~ → **CMC** seçildi (BP_ThirdPerson tabanlı). Gerekçe §4.
- [x] ~~Tur süresi~~ → **~5-10 dk**, hero kontrol zorluğuna bağlı, playtest kalibre (02).
- [x] ~~Solvability~~ → **NotSolvable** kilitlendi: çatışma çözülmez, imkânsızlık = komedi (02).
- [x] ~~Listen server yeterli mi?~~ → **ÇÖZÜLDÜ: Listen server (ship) + dedicated modda test.** Detay §11.
- [x] ~~Competence floor~~ → **KURAL: garantili "ben yaptım" anı + Best Moments** (00_Felsefe §1.2, 06 §7.5).
- [x] ~~Anti pile-on~~ → **KURAL: aynı hedefe 2'den fazla görev atanamaz** (00_Felsefe §5, 02 dağıtım).

**Gerçekten açık kalanlar (playtest/ileride):**
- [ ] Orphan çarpışma puanı (06 §13), çarpışma spam cap'i (06 §13).
- [ ] %50/40 Objective/Chaos dengesinin ham puan tavanlarıyla zorlanması.
- [ ] Weight skalası, tier eşikleri, hero combo çarpan değerleri → playtest kalibrasyonu.
- [ ] Stagger/twerk state machine: ortak mı, hero-özel mi (03 §7).
- [ ] Round tekrarı: lobiye dönüş mü, harita içi reset mi (03 §7).
