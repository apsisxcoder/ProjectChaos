# PROJECT CHAOS — OBJECTIVE SİSTEMİ

> Doküman seti: `00` → `01` → `02_Scenario` → `03_Player` → `04_Trait` → **`05_Objective`** → ...
> Ön koşul: 02 (görev havuzları), 03 (ObjectiveComponent, OwnerOnly), 04 (trait penalty kavramı).
> Bu doküman görev sisteminin **teknik kalbidir** — Scenario ve Player buna referans verir.

---

## 1. Temel İlke: Her Şey Olay Bazlı (per-frame tick YOK)

Görev takibi lag/kasma kaynağı **değildir** — yeter ki her frame "bölgede miyim?"
diye sormayalım. Lag fizikten/ragdoll'dan gelir (01_Ana_Mimari). Görev tarafında
kural:

```
Counter / Carry / Transport  → GAS GameplayEvent (olay gelince işle)
Hold / Prevent               → Overlap begin/end + DÜŞÜK FREKANSLI timer
                               (per-frame tick değil; ~4–10 Hz süre birikimi)
Progress (float 0–1)         → OwnerOnly, DEĞİŞİNCE replicate (her frame değil)
```

Overlap begin/end, "state"i event'e çevirir: trambolinin overlap volume'üne biri
*girince* `OnBeginOverlap`, *çıkınca* `OnEndOverlap` ateşler — arada tick yok.
Süre birikimi düşük frekanslı bir timer'da hesaplanır (göz farkı görmez, CPU 10×
ucuzlar). 8 oyuncu × 4 görev = 32 float; ihmal edilebilir yük.

---

## 2. Ortak Sözleşme — `ObjectiveBase` (soyut)

Tüm arketipler tek bir soyut sözleşmeden türer. Üst sistem (ObjectiveComponent,
scoring) **hangi arketip olduğunu bilmez**, sadece bu sözleşmeyle konuşur.

```
ObjectiveBase  (abstract)
 ├─ GetProgress() : float 0.0–1.0      ← herkes bunu döndürür (skor bunu okur)
 ├─ OnGameplayEvent(Tag, Context)      ← GAS event'ini nasıl işlerim
 ├─ OnPenalty(PenaltyType)             ← sersemleme/düşme gelince ne olur
 ├─ OnSteal(Instigator)               ← çalınma olunca ne olur (varsa)
 ├─ Weight : int                       ← görev ağırlığı (skor çarpanı)
 └─ TargetValue / ZoneRef / ... : data (arketipe göre parametreler)
```

**Modülerlik garantisi:** 6. arketip eklemek = yeni alt sınıf yaz + base'i
implement et + kayıt et. Mevcut 5 arketip, scoring, ObjectiveComponent — hiçbiri
değişmez. "Modüler artırırız" sözünün teknik karşılığı budur.

---

## 3. Skor Sözleşmesi

```
ObjectiveScore = GetProgress() × Weight × BasePoints
```

- **Progress-based:** yarım kalan iş puan alır (9/10 zıpla = %90).
- **Weight:** kolay görev=1, zor görev=3 gibi. "10 kez zıpla" 5 sn'de biter,
  "çatıya taşı" belki hiç bitmez — eşit puan adil değil, weight bunu dengeler.
- Tamamlanamayan görev: kalan progress kadar puan + **Chaos puanı** +
  "bir boka yaramadın ama çocuğu şu kadar sevindirdin" mesajı (06_Chaos_Scoring.md).

---

## 4. Progress Yönü (kritik ayrım)

| Yön | Anlamı | Arketipler |
|-----|--------|------------|
| **Tek yönlü (kaybedilemez)** | Kazandığın progress gitmez; penalty *yavaşlatır*, sıfırlamaz | Counter, Transport |
| **Donabilir** | Durum bozulunca birikim durur ama düşmez | Hold, Carry |
| **İki yönlü (kaybedilebilir)** | Rakip ihlal ettirdikçe progress DÜŞER | Prevent |

