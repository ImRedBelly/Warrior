# Справочник gameplay-тегов

Нативные теги объявлены в `Source/Warrior/Public/WarriorGameplayTags.h` и определены в
`Source/Warrior/Private/WarriorGameplayTags.cpp` через `UE_DECLARE_GAMEPLAY_TAG_EXTERN` /
`UE_DEFINE_GAMEPLAY_TAG` в namespace `WarriorGameplayTags`. Обращение из C++:

```cpp
#include "WarriorGameplayTags.h"
...
UWarriorFunctionLibrary::AddGameplayTagToActorIfNone(Actor, WarriorGameplayTags::Player_Status_Blocking);
```

Теги `GameplayCue.*` объявлены не в C++, а в `Config/DefaultGameplayTags.ini`.

## Соглашения об именовании

| Ветка | Смысл |
|---|---|
| `InputTag.*` | привязка `UInputAction` к способности |
| `*.Ability.*` | идентификатор способности (для `TryActivateAbilityByTag`) |
| `*.Event.*` | gameplay-событие (`SendGameplayEventToActor`), триггер способности |
| `*.Status.*` | состояние, вешается как loose gameplay tag или granted-тег эффекта |
| `*.SetByCaller.*` | магнитуда, передаваемая в `FGameplayEffectSpec` |
| `*.Cooldown.*` | granted-тег эффекта кулдауна |
| `*.Weapon*` | ключ в `CharacterCarriedWeaponMap` |
| `GameData.*` | данные вне боя: уровни, слоты сохранений |

## Input

| Тег | Строка | Назначение |
|---|---|---|
| `InputTag_Move` | `InputTag.Move` | движение |
| `InputTag_Look` | `InputTag.Look` | камера |
| `InputTag_EquipAxe` | `InputTag.EquipAxe` | экипировка топора |
| `InputTag_UnequipAxe` | `InputTag.UnequipAxe` | снятие топора |
| `InputTag_LightAttack_Axe` | `InputTag.LightAttack.Axe` | лёгкая атака |
| `InputTag_HeavyAttack_Axe` | `InputTag.HeavyAttack.Axe` | тяжёлая атака |
| `InputTag_Roll` | `InputTag.Roll` | перекат |
| `InputTag_SwitchTarget` | `InputTag.SwitchTarget` | смена цели |
| `InputTag_SpecialWeaponAbility_Light` | `InputTag.SpecialWeaponAbility.Light` | спец-способность (light) |
| `InputTag_SpecialWeaponAbility_Heavy` | `InputTag.SpecialWeaponAbility.Heavy` | спец-способность (heavy) |
| `InputTag_PickUp_Stones` | `InputTag.PickUp.Stones` | подбор камней |
| `InputTag_MustBeHeld` | `InputTag.MustBeHeld` | **родитель**: способность отменяется при отпускании |
| `InputTag_MustBeHeld_Block` | `InputTag.MustBeHeld.Block` | блок |
| `InputTag_Toggleable` | `InputTag.Toggleable` | **родитель**: повторное нажатие отменяет |
| `InputTag_Toggleable_TargetLock` | `InputTag.Toggleable.TargetLock` | таргет-лок |
| `InputTag_Toggleable_Rage` | `InputTag.Toggleable.Rage` | ярость |

## Player

### Ability

| Тег | Строка |
|---|---|
| `Player_Ability_Equip_Axe` | `Player.Ability.Equip.Axe` |
| `Player_Ability_Unequip_Axe` | `Player.Ability.Unequip.Axe` |
| `Player_Ability_Attack_Light_Axe` | `Player.Ability.Attack.Light.Axe` |
| `Player_Ability_Attack_Heavy_Axe` | `Player.Ability.Attack.Heavy.Axe` |
| `Player_Ability_HitPause` | `Player.Ability.HitPause` |
| `Player_Ability_Roll` | `Player.Ability.Roll` |
| `Player_Ability_Block` ⚠️ | `Player.Ability.Block` — объявлен как `Player_Ability_Block`, но **определён** под именем `Player_Ability_Blocking`, см. [KnownIssues.md](KnownIssues.md) |
| `Player_Ability_TargetLock` | `Player.Ability.TargetLock` |
| `Player_Ability_Rage` | `Player.Ability.Rage` |
| `Player_Ability_SpecialWeaponAbility_Light` | `Player.Ability.SpecialWeaponAbility.Light` |
| `Player_Ability_SpecialWeaponAbility_Heavy` | `Player.Ability.SpecialWeaponAbility.Heavy` |
| `Player_Ability_PickUp_Stones` | `Player.Ability.PickUp.Stones` |

