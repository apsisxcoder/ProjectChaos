# PROJECT CHAOS — TRAIT SİSTEMİ

> Doküman seti: `00` → `01` → `02_Scenario` → `03_Player` → **`04_Trait`** → ...
> Ön koşul: 03_Player.md (PassiveSlot sözleşmesi + iki fazlı init) okunmuş olmalı.
> Trait, Player'ın açtığı **PassiveSlot deliğini** doldurur. Player trait'in
> içeriğini bilmez; bu doküman o içeriği tanımlar.

---

## 1. Trait Nedir

Trait, karakterin **pasif slotunu** dolduran değişken bir modifikasyondur.
Hero kimliğinin (Q/E) parçası değildir — ayrı bir katmandır. Maç başında
(Faz 1, lobide) seçilir, harita açılınca (Faz 2) GAS ile uygulanır.

```
Hero (sabit kimlik)        Trait (değişken katman)
 ├─ Q                       PassiveSlot'a takılır
 ├─ E                       random seçilir (karma scope)
 └─ PassiveSlot  ◄──────────  bir DA_Trait
```

Komedi formülündeki yeri:
> **Komedi Hero'da · Çatışma Objective'de · Trait kaosu ÇEŞİTLENDİRİR.**
> Roket Hero + Kayan Ayaklar ile Roket Hero + Kauçuk Vücut tamamen farklı iki
> maç üretir — tek satır özel kod yazmadan.

---

## 2. Karma Scope (asıl tasarım kararı)

Her trait'in bir **scope etiketi** vardır: kime gelebileceğini söyler.

```
Scope = Global       → herkese gelebilir (her hero'nun havuzunda)
Scope = HeroLocked   → sadece AllowedHeroes[] içindeki hero'lara gelebilir
```

- **Global** trait'ler hero kimliğinden bağımsız genel kaos verir
  (Kayan Ayaklar, Kauçuk Vücut, Ters Kontrol, Ağır Kafa...).
- **HeroLocked** trait'ler hero kimliğini güçlendirir
  (İniş Patlaması Güçlenir → sadece Rocket; Manyetik Menzil → sadece Magnet...).

Tek sistem, iki etiket. Yeni trait eklemek = data + scope etiketi, **kod değil.**

---

## 3. Seçim Mantığı (Faz 1, lobide, SERVER)

Trait, oyuncunun hero'su belirlendikten **sonra** seçilir — çünkü hero-locked
havuz hangi hero olduğunu bilmek zorundadır (03_Player §6, Faz 1).

```
oyuncunun trait havuzu =
    (Scope == Global olan tüm trait'ler)
  + (Scope == HeroLocked && AllowedHeroes.Contains(SelectedHero) olanlar)

SelectedTrait = Random(havuz)
PlayerState.SelectedTrait = SelectedTrait   // Faz 2'de uygulanacak
```

Örnek — Rocket Hero için havuz:
```
[ Kayan Ayaklar, Kauçuk Vücut, Ters Kontrol, Ağır Kafa, ... ]   (Global)
+ [ İniş Patlaması Güçlenir, Egzoz İtişi, ... ]                  (Rocket-locked)
```
Magnet Hero aynı Global'leri görür ama Rocket-locked'ları görmez; kendi
locked'larını görür. Birleşim temiz: ortak havuz + hero'ya özel havuz.

> Not: İleride scope seçim ağırlığı eklenebilir (örn. %70 Global, %30 HeroLocked)
> ama şimdilik düz random yeterli (scope).

---

## 4. Uygulama (Faz 2, harita açılınca, GAS)

Trait etkisini **pasif bir GameplayEffect veya GameplayAbility** olarak uygular.
Hero etkisi + trait etkisi GAS'ta doğal olarak stack'lenir.

```
TraitComponent.ApplyTrait(ASC):
    SelectedTrait.GrantedEffects → ASC'ye uygula (pasif GE / ability)
TraitComponent.RemoveTrait(ASC):
    uygulanan GE handle'larını geri al
```

Efekt tipleri (örnekler):

| Trait | Tip | GAS uygulaması |
|-------|-----|----------------|
| Kayan Ayaklar | Movement mod | Friction attribute'unu düşüren pasif GE |
| Ağır Kafa | Movement mod | MaxWalkSpeed / denge attribute'u GE |
| Kauçuk Vücut | Impact mod | Çarpışmada extra bounce (collision response + GE) |
| Ters Kontrol | Control mod | Input'u ters çeviren GameplayAbility/tag |
| İniş Patlaması Güçlenir | Hero-skill mod | Rocket E'sinin impact attribute'unu büyüten GE |
| Egzoz İtişi | Hero-skill mod | Rocket Q sonrası ek itiş (GE/cue) |