Tasarım kuralı: **"yap" görevleri progress'i kaybetmez, "engelle" görevleri
kaybedebilir.** Düşmek seni geciktirir, silmez → sabotaj "öldürücü" değil
"geciktirici" olur, ki kaos oyununda istediğimiz tam bu.

Trait/skill penalty (04_Trait, senin "%1 azalır" örneğin): `OnPenalty` ile gelir.
Çoğu arketipte birikimi **dondurur**; Carry'de ek olarak çalınma progress'i
**transfer** edebilir.

---

## 5. Beş Arketip

### 5.1 Counter — "N kez yap"
```
Event kaynağı : GAS GameplayEvent (örn. Event.Trampoline.Jump)
Progress      : count / TargetValue        (tek yönlü)
OnPenalty     : birikmez ama kaybetmez (sayı durur)
OnSteal       : —
Örnek         : "10 kez zıpla", "trambolinin ayaklarını 3 kez kes"
```

### 5.2 Hold — "bir bölgede/durumda kal" (bayrak kapmaca)
```
Event kaynağı : Overlap begin/end + düşük frekanslı timer
Progress      : biriken_süre / TargetDuration   (donabilir)
OnPenalty     : bölgeden çıkarsa/düşerse birikim DONAR (sıfırlanmaz)
OnSteal       : —
Örnek         : "trambolinin üstünde toplam 10 sn dur / koru"
```

### 5.3 Carry — "obje taşı, çalınabilir" (tavuk kovalamaca)
```
Event kaynağı : GAS event (PickUp / Drop / Steal) + taşıma süresi/mesafesi
Progress      : taşıma ilerlemesi (hedefe yaklaşım veya süre)  (donabilir)
OnPenalty     : obje düşer, progress DONAR
OnSteal       : obje el değiştirir → progress karşı tarafa TRANSFER (senin örneğin)
Örnek         : "ayıyı çocuğa götür", "ayıyı başka oyuncuya ver"
```

### 5.4 Prevent — "bir şey olmasın" (iki yönlü)
```
Event kaynağı : Overlap begin/end + ihlal süresi timer
Progress      : ihlalsiz_süre / toplam_süre    (İKİ YÖNLÜ — düşebilir)
OnPenalty     : — (zaten ihlalle düşüyor)
OnSteal       : —
Örnek         : "kimse trambolinde olmasın", "ayı suya girmesin"
Not           : oransal ölçüm — tek saniyelik ihlal görevi öldürmez, oranı düşürür
```

### 5.5 Transport — "objeyi/kendini bir yere götür" (çalınamaz)
```
Event kaynağı : GAS event (hedef Zone overlap) + mesafe normalize
Progress      : 1 - (kalan_mesafe / başlangıç_mesafe)   (tek yönlü)
OnPenalty     : obje düşerse yerinde kalır, progress korunur
OnSteal       : — (Carry'den farkı: çalınamaz, sadece konum)
Örnek         : "trambolini çatıya taşı", "çöp kutusunu trambolinin üstüne dök"
```

---

## 6. GameplayTag Event Omurgası (isimlendirme)

Görev mantığı ability mantığından ayrıdır: ability tag broadcast eder, görev
dinler. İsimlendirme konvansiyonu (örnek):

```
Event.Trampoline.Jump          ← zıplama (Counter besler)
Event.Object.Interact          ← kes / balon bağla / minder koy (Counter)
Event.Item.PickedUp            ← obje alındı (Carry)
Event.Item.Dropped             ← obje düştü (Carry penalty)
Event.Item.Stolen              ← obje çalındı (Carry steal → transfer)
Event.Zone.Reached             ← hedef bölgeye varış (Transport)
Event.Penalty.Stagger          ← sersemleme (OnPenalty tetikler)
Event.Penalty.Knockdown        ← düşme/ragdoll (OnPenalty tetikler)
```

Overlap-driven arketipler (Hold/Prevent) tag yerine doğrudan overlap volume'den
beslenir — ama isterse aynı omurgaya `Event.Zone.Entered/Exited` olarak da
yansıtılabilir (tek dil). Yeni görev = yeni tag + arketip + parametre, **kod değil.**

---

