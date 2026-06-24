# PROJECT CHAOS — PLAYER MİMARİSİ

> Doküman seti: `00_Tasarim_Felsefesi` → `01_Ana_Mimari.md` → `02_Scenario.md` → **`03_Player.md`** → `04_Trait.md` → ...
> Ön koşul: 01 (otorite & replikasyon) ve 02 (Scenario ↔ Hero ↔ Trait ilişkisi) okunmuş olmalı.
> Framework: **GAS** (kesin). Trait'ler pasif GameplayEffect/Ability olarak uygulanır.

---

## 1. Üç Katmanlı Player Modeli

Bir oyuncu üç **bağımsız** katmandan oluşur. Katmanlar birbirinden habersizdir;
çarpışınca emergent kaos çıkar (bkz. 02_Scenario §7).

```
Player
 ├─ Visual Identity   = HeroArchetype.VisualPool içinden random mesh (silüet korunur)
 ├─ Hero Ability Set  = HeroArchetype (Q / E + PassiveSlot)
 └─ Match Modifiers   = Trait (pasif slotu doldurur) + Objective'ler + World Event
```

- **Visual ≠ Hero.** Roket Hero'nun 5 mesh'i var, biri random gelir; hepsi roket
  siluetini taşır → rakip "bu uçacak" der. Görünüş gameplay bilgisini gizlemez.
- **Hero = kimlik.** Q ve E hero'ya gömülüdür, her maç aynıdır.
- **Trait = değişken.** PassiveSlot'u doldurur, karma scope ile gelir (04_Trait.md).

---

## 2. Component Haritası & Sahiplik

```
PlayerState  (lobiden round sonuna kadar yaşar — KARARLARIN taşıyıcısı)
 ├─ SelectedHero        ← lobide (Faz 1) belirlenir
 ├─ SelectedMeshIndex   ← lobide
 ├─ SelectedTrait       ← lobide (hero biliniyor → karma scope çözülür)
 └─ ObjectiveComponent  ← atanmış görevler, COND_OwnerOnly (lobide doldurulur)

Pawn / Character  (harita açılınca spawn olur, round boyunca yaşar — UYGULAMA)
 ├─ AbilitySystemComponent (ASC)  : GAS çekirdeği (ASC Pawn'da yaşar)
 ├─ HeroComponent      : Q / E + PassiveSlot tanımı (PlayerState'ten uygulanır)
 ├─ TraitComponent     : PassiveSlot'u doldurur (PlayerState'teki trait'i GAS'la uygular)
 ├─ CosmeticComponent  : VisualPool'dan mesh (PlayerState'teki index'i uygular)
 └─ MovementComponent  : hareket + ragdoll/stagger (komedi burada yaşar)
```

> **Karar lobide (PlayerState), uygulama haritada (Pawn).** Start'a basıldığında
> Pawn henüz yoktur; server kararları PlayerState'te saklar. Harita açılıp Pawn
> spawn olunca bu kararlar Pawn'a uygulanır (§6). Maç içi respawn YOKTUR (round-
> bazlı), bu yüzden ASC Pawn'da yaşar — PlayerState-ASC (Lyra deseni) gereksiz.

