# PROJECT CHAOS — SCENARIO SİSTEMİ

> Doküman seti: `00_Tasarim_Felsefesi` → `01_Ana_Mimari.md` → **`02_Scenario.md`** → `03_Player.md` → ...
> Ön koşul: 01_Ana_Mimari.md okunmuş olmalı (otorite & replikasyon modeli oradan gelir).

---

## 1. Scenario Nedir

Scenario, oyunun **içerik birimidir**. Elle kurgulanmış sabit bir iskelet +
random doldurulan görev havuzlarından oluşur.

```
İskelet (authored, sabit)        Havuzlar (random çekilir)
─────────────────────────        ──────────────────────────
Shared Object (trambolin)        PrimaryPool[~20]  → her oyuncu 1 çeker
NPC (ağlayan çocuk)              SidePool          → her oyuncu 2 çeker
Cinematic (tek shot)             ChaosPool         → her oyuncu 1 çeker
A → B yaşam döngüsü
```

İskelet **okunabilirliği** verir (izleyici tek objede toplanmış kaosu anlar).
Havuzlar **tekrar oynanabilirliği** verir (her maç farklı görev dağılımı).
Bir Scenario izole test edilebilir bir komedi makinesidir — "bu senaryo
yabancıyı güldürüyor mu?" diye tek başına denenebilir.

---

## 2. Üç Değişmez Tasarım Kuralı

Bu kurallar Scenario'nun ruhudur. İhlal edilirse oyun sıradanlaşır.

### Kural 1 — Çatışma çözülmez. Rekabet ön plandadır.
Görevler **bilerek birbirini iptal eder.** "Trambolinin ayaklarını kes" ile
"trambolinde 10 kez zıpla" aynı anda gelebilir ve biri diğerini imkânsızlaştırır.
Bu bir hata değil, **tasarımın kendisidir.** Havuzu "hep çözülebilir" yapmaya
ÇALIŞMAYACAĞIZ. (bkz. `SolvabilityFlag = NotSolvable`, §6)

İmkânsızlık komedidir: maç sonunda "Sen ne yapıyordun?" / "Çöp kutusu
taşıyordum." / "Neden?" / "Gizli görevdi." — payoff buradadır. Bu yüzden skor,
**görevini tamamlayamayanı da ödüllendirmeli** (örn. "En Çok Sabote Edilen").

### Kural 2 — Görevler "yardım" diye sunulur, "sabotaj" diye değil.
Oyuncuya asla "trambolini boz" demeyiz. "Çocuğun boyu yetmiyor, ona yardım et"
deriz — oyuncu da ayakları keser. Niyet hep iyidir, **yöntem absürttür.**
Oyuncu kendini kötü adam değil, **yanlış anlamış bir kahraman** gibi hisseder.
İyi niyet ile saçma yöntem arasındaki uçurum komedinin kalbidir.

### Kural 3 — Komedi Scenario'da DEĞİL, Hero'da yaşar.
Scenario sadece oyuncuları aynı alana çelişen sebeplerle toplar (saf hedef).
Oyuncunun o hedefe giderken **rezil olması** Hero katmanından gelir (bozuk
skill → ragdoll/stagger). Bu yüzden:

> **Objective katmanı ragdoll'dan habersizdir.** Görevlerde "savunmasız ol",
> "şurada düş" gibi fiziksel ceza KODLANMAZ. Görev temiz bir hedeftir. Düşme,
> savrulma, sersemleme tamamen Hero/Movement katmanının işidir (bkz. 03_Player.md).

Mimari özet:
> **Komedi hero'da · Çatışma objective'de · Kaos ikisinin çarpışmasında.**

---

## 3. A → B Yaşam Döngüsü

Her Scenario'nun bir başlangıç durumu (A) ve bir bitişi (B) vardır.

```
A noktası: Başlangıç durumu kurulur.
           (trambolin park ucunda, çocuk ağlıyor, peluş ayı trambolin üstünde)
   │
   │  ← oyuncular bu süre boyunca görevlerini yapmaya çalışır
   │     ve birbirini doğal olarak sabote eder
   ▼
B noktası: Süre dolar. Görev durumu değerlendirilir, skor verilir,
           gizli görevler herkese AÇIKLANIR (komedi payoff'u).
```

**Sinematik = tek establishing shot.** Kamera çocuğa zoom, çocuk ağlama
animasyonu, ekrana oyuncunun **Primary** görevi yazılır
(örn. *"Çocuğun ayısına kavuşmasına yardım et"*). Her görev için ayrı cutscene
YOK. Sinematik bir *kamera + metin sistemidir*, animasyon filmi değil.

**Tur süresi:** hero kontrol zorluğuna bağlıdır. Karakterler bozuk olduğu için
basit bir "tramboline git" eylemi bile dakikalar sürebilir (düz çizgide
yürüyemiyor). Hedef ~5–10 dk; **kesin süre playtest ile kalibre edilir**, masada
kilitlenmez.

---