## 7. Trambolin Havuzu → Arketip Dağılımı (20 Primary)

| # | Görev | Arketip | Weight (taslak) |
|---|-------|---------|-----------------|
| 1 | Ayıyı çocuğa götür | Carry | 2 |
| 2 | Kimse trambolinde olmasın | Prevent | 3 |
| 3 | Trambolinin ayaklarını kes | Counter | 2 |
| 4 | Trambolinde 10 kez zıpla | Counter | 1 |
| 5 | Trambolini en yükseğe taşı | Transport | 3 |
| 6 | Ayıyı suya at (yıka) | Carry | 2 |
| 7 | Çöp kutusunu trambolinin üstüne dök | Transport | 2 |
| 8 | Tramboline ikinci obje getir | Transport | 1 |
| 9 | Trambolini hareketsiz tut (kimse zıplamasın) | Prevent | 2 |
| 10 | Ayıyı yüksek rafa sakla | Carry | 2 |
| 11 | Ayıyı çocuğa battaniye gibi getir | Carry | 1 |
| 12 | Trambolinin etrafını minderle | Counter | 1 |
| 13 | Trambolini roketle fırlat | Counter | 3 |
| 14 | Trambolini parkın dışına it | Transport | 3 |
| 15 | Trambolini süsle (balon bağla) | Counter | 1 |
| 16 | Ayıyı kuru tut (suya girmesin) | Prevent | 2 |
| 17 | Ayıyı başka oyuncuya ver | Carry | 1 |
| 18 | Birini havaya fırlat | Counter | 2 |
| 19 | Trambolini koru / tamir et | Hold | 2 |
| 20 | Ayıyı çocuğa hızlıca fırlat | Carry | 1 |

Dağılım: Carry ×6, Counter ×6, Transport ×4, Prevent ×3, Hold ×1.
(Beş arketip de temsil ediliyor; weight'ler playtest ile kalibre edilir.)

> Çelişki ağı (02_Scenario §5) burada da geçerli: #2 Prevent ile #4 Counter
> birbirini iptal eder; #1 Carry ile #10 Carry aynı objeyi ters yöne taşır
> (steal/transfer mekaniği tam burada devreye girer).

---

## 8. ObjectiveComponent ile İlişki (03_Player)

```
ObjectiveComponent (PlayerState, OwnerOnly)
 ├─ AssignedObjectives[] : TArray<ObjectiveBase>  (1 Primary + 2 Side + 1 Chaos)
 ├─ Faz 1'de doldurulur (lobide, scenario havuzundan)
 ├─ GAS event'lerini ilgili objective'lere yönlendirir
 └─ her objective'in GetProgress()'ini skor sistemine raporlar
```

Penalty akışı: `Event.Penalty.*` geldiğinde ObjectiveComponent aktif objective'in
`OnPenalty()`'sini çağırır. Steal akışı: `Event.Item.Stolen` → ilgili Carry
objective `OnSteal()` → progress transfer.

---

## 9. Bu Dokümanda Kilitlenenler
- Tek soyut sözleşme `ObjectiveBase` (GetProgress 0–1 + event + penalty + steal).
- 5 arketip: Counter, Hold, Carry, Prevent, Transport — modüler, artırılabilir.
- Her şey olay bazlı: GAS event + overlap begin/end + düşük frekanslı timer.
  **Per-frame tick yok.** Progress OwnerOnly, değişince replicate.
- Progress yönü: "yap" kaybetmez, "engelle" kaybedebilir; Carry steal→transfer.
- Skor = progress × weight × base; yarım iş puan alır.
- 20 trambolin görevi arketiplere dağıtıldı.

## 10. Açık Kararlar
- [ ] Weight skalası kesin değerleri (1–3 mü, 1–5 mi?) → playtest.
- [ ] Carry steal: progress %100 mü transfer olur, yoksa bir kısmı mı kaybolur?
- [ ] Hold/Prevent timer frekansı (4 Hz mi 10 Hz mi?) → performans testi.
- [ ] Side/Chaos görevleri de aynı 5 arketibe mi düşüyor? (büyük ihtimalle evet)
