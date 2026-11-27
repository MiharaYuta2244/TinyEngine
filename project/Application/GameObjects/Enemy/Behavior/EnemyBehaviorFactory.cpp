#include "EnemyBehaviorFactory.h"
#include "AssaultBehavior.h"
#include "BomberBehavior.h"
#include "NormalBehavior.h"
#include "ShotgunBehavior.h"

std::unique_ptr<IEnemyBehavior> EnemyBehaviorFactory::Create(EnemyType type) {
	switch (type) {
	case EnemyType::Shotgun:
		return std::make_unique<ShotgunBehavior>();
	case EnemyType::Bomber:
		return std::make_unique<BomberBehavior>();
	case EnemyType::Assault:
		return std::make_unique<AssaultBehavior>();
	default:
		return std::make_unique<NormalBehavior>();
	}
}