## 4. Görev Dağıtımı

Maç başında **server** (GameMode) her oyuncuya dağıtır:

```
1 × Primary  (PrimaryPool'dan random, çelişebilir)
2 × Side     (SidePool'dan random)
1 × Chaos    (ChaosPool'dan random, doğrudan sabotaj)
```

- Kimse ne çekeceğini önceden bilmez.
- Kimse başkasının görevini göremez → `COND_OwnerOnly` replikasyon (01_Ana_Mimari §4).
- Görevler GameplayTag event'leriyle takip edilir (detay: 05_Objective.md).

**Anti pile-on (SERT KURAL — 00_Felsefe §5):** Hedefli görevler (chaos/side,
örn. "şu oyuncunun elindekini çal", "3 oyuncuyu düşür") dağıtılırken **aynı
hedef oyuncuya 2'den FAZLA görev atanamaz.** Server Faz 1'de bir hedef-sayacı
tutar; bir oyuncu zaten 2 görevin hedefiyse, yeni hedefli görev başka oyuncuya
yönlendirilir. Sebep: 8 kişilik free-for-all'da tek oyuncu herkesin hedefi
olursa zorbalık hisseder ve churn eder (fizik kaosu chain ile zaten dağılıyor,
ama kasıtlı görev hedeflemesi bu kuralla dağıtılır).

---

## 5. ÖRNEK SCENARIO — "Trambolin"

### İskelet
- **SharedObject:** Trambolin (park ucunda)
- **NPC:** Ağlayan çocuk
- **Prop:** Peluş ayı (başlangıçta trambolinin üstünde)
- **Cinematic:** kamera çocuğa zoom + ağlama anim + Primary metni

### PrimaryPool (örnek ~20, hepsi "yardım" çerçevesinde, çelişkili)

| # | Sunulan sebep (oyuncu görür) | Absürt yöntem (gerçek görev) | Çatışır |
|---|------------------------------|------------------------------|---------|
| 1 | Çocuk ayısına kavuşsun | Ayıyı çocuğa götür | 10, 17 |
| 2 | Çocuk trambolini çok seviyor | Kimse üstüne çıkmasın (koru) | 4, 15, 18 |
| 3 | Çocuğun boyu yetmiyor | Trambolinin ayaklarını kes / alçalt | 19 |
| 4 | Çocuk nasıl oynanacağını bilmiyor | Trambolinde 10 kez zıpla (göster) | 2, 9 |
| 5 | Çocuk yükseklik seviyor | Trambolini en yüksek noktaya taşı | 14 |
| 6 | Ayı kirlendi, çocuk iğrendi | Ayıyı suya/çeşmeye at (yıka!) | 16 |
| 7 | Çocuk çöpü görmesin | Çöp kutusunu trambolinin üstüne devir (ört) | 2 |
| 8 | Çocuk yalnız | Tramboline ikinci bir obje/NPC getir | — |
| 9 | Çocuk gürültüden korkuyor | Trambolini hareketsiz tut (kimse zıplamasın) | 4, 18 |
| 10 | Çocuk ayıyı kaybetmesin | Ayıyı güvenli yere (yüksek rafa) sakla | 1, 17 |
| 11 | Çocuk üşüyor | Ayıyı çocuğa sarıl-battaniye gibi getir | — |
| 12 | Çocuk düşmesin | Trambolinin etrafını minderle yumuşat | — |
| 13 | Çocuk heyecanlansın | Trambolini roketle fırlat (tehlikeli "yardım") | 2, 12 |
| 14 | Çocuk temiz hava istiyor | Trambolini parkın dışına it | 5 |
| 15 | Çocuğun doğum günü | Trambolini süsle (üstüne çık, balon bağla) | 2 |
| 16 | Ayı ıslanmasın | Ayıyı kuru tut, sudan uzak tut | 6 |
| 17 | Çocuk paylaşmayı öğrensin | Ayıyı başka bir oyuncuya ver | 1, 10 |
| 18 | Çocuk en yükseği görsün | Birini süper-zıplatıp havaya fırlat | 2, 9 |
| 19 | Trambolin bozulmasın | Trambolini koru / tamir et | 3 |
| 20 | Çocuk hemen sussun | Ayıyı çocuğa hızlıca fırlat (kafasına da gelebilir) | — |

> Dikkat: çelişki tek yönlü değil, **çok yönlü bir ağ.** P1 ayıyı götürmeye
> çalışırken P10 ayıyı saklamaya, P2 trambolini korurken P4 üstünde zıplamak
> zorunda. Herkes aynı 2 m²'de birbirinin doğal düşmanı — ama silah yok, sebep
> trambolin.

### SidePool (örnek — küçük, dünyaya/başkalarına yönelik)
- 3 farklı oyuncuyu trambolinden düşür
- Bir oyuncuyu çeşmeye it
- Masadaki tabak-çanağı dağıt
- Bir oyuncuyu 5 sn stagger'da tut

