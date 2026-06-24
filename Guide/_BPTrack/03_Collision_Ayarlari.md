# 03 — Collision Ayarları (Mesh + Capsule)

> Konu: İki katmanın çarpışma kuralları. Çekirdek ilke: **gameplay sonucu kapsülde, ragdoll sadece görsel.**
> Tasarım: oyuncular SOLID (birbirinin içinden geçmez); aktarım fiziksel çarpmayla.

---

## Katman özeti
| Katman | Rolü | Pawn response |
|--------|------|---------------|
| **Capsule** | Gameplay + solid çarpışma + aktarım algısı (Hit) | **Block** |
| **Mesh (ragdoll)** | Yalnız görsel; kimseyi itmemeli | **Ignore** |

İkisini karıştırma: capsule Block (solid + hit), mesh Ignore (görsel).

---

## Mesh — Skeletal Mesh Component (✅ doğrulandı, ekran görüntüsü)
- Collision Presets: **Custom**
- Collision Enabled: **Query and Physics** (fizik simüle edebilsin)
- Object Type: **PhysicsBody**
- Object Responses:
  - WorldStatic = **Block** (yere batmaz)
  - WorldDynamic = Block
  - **Pawn = Ignore** ← kritik: ragdoll oyuncu kapsüllerini itmez
  - PhysicsBody / Vehicle / Destructible = Block
- Trace: Visibility = Ignore, Camera = Block

> Not: Mesh collision "Query and Physics" olmazsa kısmi ragdoll blend'i (02) çalışmaz.

---

## Capsule — Capsule Component (✅ doğrulandı, ekran görüntüsü)
- Collision Presets: **Pawn**
- Collision Enabled: **Query and Physics**
- Object Type: **Pawn**
- **Simulation Generates Hit Events: ✔** (aktarım için Hit event şart)
- Generate Overlap Events: ✔
- Object Responses:
  - **Pawn = Block** ← oyuncular solid, birbirini iter, içinden geçmez
  - WorldStatic / WorldDynamic / PhysicsBody / Vehicle / Destructible = Block
  - Visibility = Ignore, Camera = Block
- Aktarım `OnComponentHit` (solid çarpma → Hit event) ile yakalanır.

---

## Karar geçmişi (neden böyle)
- Önce Overlap denendi → oyuncular birbirinin içinden geçti, reddedildi.
- Karar: **solid (Block) + Hit ile aktarım.** Skill = ragdoll başlatıcı, ragdoll = dağıtıcı.
