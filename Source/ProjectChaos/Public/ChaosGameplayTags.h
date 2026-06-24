// Project Chaos - Native GameplayTag'ler (tag event omurgası, 06 §10).

#pragma once

#include "NativeGameplayTags.h"

// Chaos olay kanalları. Component'ler bunlara yayın yapar; ChaosSubsystem dinler.
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Skill_Used);       // skill kullanıldı → yeni ChainID
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Penalty_Recovered); // oyuncu ayıldı → zincirden çık
