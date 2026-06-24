// Project Chaos - GameState: ChaosSubsystem'in (ChainID beyni) evi.

#include "ChaosGameState.h"

#include "ChaosChainComponent.h"

AChaosGameState::AChaosGameState()
{
	ChainComp = CreateDefaultSubobject<UChaosChainComponent>(TEXT("ChaosChainComponent"));
}