### Cooldown / Weapon

| Тег | Строка |
|---|---|
| `Player_Cooldown_SpecialWeaponAbility_Light` | `Player.Cooldown.SpecialWeaponAbility.Light` |
| `Player_Cooldown_SpecialWeaponAbility_Heavy` | `Player.Cooldown.SpecialWeaponAbility.Heavy` |
| `Player_Weapon_Axe` | `Player.Weapon.Axe` |

### Event

| Тег | Строка | Кто посылает |
|---|---|---|
| `Player_Event_Equip_Axe` | `Player.Event.Equip.Axe` | anim notify монтажа экипировки |
| `Player_Event_Unequip_Axe` | `Player.Event.Unequip.Axe` | anim notify |
| `Player_Event_HitPause` | `Player.Event.HitPause` | `UHeroCombatComponent` |
| `Player_Event_SuccessfulBlock` | `Player.Event.SuccessfulBlock` | `UEnemyCombatComponent`, `AWarriorProjectileBase` |
| `Player_Event_SwitchTarget_Left` | `Player.Event.SwitchTarget.Left` | `AWarriorHeroCharacter::Input_SwitchTargetCompleted` |
| `Player_Event_SwitchTarget_Right` | `Player.Event.SwitchTarget.Right` | то же |
| `Player_Event_ActivateRage` | `Player.Event.ActivateRage` | anim notify монтажа ярости |
| `Player_Event_AOE` | `Player.Event.AOE` | anim notify спец-способности |
| `Player_Event_ConsumeStones` | `Player.Event.ConsumeStones` | `AWarriorHeroCharacter::Input_PickUpStonesStarted` |

### Status

| Тег | Строка | Смысл |
|---|---|---|
| `Player_Status_JumpToFinisher` | `Player.Status.JumpToFinisher` | переход к финишеру в комбо |
| `Player_Status_Rolling` | `Player.Status.Rolling` | перекат (блокирует поворот таргет-лока) |
| `Player_Status_Blocking` | `Player.Status.Blocking` | блок (проверяется врагами и снарядами) |
| `Player_Status_TargetLock` | `Player.Status.TargetLock` | активен таргет-лок |
| `Player_Status_Rage_Activating` | `Player.Status.Rage.Activating` | ярость активируется |
| `Player_Status_Rage_Active` | `Player.Status.Rage.Active` | ярость активна |
| `Player_Status_Rage_Full` | `Player.Status.Rage.Full` | шкала полна (ставится в `PostGameplayEffectExecute`) |
| `Player_Status_Rage_None` | `Player.Status.Rage.None` | шкала пуста (там же) |

### SetByCaller

| Тег | Строка | Значение |
|---|---|---|
| `Player_SetByCaller_AttackType_Light` | `Player.SetByCaller.AttackType.Light` | номер комбо лёгкой атаки |
| `Player_SetByCaller_AttackType_Heavy` | `Player.SetByCaller.AttackType.Heavy` | номер комбо тяжёлой атаки |

## Enemy

| Тег | Строка | Смысл |
|---|---|---|
| `Enemy_Ability_Melee` | `Enemy.Ability.Melee` | ближняя атака (может быть выдана несколько раз для вариативности) |
| `Enemy_Ability_Ranged` | `Enemy.Ability.Ranged` | дальняя атака |
| `Enemy_Ability_SummonEnemies` | `Enemy.Ability.SummonEnemies` | призыв врагов |
| `Enemy_Ability_SpawnStone` | `Enemy.Ability.SpawnStone` | спавн камня |
| `Enemy_Weapon` | `Enemy.Weapon` | ключ оружия врага |
| `Enemy_Event_SummonEnemies` | `Enemy.Event.SummonEnemies` | событие для `UAbilityTask_WaitSpawnEnemies` |
| `Enemy_Status_Strafing` | `Enemy.Status.Strafing` | кружит вокруг цели |
| `Enemy_Status_UnderAttack` | `Enemy.Status.UnderAttack` | под атакой |
| `Enemy_Status_Unbloackable` ⚠️ | `Enemy.Status.Unbloackable` | неблокируемая атака (опечатка в имени сохранена) |