| Veri | Nerede | Otorite | Replikasyon |
|------|--------|---------|-------------|
| Atanmış görevler | ObjectiveComponent (PlayerState) | Server | **OwnerOnly** (gizli) |
| Hero kimliği (Q/E) | HeroComponent | Server seçer | Herkese (ability'ler GAS rep) |
| Seçilen trait | TraitComponent | Server seçer | Herkese (etki görünür, gizli değil) |
| Visual mesh | CosmeticComponent | Server seçer | Herkese (cosmetic-only) |
| Ragdoll/stagger | MovementComponent | **Server** | Herkese (simüle) |

Kritik: **görevler gizli, trait/mesh açık.** Görev OwnerOnly replicate olur;
trait ve mesh herkese görünür (zaten görsel/davranışsal etkileri ekranda).

---

## 3. Hero Yapısı (Q / E + PassiveSlot)

Hero'nun iki yeteneği gömülü, bir slotu boştur:

```
HeroComponent (örnek: Rocket Hero)
 ├─ Q  : ana skill — "broken by design"
 │        Roket düz fırlar, yön kontrolü YOK. Önüne ne gelirse çarpar,
 │        çarpınca düşer + kısmi stagger (penalty). Bozukluk skill'in
 │        kendi tasarımında, görevden bağımsız.
 ├─ E  : impact skill
 │        Yüksekten inerse alan etkisi: çevredekiler savrulur, prop'lar dağılır.
 │        Bu zaten bir "başkasını bozma" aracı (görev gerektirmez).
 └─ PassiveSlot : BOŞ DELİK → TraitComponent doldurur (§5)
```

Q ve E **GAS GameplayAbility** olarak hero'ya gömülüdür ve hero atanırken ASC'ye
grant edilir. Bozukluk (yön yok, çarpma cezası) ability'nin/movement'ın içinde
kodlanır — Objective katmanı bundan habersizdir (02_Scenario, Kural 3).

> Önemli ayrım: **Q/E'nin bozukluğu hero'ya gömülüdür (hero-özel kod). PassiveSlot
> ise paylaşılan trait sistemiyle dolar.** Demin sorduğumuz "ortak mı, hero-özel
> mi" sorusunun cevabı: ikisi de — gömülü kısım özel, slot kısmı paylaşılan.

---

## 4. Visual Pool (Cosmetic)

```
CosmeticComponent
 └─ VisualMesh = Random(HeroArchetype.VisualPool)   // server seçer, replicate
```

Kurallar:
- Tüm mesh'ler hero **siluetini korur** (roket = sırt çantası/egzoz/pilot teması).
- Mesh seçimi **yalnız görseldir** — capsule/collision/hitbox DEĞİŞMEZ. Gameplay'i
  etkilemez, sadece çeşitlilik verir.
- İleride modüler parçalara (Head/Body/Backpack/Accessory) bölünebilir; şimdilik
  bütün mesh pool yeterli (scope).

---

## 5. PassiveSlot & Trait — Sözleşme (özet, detay: 04_Trait.md)

Player katmanı trait'in **içeriğini bilmez.** Sadece slotun yaşam döngüsünü ve
sözleşmesini garanti eder. Player, trait'e bir **delik** açar; trait sistemi
o deliği doldurur.

- **Sahiplik:** PassiveSlot Pawn üzerindedir; içeriğini **TraitComponent** yönetir.
- **Doldurma zamanı:** init'te, Hero atandıktan **sonra** — çünkü hero-locked
  trait'ler hangi hero olduğunu bilmek zorundadır (karma scope).
- **Otorite:** trait seçimi **server**'da yapılır, sonuç replicate olur (etki
  görünür, gizli değildir).
- **Uygulama (GAS):** trait, karaktere etkisini **pasif bir GameplayEffect /
  GameplayAbility** olarak uygular. "Kayan Ayaklar" = friction attribute'unu
  düşüren bir GE; "İniş Patlaması Güçlenir" = E'nin impact'ini büyüten bir
  pasif. Hero etkisi + trait etkisi GAS'ta doğal olarak stack'lenir.
- **Sözleşme:** `ApplyTrait(ASC)` / `RemoveTrait(ASC)`. Player bu etkilerin *ne*
  olduğuna karışmaz.
- **Decoupling garantisi:** Player kodu trait olmadan da derlenir/çalışır (slot
  boş kalabilir). Trait sistemi yazılırken Player'a dokunulmaz, sadece delik dolar.
- **Detay:** havuz, karma scope (Global / HeroLocked), seçim mantığı, efekt
  tipleri → **04_Trait.md**.

---

## 6. İki Fazlı Init (KARAR lobide, UYGULAMA haritada)

Akış: **Lobi → Start → (instant karar) → harita açılır → Pawn spawn → uygulama.**
Start'a basıldığında Pawn yoktur; bu yüzden init iki faza ayrılır.

