# PROJECT CHAOS — TASARIM FELSEFESİ (DEĞİŞMEZ İLKELER)

> **Bu, setin EN ÜST dokümanıdır. Her şeyden önce bu okunur.**
> Doküman seti: **`00_Tasarim_Felsefesi`** → `01_Ana_Mimari` → `02_Scenario`
> → `03_Player` → `04_Trait` → `05_Objective` → `06_Chaos_Scoring`
>
> Buradaki ilkeler **psikolojik temellidir** (SDT, overjustification, variable
> reward, loss aversion) ve oyunun uzun vadeli hayatta kalmasını belirler.
> Bir kod/tasarım kararı bu ilkelerle çelişirse — **ilke kazanır.**

---

## 0. Tek Cümlelik Vizyon

Oyuncular birbirini öldürmez; **aynı dünya objesi üzerinde, çelişen gizli
görevleri "yardım" diye yapmaya çalışırken bozuk güçleriyle rezil olur.**
Kazanan en çok kill yapan değil, en çok **paylaşılabilir kaos hikâyesi** üreten.

Tek metrik her şeyin üstünde:
> *Oyunu hiç bilmeyen biri 10 sn'lik klibe gülüyor mu?*

---

## 1. Üç Psikolojik İhtiyaç (SDT — retention'ın motoru)

Araştırma: autonomy + competence + relatedness, gelecekteki oynama
motivasyonunu doğrudan yordar. Üçü de beslenmezse oyuncu churn eder.

### 1.1 Autonomy (✓ doğal güçlü)
Gizli görevler + random hero/trait + "nasıl yardım edeceğine sen karar ver".
Oyuncu kendi hikâyesinin sahibi. **Koru:** asla tek doğru yol dayatma.

### 1.2 Competence (⚠️ en kırılgan — iki katmanlı korunur)
Hero'lar bilerek bozuk → kontrol ustalığı YOK. Bu casual için iyi (düşük stres)
ama competence ihtiyacını aç bırakma riski taşır. İki katmanla beslenir:

- **Competence FLOOR (maç içi, garantili):** her oyuncu her maç **en az bir
  "ben yaptım!" anı** yaşamalı — bir combo başlatmak veya bir minik görev.
  Çıktı belirsiz olabilir ama oyuncu *bir şeyi* güvenilir başarabilmeli.
  Yoksa yeni oyuncu saf çaresizlik hisseder ve gider.
- **Best Moments (maç sonu, kutlama):** PUBG Mobile modeli. ChaosSubsystem
  zaten her chain/impact'i kaydediyor → oyuncunun en yüksek puanlı anının
  klibi/replay'i maç sonu gösterilir. Hem "ben yaptım" tatmini hem
  paylaşılabilir viral klip. **Floor o anı yaşatır, Best Moments kutlar.**
- **Derinlik (rekabetçi oyuncu için):** ustalık kontrol değil **sistem zekâsı**
  — combo okuma, rakip görevini tahmin, hero combo çarpanı stratejisi.
  Böylece hem çocuk hem rekabetçi aynı anda tutulur.

### 1.3 Relatedness (✓ retention'ın ASIL motoru — en çok yatırım buraya)
Among Us'ı Fall Guys'a üstün kılan şey buydu (sosyal hikâye > fizik komedisi).
Beslendiği yerler: aynı obje üzerinde çatışma + **maç sonu reveal** + paylaşılan
kahkaha. Reveal iki iş birden yapar: relatedness + onboarding (§4).

---

## 2. Ödül Psikolojisi (overjustification — SERT KURAL)

İçsel olarak komik olan bir şeyi dışsal ödülle (puan) ödüllendirmek, onu **"işe"
çevirir** ve içsel motivasyonu yok eder ("crowding out"). Sonuç: farming + sahtelik.

- **Awards PUAN VERMEZ.** Sıralamaya girmez. Saf **bilgilendirici tanıma**
  (kimlik/rozet/başlık): "Sen Biggest Fail'sin". Maçın hikâyesini anlatır,
  kazananı belirlemez. (06_Chaos_Scoring §7)
- **Chaos puanı yönlendirir, motivasyon ÜRETMEZ.** Çekirdek loop puansız da
  komik olmalı. Puan bir scaffold'dur, oyunun kalbi değil. Streamer gittikten
  sonra oyunu ayakta tutan şey puan değil, "arkadaşımla itişmek komik" gerçeği.

---

## 3. Variable Reward (engagement motoru — bilinçli koru)