## Shared

| Тег | Строка | Смысл |
|---|---|---|
| `Shared_Ability_HitReact` | `Shared.Ability.HitReact` | реакция на удар |
| `Shared_Ability_Death` | `Shared.Ability.Death` | смерть |
| `Shared_Event_MeleeHit` | `Shared.Event.MeleeHit` | попадание в ближнем бою |
| `Shared_Event_HitReact` | `Shared.Event.HitReact` | запуск реакции у цели |
| `Shared_Event_SpawnProjectile` | `Shared.Event.SpawnProjectile` | спавн снаряда из монтажа |
| `Shared_SetByCaller_BaseDamage` | `Shared.SetByCaller.BaseDamage` | базовый урон в спеке эффекта |
| `Shared_Status_Death` | `Shared.Status.Death` | мёртв (ставится в `PostGameplayEffectExecute`, прерывает AI) |
| `Shared_Status_HitReact_Front` | `Shared.Status.HitReact.Front` | удар спереди |
| `Shared_Status_HitReact_Left` | `Shared.Status.HitReact.Left` | удар слева |
| `Shared_Status_HitReact_Right` | `Shared.Status.HitReact.Right` | удар справа |
| `Shared_Status_HitReact_Back` | `Shared.Status.HitReact.Back` | удар сзади |
| `Shared_Status_Invincible` | `Shared.Status.Invincible` | неуязвимость |

## GameData

| Тег | Строка | Использование |
|---|---|---|
| `GameData_Level_MainMenuMap` | `GameData.Level.MainMenuMap` | `UWarriorGameInstance::GetGameLevelByTag` |
| `GameData_Level_SurvivalGameModeMap` | `GameData.Level.SurvivalGameModeMap` | то же |
| `GameData_SaveGame_Slot_1` | `GameData.SaveGame.Slot.1` | имя слота сохранения (строка тега) |

## GameplayCue (из ini)

`Config/DefaultGameplayTags.ini`, ассеты в `Content/GameplayCues/`:

| Тег | Ассет |
|---|---|
| `GameplayCue.FX.MagixShield` | `GC_Hero_MagicShield` |
| `GameplayCue.FX.MagixShield.PerfectBlock` | `GC_Hero_PerfectBlock` |
| `GameplayCue.FX.MagixShield.SuccessfulBlock` | `GC_Hero_SuccessfulBlock` |
| `GameplayCue.FX.Rage.Activating` | `GC_Hero_ActivateRage` |
| `GameplayCue.FX.UnbloackableWarning` | `GC_Enemy_AttackWarning` |
| `GameplayCue.Sounds.MeleeHit.Axe` | `GC_Hero_AxeHit` |
| `GameplayCue.Sounds.MeleeHit.Stick` | `GC_Enemy_HitSound_Stick` |
| `GameplayCue.Sounds.Death.FrostGiant` | `GC_FrostGiant_DeathSound` |
| `GameplayCue.Sounds.Death.Guardian` | `GC_Guardian_DeathSound` |

## Фильтры тегов в редакторе

Многие свойства ограничивают выбор тега метаданными — это защита от опечаток:

```cpp
UPROPERTY(EditDefaultsOnly, meta = (Categories = "InputTag"))          FGameplayTag InputTag;
UPROPERTY(EditDefaultsOnly, meta = (Categories = "Player.Cooldown"))   FGameplayTag AbilityCooldownTag;
UPROPERTY(EditDefaultsOnly, meta = (Categories = "GameData.Level"))    FGameplayTag LevelTag;
```

Настройки системы тегов (`Config/DefaultGameplayTags.ini`): `ImportTagsFromConfig=True`,
`WarnOnInvalidTags=True`, `FastReplication=False`.