### FAZ 1 — Lobi (Pawn YOK, harita YOK) · SERVER
```
Start'a basılır.
Server her PlayerState için KARAR verir ve saklar:
  1. SelectedHero        ← hangi hero
  2. SelectedMeshIndex   ← hero'nun VisualPool'undan random
  3. SelectedTrait       ← hero biliniyor, karma scope ÇÖZÜLÜR (Global+HeroLocked)
  4. AssignedObjectives  ← scenario havuzlarından (1 Primary + 2 Side + 1 Chaos)
Bu kararlar PlayerState'te durur (Pawn yok, henüz uygulanamaz).
ObjectiveComponent burada doldurulur (OwnerOnly replicate).
```

### FAZ 2 — Harita açıldıktan sonra (Pawn SPAWN olur)
```
1. Pawn possess edilir.
2. ASC.InitAbilityActorInfo(owner=Pawn, avatar=Pawn).   ← GAS hazır (ASC Pawn'da)
   ──────────────────────────────────────────────────── (SERVER uygular)
3. PlayerState.SelectedHero → Q/E ability'lerini ASC'ye GRANT et, HeroComponent doldur.
4. PlayerState.SelectedMeshIndex → CosmeticComponent'e uygula (replicate).
5. PlayerState.SelectedTrait → TraitComponent.ApplyTrait(ASC) ile pasif GE uygula.
6. ObjectiveComponent zaten doludur (Faz 1) → gameplay artık okuyabilir.
```

Bağımlılık zinciri: **(Faz 1) Hero → Trait/Mesh/Objective kararı**, sonra
**(Faz 2) ASC → ability grant → trait GE.**
- Trait *seçimi* Faz 1'de Hero'ya bağımlı (scope için).
- Trait *uygulaması* Faz 2'de ASC'ye bağımlı (GE için).
- Mesh/collision: mesh yalnız görsel, hitbox değişmez.

> Yanlış sıra = en sık bug kaynağı. Trait'i hero kararından önce seçersen
> hero-locked havuz yanlış çözülür; trait GE'sini ASC init'ten önce uygularsan
> sessizce düşer.

---

## 7. Açık Kararlar (Player'a özel)

- [x] ~~ASC nerede yaşar?~~ → **ÇÖZÜLDÜ: Pawn.** Maç içi respawn yok (round-bazlı),
      PlayerState-ASC (Lyra deseni) gereksiz.
- [x] ~~Respawn modeli?~~ → **ÇÖZÜLDÜ: round-bazlı, maç içi respawn yok.** Pawn
      round boyunca yaşar; ragdoll = ölüm değil, düşüp kalkma.
- [ ] Stagger/twerk gibi "komik ceza" state'leri ortak bir state machine'de mi
      toplanacak, yoksa her hero kendi mi tanımlayacak? (Movement katmanı kararı.)
- [ ] Round bitince yeni round: oyuncular lobiye mi döner, yoksa harita içinde mi
      yeni dağıtım + reset olur? (Faz 1'in nerede tekrarlanacağını belirler.)

---

## 8. Bu Dokümanda Kilitlenenler
- 3 katman: Visual / Hero(Q-E-PassiveSlot) / Match Modifiers — bağımsız çalışır.
- Görevler OwnerOnly (gizli); trait + mesh herkese görünür.
- Q/E bozukluğu hero'ya gömülü; PassiveSlot paylaşılan trait sistemiyle dolar.
- Trait GAS üzerinden pasif GE/Ability olarak uygulanır.
- **ASC Pawn'da yaşar; maç içi respawn yok (round-bazlı).**
- **İki fazlı init: KARAR lobide (PlayerState) → UYGULAMA haritada (Pawn).**
- Trait seçimi Faz 1'de Hero'ya, trait uygulaması Faz 2'de ASC'ye bağımlı.
- Visual mesh yalnız görsel; collision/hitbox değişmez.