Combo'ların öngörülemezliği = *variable ratio reinforcement* (en güçlü engagement
şeması). "MEGA CHAOS çıkacağını biliyor muydun? Hayır!" Sürpriz, motorun kalbi.
Oyun bunu doğal taşıyor (ChainID + fizik). **Koru:** sonuçları fazla
deterministik/öngörülebilir yapma; kaza ve sürpriz değerlidir.

---

## 4. Loss Aversion & Onboarding (kaybeden bile gülmeli)

### 4.1 Başarısızlık acıtmaz, güldürür
- **Progress-based skor:** yarım iş puan alır (9/10 = %90). Binary değil.
- **Sabotaj geciktirir, silmez:** penalty progress'i dondurur, sıfırlamaz.
- **Empatik mesaj:** "bir boka yaramadın ama çocuğu şu kadar sevindirdin."
- Amaç: casual oyuncunun rage-quit etmesini önlemek.

### 4.2 Progressive Disclosure (ilk maç anlaşılırlığı)
Sistem derin; yeni oyuncuya her şeyi aynı anda gösterme. İlk maç deneyimi
**kademeli açığa çıkarma** ile tasarlanır. İlkeler (detay map kurulumunda):
- İlk Primary ölesiye basit (tercihen Counter — en okunabilir arketip).
- Trait ilk maç(lar)da yok veya en hafifi (bozuk kontrol üstüne trait = boğulma).
- **Reveal = öğretmen.** İlk maç reveal'i ekstra açıklayıcı: "P3 ayakları
  kesiyormuş, o yüzden zıplayamadın" — nedensellik açıkça gösterilir.
- Derinlik sonraki maçlarda kendiliğinden artar (oyuncu reveal'den öğrenir).
> Onboarding büyük ölçüde **level/map kurulumu** meselesidir → her map için ayrıca
> tasarlanır.

---

## 5. Anti Pile-On (zorbalık değil, kaos)

Schadenfreude (başkasının düşüşünü izlemek) komik; **sürekli herkesin hedefi
olmak** komik değil, zorbalıktır → o oyuncu churn eder.

- Fizik katmanı bunu doğal hafifletir: ChainID yayılımı kurbanı sürekli değiştirir.
- Ama **görev katmanı hedefli olabilir.** SERT KURAL:
  > **Görev dağıtımında (Faz 1) aynı hedef oyuncuya 2'den FAZLA chaos/side
  > görevi atanamaz.** Server dağıtırken hedef-sayacı tutar.
  (03_Player Faz 1 / 02_Scenario görev dağıtımı)

---

## 6. Çekirdek Tasarım Aksiyomları (tek satırlık özetler)

```
Komedi hero'da · Çatışma objective'de · Kaos ikisinin çarpışmasında.
Görevler "yardım" diye sunulur, "sabotaj" diye değil.
İmkânsızlık komedidir (çatışma çözülmez).
Sinematik bir sistemdir (kamera+metin), film değil.
İçerik = data (Scenario/Trait/Objective), yeni kod değil.
Okunabilirlik kutsaldır: görünmeyen/klip üretmeyen sistem eklenmez.
Server her şeyde authoritative; per-frame tick yok.
```

---

## 7. Scope Kilidi (asla ekleme)

Battle pass · Ranked · 50 hero (max 4) · Crafting · Inventory · MMO.

**Altın kural:** bir özellik (1) kaos, (2) oyuncu etkileşimi, (3) komik hikâye
üretmiyorsa — yapılmaz.

---

## 8. Bu Dosyanın Diğer Dokümanlara Bağı

| İlke | Nerede uygulanır |
|------|------------------|
| Competence floor + Best Moments | 06_Chaos_Scoring (best moments, garantili an) |
| Awards puansız (overjustification) | 06_Chaos_Scoring §7 |
| Relatedness / Reveal | 06_Chaos_Scoring §8 |
| Progressive disclosure / onboarding | 02_Scenario + map kurulumu |
| Anti pile-on (2 görev limiti) | 02_Scenario dağıtım + 03_Player Faz 1 |
| Loss aversion (progress, mesaj) | 05_Objective + 06_Chaos_Scoring §9 |
| Variable reward | 06_Chaos_Scoring (ChainID, tier) |

> Kodlamaya başlamadan önce: bu dosyayı oku, sonra 01'den 06'ya in. Her sistem
> kararı yukarıdaki ilkelerden en az birine hizmet etmeli; hiçbirine hizmet
> etmiyorsa veya biriyle çelişiyorsa — yapma.
