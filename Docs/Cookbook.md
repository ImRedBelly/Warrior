# Рецепты

Пошаговые сценарии типовых доработок. Почти всё делается без правки C++ — при условии, что нужный
тег уже существует.

## Добавить нового врага

1. **StartUp-данные.** Создать `UDataAsset_EnemyStartUpData` (`DA_MyEnemy`): заполнить
   `StartupGameplayEffects` (эффект инициализации статов, читающий свою curve table),
   `ReactiveAbilities` (hit react, смерть), `EnemyCombatAbilities` (боевые способности).
2. **Статы.** Создать curve table (`CT_MyEnemy_Stats`) с кривыми на 4 уровня — по одному на каждую
   сложность — и `GE_MyEnemy_StartUp`, который берёт значения из неё по уровню эффекта.
3. **Персонаж.** Наследовать Blueprint от `BP_EnemyCharacter_Base`: меш, AnimBP, назначить
   `CharacterStartUpData = DA_MyEnemy`, задать `LeftHandCollisionBoxAttachBoneName` /
   `RightHandCollisionBoxAttachBoneName` (боксы переприкрепятся сразу), настроить размеры боксов,
   выбрать класс виджета для `EnemyHealthWidgetComponent`.
4. **Способности.** Наследовать от `GA_Enemy_MelleAttack_Base` / `GA_Enemy_SpawnStone_Base` /
   `GA_Shared_SpawnWeapon`. В теге способности использовать `Enemy.Ability.Melee` (или `Ranged`),
   в монтаже — `ANS_ToggleWeaponCollision` для окна урона.
5. **AI.** Наследовать AI-контроллер от `AIC_Enemy_Base`, назначить StateTree-ассет в
   `StateTreeAIComponent` (или Behavior Tree, если идёте по старому стеку). В дереве использовать
   `Get Enemy Target Info` как источник цели и `Active Ability By Tag` для атак.
6. **Волны.** Добавить класс в нужные строки `DT_EnemyWaveSpawner`.

## Добавить способность герою

### Обычная способность (по нажатию клавиши)

1. Создать `UInputAction` (`IA_MyAbility`) и добавить его в mapping-контекст (`IMC_Default` или
   `IMC_Axe`, если способность привязана к оружию).
2. Добавить нативный тег в `WarriorGameplayTags.h` / `.cpp` — пару `InputTag.MyAbility` и
   `Player.Ability.MyAbility`.
3. В `DA_InputConfig` добавить запись в **`AbilityInputActions`**: тег `InputTag.MyAbility` +
   `IA_MyAbility`.
4. Создать Blueprint-способность от `UWarriorHeroGameplayAbility`; в `Ability Tags` указать
   `Player.Ability.MyAbility`.
5. Выдать способность:
   * постоянно — добавить `FWarriorHeroAbilitySet` (тег + класс) в `HeroStartUpAbilitySets` в
     `DA_Hero`;
   * только с оружием — в `DefaultWeaponAbilities` внутри `HeroWeaponData` у `BP_HeroAxe`.

Никакого C++ помимо объявления тегов не требуется — привязка ввода идёт через
`DynamicSpecSourceTags`.

### Удерживаемая или переключаемая способность

Выдать способность с input-тегом из нужной ветки:

* `InputTag.MustBeHeld.*` — способность отменится при отпускании клавиши;
* `InputTag.Toggleable.*` — повторное нажатие её отменит.

### Способность, срабатывающая от события

В `Ability Triggers` указать event-тег (например, `Shared.Event.HitReact`) и положить способность в
`ReactiveAbilities` StartUp-ассета. Событие пошлёт C++ или anim notify
`AN_SendGameplayEventToOwner`.

### Способность-однократка при старте

В способности выставить `AbilityActivationPolicy = OnGiven` и положить класс в
`ActivateOnGivenAbilities`. Способность активируется при выдаче и удалится из ASC по завершении
(так работает спавн оружия).

## Добавить оружие герою

1. Наследовать Blueprint от `BP_HeroWeaponBase`: меш, размер `WeaponCollisionBox`.
2. Заполнить `HeroWeaponData`:
   * `WeaponAnimLayerToLink` — класс от `UWarriorHeroLinkedAnimInstance` (по образцу
     `AnimLayer_HeroAxe`);
   * `WeaponInputMappingContext` — свой `IMC_*` с атаками этого оружия;
   * `DefaultWeaponAbilities` — обычные атаки (тег ввода + способность);
   * `SpecialWeaponAbilities` — спец-атаки, дополнительно иконка и тег кулдауна
     `Player.Cooldown.*`;
   * `WeaponBaseDamage` — `FScalableFloat` со ссылкой на curve table;
   * `SoftWeaponIconTexture` — иконка для HUD.
3. Добавить тег оружия (`Player.Weapon.MyWeapon`) в `WarriorGameplayTags`.
4. В способности спавна оружия зарегистрировать его:
   `RegisterSpawnedWeapon(тег, оружие, bRegisterAsEquippedWeapon)`. Не забыть выставить
   `Instigator` при спавне — иначе `AWarriorWeaponBase` упадёт с `checkf` при первом оверлапе.
5. В способностях экипировки/снятия вызвать `GrantHeroWeaponAbilities` /
   `RemoveGrantedHeroWeaponAbilities`, хендлы хранить через
   `AssignGrantedAbilitySpecHandles`.