Kural: trait, etkisini **attribute / GameplayTag** üzerinden yapar. Hero ability'leri
bu attribute/tag'leri okur. Böylece "İniş Patlaması Güçlenir" trait'i, Rocket E
ability'sinin koduna dokunmadan onu büyütür (decoupling).

---

## 5. DA_Trait — Data Yapısı (taslak)

```
DA_Trait
 ├─ TraitName       : FName              ("SlipperyFeet")
 ├─ DisplayName     : FText              ("Kayan Ayaklar")
 ├─ Scope           : Global | HeroLocked
 ├─ AllowedHeroes[] : TArray<HeroId>     (HeroLocked ise dolu, Global ise boş)
 ├─ GrantedEffects[]: TArray<TSubclassOf<GameplayEffect>>   (pasif etkiler)
 ├─ GrantedAbilities[] : TArray<TSubclassOf<GameplayAbility>> (örn. Ters Kontrol)
 ├─ VisualCue       : GameplayCue / VFX   (trait görünür olsun — klip değeri)
 └─ ClipValue       : Low | Mid | High    (ne kadar komik/görünür?)
```

`ClipValue` bir filtredir: ekranda görünmeyen, sadece sayısal bir trait
(örn. "+%5 hız") düşük değerlidir ve havuza alınmaz. Trait **görünür ve komik**
olmalı (Kayan Ayaklar ✓, görünmez stat ✗) — 07_World_Events.md'deki klip
filtresinin aynısı.

---

## 6. Trait Havuzu — İlk Set (taslak)

### Global (herkese)
- **Kayan Ayaklar** — düşük friction, durmak zor
- **Ağır Kafa** — denge bozuk, ani yön değişiminde tökezleme
- **Kauçuk Vücut** — çarpışmada fazladan seker
- **Ters Kontrol** — input ters (süreli/sürekli)
- **Minik Zıplama** — zıplama gücü çok düşük
- **Manyetik Bot** — yakındaki metal/oyuncuya hafif çekim
- **Rastgele Hıçkırık** — periyodik ani küçük zıplama/sarsıntı

### HeroLocked (örnek)
- **İniş Patlaması Güçlenir** — `[Rocket]` E impact'i büyür
- **Egzoz İtişi** — `[Rocket]` Q sonrası geri tepme
- **Manyetik Menzil** — `[Magnet]` çekim/itiş menzili artar
- **Dev Adım** — `[Giant]` adımlar yeri sarsar (yakındakiler sendeler)

> Sayılar (friction değeri, menzil, süre) playtest ile kalibre edilir; şu an
> tipler ve scope'lar kilitleniyor, değerler değil.

---

## 7. Player ile Sözleşme (tekrar — sınır net)

Trait sistemi Player'a **dokunmaz**, sadece deliği doldurur (03_Player §5):
- Player `PassiveSlot` + `ApplyTrait(ASC)` / `RemoveTrait(ASC)` sağlar.
- Trait `DA_Trait` + `GrantedEffects` ile bu sözleşmeyi karşılar.
- Player trait olmadan da derlenir/çalışır (slot boş kalabilir).
- Trait yazılırken Player kodu değişmez.

---

## 8. Bu Dokümanda Kilitlenenler
- Karma scope: her trait `Global` veya `HeroLocked` (+ `AllowedHeroes`).
- Seçim Faz 1'de (lobide), Hero belirlendikten sonra; havuz = Global + hero-locked.
- Uygulama Faz 2'de (haritada), GAS pasif GE/Ability ile.
- Trait etkisi attribute/GameplayTag üzerinden → hero ability'lerine dokunmaz.
- Trait görünür ve komik olmalı (`ClipValue` filtresi).
- Trait, hero kimliğinin parçası değil; ayrı katman.

## 9. Açık Kararlar
- [ ] Scope seçim ağırlığı (düz random mı, Global/HeroLocked oranı mı?).
- [ ] Trait süreli mi (round boyunca sabit) yoksa maç içi değişebilir mi?
      (Şimdilik varsayım: round boyu sabit, Faz 1'de set edilir.)
- [ ] Trait değerlerinin (friction, menzil...) kalibrasyonu → playtest.
