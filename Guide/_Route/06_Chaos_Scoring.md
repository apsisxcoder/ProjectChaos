# PROJECT CHAOS — CHAOS & SCORING (OYUNUN BEYNİ)

> Doküman seti: `00` → `01` → `02_Scenario` → `03_Player` → `04_Trait` → `05_Objective` → **`06_Chaos_Scoring`**
> Ön koşul: 05_Objective (ObjectiveBase, progress) + 03_Player (ASC, penalty event'leri).
> **Bu doküman implementation-ready'dir — Claude Code ile doğrudan kodlanacak.**
> Otorite: tüm hesap **server-authoritative** (ChaosSubsystem, GameState tarafı).
> Performans: olay bazlı, **per-frame tick YOK** (05 ile tutarlı).

---

## 0. İki Skor Hattı (karıştırma)

Oyunda **iki ayrı** skor kaynağı vardır. Aynı şey değildir:

```
HAT 1 — Objective Score : "görevini ne kadar yaptın"     (05_Objective)
HAT 2 — Chaos Score     : "ne kadar komik kaos yarattın"  (BU DOKÜMAN)
```

Biri görevini hiç yapamayıp yine de maçın yıldızı olabilir (herkesi birbirine
çarpıştıran ama kendi çöpünü taşıyamayan adam). Bu bilinçlidir.

Final sıralama (model C):
```
PlayerFinalScore = ObjectiveScore + ChaosScore
                   (Awards SIRALAMAYA GİRMEZ — §7, psikolojik gerekçe)
Hedef ağırlık (playtest kalibresi): ~%50 Objective / ~%40 Chaos / Awards puansız
```

---

## 1. Tasarım Kuralları (SERT — ihlal edilemez)

Bunlar psikoloji araştırmasından gelen, retention'ı belirleyen kurallardır:

1. **Awards puan VERMEZ.** Overjustification effect: zaten içsel komik olan bir
   şeyi puanla ödüllendirmek onu "işe" çevirir ve farming üretir. Awards yalnız
   **bilgilendirici tanıma** (kimlik/rozet), sıralama dışı. (§7)
2. **Chaos puanı yönlendirir, motivasyon ÜRETMEZ.** Çekirdek loop puansız da
   komik olmalı. Puan bir scaffold'dur, oyunun kalbi değil.
3. **Competence = sistem zekâsı, kontrol hassasiyeti DEĞİL.** Hero'lar bozuk;
   ustalık combo okumak, rakip görevini tahmin etmek, kaosu yönlendirmektir.
   Bu hem çocuğu (düşük stres) hem rekabetçiyi (derinlik) aynı anda tutar.
4. **Reveal = relatedness motoru.** Maç sonu gizli görev açıklaması retention'ın
   asıl kaynağıdır (Among Us'ı Fall Guys'a üstün kılan sosyal hikâye). (§8)
5. **Sabotaj geciktirir, silmez.** (05'ten) Penalty progress'i dondurur/azaltır,
   sıfırlamaz — frustrasyon değil komedi.

---

## 2. ChainID Modeli — Combo'nun Kalbi

Combo'lar **global değildir.** Haritada aynı anda birçok bağımsız zincir olabilir.
Her zincir bir `ChainID` ile izlenir. Zincir = olay grafiği, global sayaç değil.

### Çekirdek kurallar
```
1. SKILL kullanımı   → YENİ ChainID açar, o oyuncuya (Owner) etiketlenir.
2. İstemsiz çarpışma → MEVCUT ChainID'yi yayar (savrulan kişi kendi taşıdığı
                       ChainID'yi bir sonraki kurbana bulaştırır).
3. Bir oyuncu aynı anda TEK zincire aittir (en son onu sersemleten zincir).
   Yeniden vurulursa yeni zincire TAŞINIR (re-tag).
4. Zincir, KENDİ etiketli sersem oyuncuları kalmayınca KAPANIR
   (ayılan/başka zincire geçen). Global sersem sayısı DEĞİL.
5. Sersemlik uzaması (delta-time): sersem biri yeniden vurulursa →
   yeni_sersemlik = kalan_sersemlik + taze_sersemlik. ChainID korunur.
6. Zincir kapanınca toplam puan ChainID.Owner'a yazılır (× hero combo çarpanı).
```

### Veri yapıları
```cpp
struct FChainLink {
    APlayerState* Instigator;   // bu halkayı kim üretti
    APlayerState* Victim;       // kim çarpıldı (varsa)
    EImpactType   Type;         // Fall / WorldHit / PlayerHit
    int32         Points;       // bu halkanın ham puanı
    float         Timestamp;
};

struct FChaosChain {
    int32                 ChainID;
    APlayerState*         Owner;            // zinciri başlatan (skill sahibi)
    TArray<FChainLink>    Links;
    TSet<APlayerState*>   ActiveStaggered;  // bu zincire ait HÂLÂ sersem olanlar
    int32                 LinkCount;        // player-hit sayısı (eskalasyon için)
    float                 StartTime;
    int32                 AccumulatedPoints;
};
```

`ChaosSubsystem` (GameState tarafı, server) açık zincirleri tutar:
```cpp
TMap<int32, FChaosChain> ActiveChains;
TMap<APlayerState*, int32> PlayerCurrentChain;  // oyuncu → ait olduğu ChainID
int32 NextChainID = 1;
```

---

## 3. Zincir Yaşam Döngüsü (adım adım — server)

```
OnSkillUsed(Player):
    ChainID = NextChainID++
    ActiveChains.Add(ChainID, FChaosChain{ Owner=Player, ... })
    PlayerCurrentChain[Player] = ChainID
    // skill kullanımının kendisi 0 puan (§4 tablo)

OnImpact(Instigator, Victim, Type):
    ChainID = ResolveChain(Instigator)      // Instigator hangi zincire ait?
    Chain   = ActiveChains[ChainID]
    pts     = ScoreImpact(Type, Chain.LinkCount)   // §4 formül
    Chain.Links.Add({Instigator, Victim, Type, pts})
    Chain.AccumulatedPoints += pts

    if (Type == PlayerHit):
        Chain.LinkCount++                   // eskalasyon
        ApplyStagger(Victim, ChainID)       // delta-time uzama §2.5
        ReTagVictim(Victim, ChainID)        // eski zincirden çıkar, buraya ekle
        Chain.ActiveStaggered.Add(Victim)

OnStaggerRecovered(Player):
    ChainID = PlayerCurrentChain[Player]
    Chain.ActiveStaggered.Remove(Player)
    if (Chain.ActiveStaggered.IsEmpty()):
        CloseChain(ChainID)                 // §5

CloseChain(ChainID):
    Chain = ActiveChains[ChainID]
    tier  = GetTier(Chain.LinkCount)        // §5 tier tablosu
    final = Chain.AccumulatedPoints
            * HeroComboMultiplier(Chain.Owner)   // hero stat (Rocket 1.0x)
            * tier.Bonus
    AddChaosScore(Chain.Owner, final)
    BroadcastComboCue(Chain.Owner, tier, final)  // GameplayCue (verimli, §6)
    ActiveChains.Remove(ChainID)
```

`ResolveChain(Instigator)`: Instigator `PlayerCurrentChain`'de varsa o ChainID;
yoksa (skill'siz serbest çarpışma — nadirdir) o anki en yakın bağlamdaki zincir
ya da küçük bir "orphan" zincir. **Kural:** skill kullanan her zaman bir zincir
sahibidir; sahipsiz çarpışma minimum puanlıdır.

---

## 4. Tekil Olay Puanları (senin tablon → formül)

```
Skill kullanımı (tetik)              :  0   (zincir açar, puan yok)
Self-fall (eylem yok, düşme ragdoll) : +1
World object hit (duvar/materyal)    : +5   (sersemlik yer)
Player hit (oyuncuya çarpma)         : +5  (çarpma)
   + verilen sersemlik                : +5
   + eskalasyon                       : +3 × LinkCount   (zincir uzadıkça artar)
```

`ScoreImpact(Type, linkCount)`:
```cpp
switch (Type) {
  case Fall:      return 1;
  case WorldHit:  return 5;
  case PlayerHit: return 5 + 5 + (3 * (linkCount + 1));  // +1: bu halka dahil
}
```

> Eskalasyon kritik: ilk player-hit `5+5+3=13`, ikinci `5+5+6=16`, üçüncü
> `5+5+9=19`... zincir uzadıkça her halka daha değerli. "MEGA CHAOS" hissi
> buradan ve tier bonusundan gelir.

---

## 5. Tier Sistemi (LinkCount → çarpan, data-driven)

```
LinkCount  Tier           Bonus   Cue
─────────  ─────────────  ─────   ──────────────
0–1        (tekil)        ×1.0    "+N"
2          Combo          ×1.0    "COMBO! +N"
3          Big Chaos      ×1.5    "BIG CHAOS! +N"
4+         MEGA CHAOS     ×3.0    "MEGA CHAOS!! +N"
```

Tier eşikleri ve bonuslar `DA_ChaosTuning` data asset'inde — playtest ile
kalibre edilir, kodda hardcode DEĞİL.

```cpp
struct FChaosTier { int32 MinLinks; float Bonus; FText CueLabel; };
// DA_ChaosTuning içinde TArray<FChaosTier>
```

---

## 6. Hero Combo Çarpanı (kimliği skora bağlar)

Her hero'nun bir `ComboMultiplier` stat'i vardır (HeroComponent, 03_Player):
```
Rocket  : 1.0x   (tek başına felaket, dengeli)
Magnet  : 1.5x   (işi başkalarını BİRBİRİNE bağlamak → combo ustası)
Giant   : 2.0x   (tek çarpışma bile büyük zincir başlatır)
Teleport: 1.0x   (öngörülemez, ama combo odaklı değil)
```
Bu, hero seçimini stratejiye bağlar (competence = sistem zekâsı, §1.3):
combo avcısı oyuncu Magnet ister. Değerler `DA_HeroTuning`'de, kalibre edilir.

Verimlilik: combo popup'ları **GameplayCue** ile yayılır (dedicated server'da
tetiklenmez, normal RPC'den ucuz). Skor verisi GameState'te replicate (herkese).

---

## 7. Awards — PUAN VERMEZ (sert kural)

Awards maç sonu izlenen istatistiklerden hesaplanır ama **sıralamaya girmez.**
Saf bilgilendirici tanıma: maçın hikâyesini anlatır, kazananı belirlemez.

```
İzlenen istatistikler (server, maç boyunca):
  - ToplamChainLink (combo katkısı)
  - DüşmeSayısı / ragdoll süresi
  - HavadaGeçenSüre
  - Verilen/yenen sersemlik
  - Sabote edilme miktarı (objective progress kaybı)

Award atama (maç sonu, her oyuncuya MUTLAKA bir başlık):
  En Çaotik       → en yüksek ChaosScore
  Biggest Fail    → en çok kendi kendine düşen
  Most Air Time   → en uzun havada
  En Çok Sabote Edilen → en çok progress kaybeden  ← teselli (relatedness)
  Accidental Hero → skill'siz en çok combo katkısı (kazara)
```

**Neden puansız:** "En Çok Sabote Edilen"e puan verirsen oyuncu bilerek sabote
olur (farming). Puansız tanıma "kaybeden bile güler" amacına temiz ulaşır ve
manipüle edilemez. Bu psikolojik bir zorunluluktur, tercih değil. (§1.1)

---

## 7.5 Competence Floor + Best Moments (00_Felsefe §1.2 — SERT KURAL)

Hero'lar bozuk olduğu için yeni oyuncu çaresizlik hissedip churn edebilir.
Competence iki katmanla garanti edilir:

**Maç içi — garantili "ben yaptım" anı:** her oyuncu her maç EN AZ bir kez
güvenilir şekilde *bir şey başarabilmeli*: bir combo başlatmak (ChainID açmak
zaten her skill'de olur) veya bir minik side görev. Bu yüzden side/chaos
havuzunda **en az bir "düşük eşikli, kolay tetiklenen" görev** her oyuncuya
ulaşılabilir olmalı. Çıktı belirsiz olabilir ama imkânsız olmamalı.

**Maç sonu — Best Moments (PUBG Mobile modeli):** ChaosSubsystem zaten her
chain'i ve impact'i kaydediyor (§2 `FChaosChain.Links`). Maç sonu, her oyuncunun
**en yüksek puanlı anının** replay/klibini göster:
```cpp
FChainLink* GetBestMoment(APlayerState* p):
    return en yüksek (Chain.AccumulatedPoints) olan, p'nin Owner olduğu chain
```
Bu hem competence tatmini ("ben bunu yaptım!") hem paylaşılabilir viral klip
(relatedness + acquisition). **Floor o anı yaşatır, Best Moments kutlar.**

---

## 8. End-Match Reveal (relatedness motoru)

B noktasında (süre dolar) en güçlü retention anı çalışır:

```
Maç sonu sırası:
  1. Gizli görevler HERKESE açıklanır (artık OwnerOnly değil).
  2. Her oyuncunun görevi + progress%'i gösterilir.
  3. "Mesaj sistemi" her görevi bir cümleye çevirir (§9).
  4. Awards başlıkları dağıtılır (puansız).
  5. Final sıralama (Objective + Chaos).
```

Komedi payoff'u burada: "P3 bütün maç trambolinin ayaklarını kesmeye çalışıyormuş,
o yüzden P4 hiç zıplayamamış." Bu paylaşılan kahkaha = oyuncuların tekrar
oynama sebebi. Reveal'i hızlı, görsel ve komik tut — sıkıcı tablo DEĞİL.

---

## 9. "Bir Boka Yaramadın Ama..." Mesaj Sistemi

Tamamlanamayan görev için progress'i empatik/komik bir cümleye çevir. Bu
sistem **görev tipi + progress bucket** ile templated:

```cpp
FText BuildResultMessage(ObjectiveBase* obj):
    p = obj->GetProgress();
    bucket = (p==0)?Zero : (p<0.5)?Low : (p<1.0)?High : Complete;
    return Template[obj->Type][bucket];   // DA_Messages data asset
```

Örnek (Carry "ayıyı götür"):
```
Complete : "Ayıyı çocuğa ulaştırdın. Kahramansın. (nadiren)"
High     : "Ayıyı neredeyse götürüyordun ama biri çaldı. Çocuk yine de gülümsedi."
Low      : "Bir boka yaramadın ama en azından ayıyı 2 metre taşıdın."
Zero     : "Ayıya hiç dokunamadın bile. Çocuk hâlâ ağlıyor. Tebrikler."
```

Mesajlar `DA_Messages`'te, görev tipine göre — yeni görev = yeni şablon, kod değil.

---

## 10. ChaosSubsystem — Konum & Otorite

```
GameState (replicated → all)
 └─ ChaosSubsystem (server-authoritative)
     ├─ ActiveChains : TMap<ChainID, FChaosChain>
     ├─ PlayerCurrentChain : TMap<PlayerState, ChainID>
     ├─ PlayerChaosScore : TMap<PlayerState, int32>   (replicate)
     ├─ MatchStats : per-player award istatistikleri
     └─ OnImpact / OnSkillUsed / OnStaggerRecovered  (event girişleri)
```

Component'ler (03_Player) sadece olay ATAR:
```
HeroComponent  → OnSkillUsed (Event.Skill.Used)
MovementComp   → OnImpact (Event.Impact.PlayerHit / WorldHit / Fall)
               → OnStaggerRecovered (Event.Penalty.Recovered)
```
**Combo değerlendirmesi tek oyuncuda OLAMAZ** (birden çok oyuncunun etkileşimi
gerekir) → bu yüzden merkezde, GameState tarafında. (01_Ana_Mimari §8 ile tutarlı.)

---

## 11. Çalışılmış Örnek (gerçek sayılarla)

```
Oyuncular: P1=Rocket(1.0x), P2, P3.

t=0.0  P1 skill kullanır       → ChainID_A açılır, Owner=P1. (0 puan)
t=0.5  P1 uçar, P2'ye çarpar   → PlayerHit, LinkCount 0→1
                                  pts = 5+5+(3×1) = 13.  P2 sersem(ChainA).
t=1.1  Sersem P2 savrulup P3'e → PlayerHit (istemsiz, ChainA yayılır), LC 1→2
                                  pts = 5+5+(3×2) = 16.  P3 sersem(ChainA).
t=1.6  P3 duvara çarpar        → WorldHit, pts = 5.
t=4.x  P2 ayılır               → ChainA.ActiveStaggered = {P3}
t=4.y  P3 ayılır               → ChainA.ActiveStaggered = {} → CloseChain

Zincir A:
  AccumulatedPoints = 13 + 16 + 5 = 34
  LinkCount = 2 → Tier "Combo" ×1.0
  Final = 34 × 1.0 (hero) × 1.0 (tier) = 34 → P1.ChaosScore += 34
  Cue: "COMBO! +34" herkese.

Eğer P1 Magnet olsaydı (1.5x): 34 × 1.5 = 51.
Eğer LinkCount 4 olsaydı: tier ×3.0 → MEGA CHAOS, çok daha büyük.
```

---

## 12. Bu Dokümanda Kilitlenenler
- İki skor hattı: Objective + Chaos; final = ikisinin toplamı, Awards dışında.
- **ChainID modeli:** skill = yeni zincir; istemsiz çarpışma mevcut zinciri yayar;
  zincir KENDİ sersemleri kalmayınca kapanır; puan Owner'a (× hero çarpanı).
- Sersemlik delta-time ile uzar; oyuncu tek zincire ait, yeniden vurulursa taşınır.
- Tekil puanlar + eskalasyon (3×LinkCount) + tier bonus (data-driven).
- Hero combo çarpanı kimliği skora bağlar (Magnet combo ustası).
- **Awards puansız** (overjustification) — bilgilendirici tanıma, sıralama dışı.
- Reveal = relatedness motoru; mesaj sistemi templated.
- ChaosSubsystem GameState'te, server-authoritative, olay bazlı (tick yok).
- Tüm sayılar `DA_ChaosTuning` / `DA_HeroTuning` / `DA_Messages`'te — playtest kalibresi.

## 13. Açık Kararlar
- [ ] Orphan çarpışma (skill'siz, zincirsiz) puanlaması: 0 mı, minimum mu?
- [ ] Aynı kurbana tekrarlı çarpışma spam'i: puan cap'i / diminishing var mı?
- [ ] Objective/Chaos %50/40 oranı: ham puan tavanlarıyla mı dengelenecek?
- [ ] Tier eşikleri ve hero çarpan değerleri → playtest kalibrasyonu.