> `GrantHeroWeaponAbilities` выходит досрочно, если `DefaultWeaponAbilities` пуст, — у оружия
> должна быть хотя бы одна обычная атака, иначе спец-способности не выдадутся.

## Добавить волну в режим выживания

1. Открыть `DT_EnemyWaveSpawner`.
2. Добавить строку с именем строго `WaveN`, где `N` — следующий номер по порядку (пропуск номера
   приведёт к `checkf` при переходе к этой волне).
3. Заполнить `EnemyWaveSpawnerDefinitions`: класс врага (soft class), `MinPerSpawnCount`,
   `MaxPerSpawnCount`.
4. Указать `TotalEnemyToSpawnThisWave` — сколько врагов всего должно появиться за волну (спавн
   идёт «партиями», доспавн происходит по мере убийства).
5. Убедиться, что на карте расставлены `ATargetPoint` — из них выбираются точки спавна (радиус
   поиска по навмешу — 400).

Количество волн задавать не нужно: `TotalWavesToSpawn = число строк таблицы`.

## Добавить новый вид камня

1. Создать `UGameplayEffect` (`GE_Item_MyStone`) — что даёт камень при потреблении.
2. Наследовать Blueprint от `BP_StoneBase`, назначить `StoneGameplayEffectClass`.
3. Реализовать BP-событие `On Stone Consumed` (VFX, звук, `Destroy Actor`).
4. Настроить меш и радиус `PickUpCollisionSphere`; убедиться, что объектный тип камня входит в
   `StonesTraceChannel` способности `GA_Hero_PickUp_Stones` — иначе box trace его не найдёт.

## Добавить атрибут

1. В `UWarriorAttributeSet` добавить `FGameplayAttributeData` + `ATTRIBUTE_ACCESSORS`.
2. Инициализировать в конструкторе (`InitMyAttr(1.f)`).
3. При необходимости обработать в `PostGameplayEffectExecute` (клампы, теги, бродкаст в UI).
4. Для отображения — добавить делегат в `UPawnUIComponent` / `UHeroUIComponent` и подписаться из
   виджета.
5. Если атрибут участвует в расчёте урона — добавить `DECLARE_ATTRIBUTE_CAPTUREDEF` /
   `DEFINE_ATTRIBUTE_CAPTUREDEF` в `FWarriorDamageCapture` и `RelevantAttributesToCapture` в
   конструкторе `UGEExecCalc_DamageTaken`.

## Добавить StateTree-ноду на C++

Таск (структурный, без подписок на делегаты) — по образцу
`FStateTreeActiveAbilityByTagTask` в `AI/StateTree/EnemyTasksUtility.h`:

```cpp
USTRUCT()
struct FStateTreeMyTaskInstanceData
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category = Context) TObjectPtr<ACharacter> OwnerPawn;
    UPROPERTY(EditDefaultsOnly)                float MyParam = 0.f;
};

USTRUCT(meta=(DisplayName="My Task", Category="Combat"))
struct FStateTreeMyTask : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()
    using FInstanceDataType = FStateTreeMyTaskInstanceData;
    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
                                          const FStateTreeTransitionResult& Transition) const override;
#if WITH_EDITOR
    virtual FText GetDescription(...) const override;   // подпись ноды в редакторе
#endif
};
```

Условие — то же, но от `FStateTreeConditionCommonBase` с `TestCondition`, плюс
`STATETREE_POD_INSTANCEDATA(...)` для POD-данных.

**Если нужна подписка на делегат** — структурный узел не подойдёт: наследуйтесь от
`UStateTreeTaskBlueprintBase`, как `USTT_GetEnemyTargetInfo`.

## Добавить именованный кулдаун для AI

Использовать `UCooldownSubsystem` напрямую или через готовое условие *Cooldown*:

```cpp
if (UCooldownSubsystem* Subsystem = Actor->GetGameInstance()->GetSubsystem<UCooldownSubsystem>())
{
    if (Subsystem->HasTimePassed(Actor, FName("MyAction"), 10.f)) { /* можно */ }
}
```

Помните: **первый** вызов всегда возвращает `true` (кулдаун стартует «прогретым»).

## Добавить уровень, доступный из меню

1. Добавить тег `GameData.Level.MyMap` в `WarriorGameplayTags`.
2. В `BP_WarriorGameInstance` добавить запись в `GameLevelSets`: тег + soft-ссылка на карту.
3. В виджете меню получить карту через `Get Warrior Game Instance` → `Get Game Level By Tag` и
   открыть её.

## Отладка

* `Debug::Print("текст")` / `Debug::Print("имя", floatValue)` из `WarriorDebugHelper.h` — вывод на
  экран и в лог одной строкой.
* В `GEExecCalc_DamageTaken.cpp` закомментированы вызовы `Debug::Print` для каждого шага расчёта
  урона — раскомментировать при разборе «почему такой урон».
* `bDrawDebugShape` в `UHeroGameplayAbility_PickUpStones` и `bShowPersistentDebugShape` в
  `UHeroGameplayAbility_TargetLock` рисуют трейсы.
* `bUseDebugTargetFromHud = true` в `DefaultGame.ini` — работают консольные команды
  `showdebug abilitysystem`.
* Отладочные StateTree-таски: `STT_Debug`, `STT_DebugEnemyTarget`.