### ChaosPool (örnek — doğrudan sabotaj)
- Ayıyı taşıyan oyuncudan ayıyı çal
- Bir oyuncuyu harita dışına at (ring-out)
- İki oyuncuyu birbirine çarptır
- Trambolini ateşe ver / devre dışı bırak

---

## 6. DA_Scenario — Data Yapısı (taslak)

```
DA_Scenario
 ├─ ScenarioName      : FName        ("Trampoline")
 ├─ SharedObjectClass : TSubclassOf  (spawn edilecek paylaşılan obje)
 ├─ NPCClass          : TSubclassOf  (establishing shot öznesi)
 ├─ PropClasses[]     : TArray       (ayı vb. başlangıç objeleri)
 ├─ Cinematic
 │    ├─ CameraTarget : NPC | SharedObject
 │    ├─ NPCAnim      : UAnimMontage
 │    └─ (primary metni runtime'da atanan görevden gelir)
 ├─ SpawnLayout       : A noktası kurulum verisi (transform'lar)
 ├─ PrimaryPool[]     : TArray<DA_Objective>   (~20, çelişebilir)
 ├─ SidePool[]        : TArray<DA_Objective>
 ├─ ChaosPool[]       : TArray<DA_Objective>
 └─ SolvabilityFlag   : NotSolvable   ← KİLİTLİ (Kural 1)
```

`DA_Objective`'in iç yapısı 05_Objective.md'de tanımlanır. Scenario sadece
ona referans tutar; görevin nasıl çalıştığını bilmez (decoupling).

---

## 7. Scenario ↔ Hero ↔ Trait — Tam Resim

Üç katman birbirinden habersiz çalışır; çarpışınca emergent kaos çıkar.

```
Scenario   → oyuncuları aynı alana çelişen sebeplerle toplar (saf hedef)
Hero (Q/E) → oyuncuyu hedefe giderken rezil eder (bozuk skill, ragdoll)
Trait      → pasif slotu doldurur (karma scope), kaosu çeşitlendirir
```

### Örnek maç akışı
> **P1:** Roket Hero + *Kayan Ayaklar* (global trait) — Primary: "Çocuğun ayısına
> kavuşmasına yardım et."
> **P2:** Roket Hero + *İniş Patlaması Güçlenir* (hero-locked trait) — Primary:
> "Kimse trambolinin üstüne çıkmasın."
>
> P1 ayıyı almak için tramboline gider; roket düz fırlar (yön yok), kayan ayaklar
> binince varışta duramaz, trambolinden kayıp düşer → ragdoll. P2 trambolini
> korur, E'sini patlatır; güçlü impact P1'i havaya savurur, ayı uçar, ikisi de
> yere serilip stagger'a girer. Çocuk hâlâ ağlar. Üçüncü oyuncu olanı anlamadan
> oraya yürürken P1'in savrulan bedenine çarpıp o da sendeler.

Hiçbir yerinde "bu kombinasyon için" özel kod yok. Üç bağımsız sistem aynı
karakterde çalışıp sonucu kendileri üretti. Bu, solo geliştirici için sürekli
yeni senaryo üretmenin en ucuz yolu.

---

## 8. Yeni Scenario Eklemek (içerik üretim akışı)

1. Bir **paylaşılan obje** seç (klip filtresinden geçsin: ekranda tek bakışta
   anlaşılır mı? trambolin ✓, görünmez enerji alanı ✗).
2. Bir **NPC + sebep** kur (ağlayan çocuk + ayı).
3. PrimaryPool'a ~20 **çelişen "yardım" görevi** yaz (yöntemler absürt olsun).
4. SidePool + ChaosPool'u doldur.
5. Sinematik için kamera hedefi + NPC anim seç (yeni film çizme).
6. İzole test et: yabancı 10 sn'lik klibe gülüyor mu?

Yeni Scenario = çoğunlukla **data**, kod değil. Hedef: 1 yeni obje + havuz =
yüzlerce yeni emergent maç.

---

## 9. Bu Dokümanda Kilitlenenler
- Çatışma çözülmez (`SolvabilityFlag = NotSolvable`).
- Görevler "yardım" çerçevesinde sunulur, absürt yöntemle.
- Komedi Hero'da; Objective katmanı ragdoll'dan habersiz.
- Dağıtım: 1 Primary + 2 Side + 1 Chaos, OwnerOnly.
- Sinematik = tek shot + metin sistemi.
- Tur süresi playtest ile kalibre (~5–10 dk, kilitli değil).

## 10. Açık Kararlar (Scenario'ya özel)
- [ ] Skor, görev zorluğuna/büyüklüğüne göre mi verilecek? ("10 kez zıpla" 5 sn'de
      biter, "çatıya taşı" belki hiç bitmez — eşit puan adil değil.)
- [ ] "En Çok Sabote Edilen" gibi telafi ödülleri tam liste (→ 06_Chaos_Scoring.md).
- [ ] Side/Chaos görevleri de scenario'ya mı özel, yoksa genel havuzdan mı?
