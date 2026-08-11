# Gameplay Ability System

Вся боевая механика построена на GAS. C++ предоставляет ASC, набор атрибутов, кастомный расчёт
урона, базовые классы способностей и два ability-таска; конкретные способности (`GA_*`) и эффекты
(`GE_*`) — это Blueprint-ассеты.

## Компоненты на персонаже

`AWarriorBaseCharacter` создаёт в конструкторе:

```cpp
WarriorAbilitySystemComponent = CreateDefaultSubobject<UWarriorAbilitySystemComponent>(...);
WarriorAttributeSet          = CreateDefaultSubobject<UWarriorAttributeSet>(...);
MotionWarpingComponent       = CreateDefaultSubobject<UMotionWarpingComponent>(...);
```

`InitAbilityActorInfo(this, this)` вызывается в `PossessedBy` — владелец и аватар совпадают.

## Атрибуты

`Source/Warrior/Public/AbilitySystem/WarriorAttributeSet.h`

| Атрибут | Категория | Назначение |
|---|---|---|
| `CurrentHealth` | Health | текущее здоровье |
| `MaxHealth` | Health | максимум здоровья |
| `CurrentRage` | Rage | текущая ярость (только герой использует) |
| `MaxRage` | Rage | максимум ярости |
| `AttackPower` | Damage | множитель атаки источника |
| `DefensePower` | Damage | делитель защиты цели |
| `DamageTaken` | Damage | **мета-атрибут**: транзитный урон, конвертируется в потерю здоровья |

Все атрибуты инициализируются значением `1.f` в конструкторе. Реальные значения задаются
StartUp-эффектами (`GE_Hero_StartUp`, `GE_Guardian_StartUp`, `GE_FrostGiant_StartUp`, …), которые
читают кривые из curve table (`CT_HeroStats`, `CT_Guardian_Stats`, `CT_Glacer_Stats`,
`CT_FrostGiant_Stats`) — поэтому уровень применения эффекта и определяет фактическую силу
персонажа.

Макрос `ATTRIBUTE_ACCESSORS` генерирует для каждого атрибута getter/setter/initter и
`GetXAttribute()`.

### PostGameplayEffectExecute

Единственное место, где атрибуты «оживают». Логика (`WarriorAttributeSet.cpp`):

1. Кэширует `IPawnUIInterface` цели, падает с `checkf`, если аватар его не реализует.
2. **`CurrentHealth`** — клампится в `[0, MaxHealth]`, бродкастится
   `PawnUIComponent->OnCurrentHealthChanged` в виде доли `Current/Max`.
3. **`CurrentRage`** — клампится, затем:
   * `== MaxRage` → добавляется loose-тег `Player.Status.Rage.Full`;
   * `== 0` → добавляется `Player.Status.Rage.None`;
   * иначе оба тега снимаются.
   Бродкастится `HeroUIComponent->OnCurrentRageChanged`.
4. **`DamageTaken`** — из текущего здоровья вычитается урон, результат клампится и записывается в
   `CurrentHealth`, бродкастится изменение здоровья. При здоровье `0` добавляется
   `Shared.Status.Death` — именно этот тег запускает BP-способности смерти (`GA_Hero_Death`,
   `GA_*_Death`) и прерывает логику AI.

## Расчёт урона: UGEExecCalc_DamageTaken

`Source/Warrior/Private/AbilitySystem/GEExecCalc/GEExecCalc_DamageTaken.cpp`

Захватываемые атрибуты (все **не** snapshot):

| Атрибут | Источник |
|---|---|
| `AttackPower` | Source |
| `DefensePower` | Target |
| `DamageTaken` | Target |

Читаемые SetByCaller-магнитуды:

| Тег | Смысл |
|---|---|
| `Shared.SetByCaller.BaseDamage` | базовый урон (у героя — из данных оружия, у врага — из `FScalableFloat` способности) |
| `Player.SetByCaller.AttackType.Light` | номер комбо лёгкой атаки |
| `Player.SetByCaller.AttackType.Heavy` | номер комбо тяжёлой атаки |

Формула:

```
если комбо лёгкой атаки N != 0:  BaseDamage *= 1 + (N - 1) * 0.05     // +5 % за каждый удар после первого
если комбо тяжёлой атаки M != 0: BaseDamage *= 1 + M * 0.15           // +15 % за каждый удар комбо

FinalDamage = BaseDamage * SourceAttackPower / TargetDefensePower
```

Результат применяется как `EGameplayModOp::Override` к `DamageTaken` цели, и только если
`FinalDamage > 0`. Дальше срабатывает `PostGameplayEffectExecute` и урон превращается в потерю
здоровья.

> Множители комбо не суммируются: способность передаёт либо light-, либо heavy-тег.

Носитель эффекта — `GE_Shared_DealDamage` (Instant, execution = `GEExecCalc_DamageTaken`).

## UWarriorAbilitySystemComponent

`Source/Warrior/Public/AbilitySystem/WarriorAbilitySystemComponent.h`

### Привязка ввода к способностям

Способность связывается с input-тегом не через `InputID`, а через
`AbilitySpec.GetDynamicSpecSourceTags()`. Логика обработки:

```cpp
void OnAbilityInputPressed(const FGameplayTag& InInputTag)
```

* перебирает `GetActivatableAbilities()`;
* пропускает спеки без точного совпадения тега (`HasTagExact`);
* если тег наследует `InputTag.Toggleable` **и** способность уже активна → `CancelAbilityHandle`
  (второе нажатие выключает);
* иначе → `TryActivateAbility`.

```cpp
void OnAbilityInputReleased(const FGameplayTag& InInputTag)
```

* работает только для тегов, наследующих `InputTag.MustBeHeld`;
* отменяет активную способность при отпускании клавиши (так реализован блок).

### Выдача способностей оружия

```cpp
void GrantHeroWeaponAbilities(
    const TArray<FWarriorHeroAbilitySet>& InDefaultWeaponAbilities,
    const TArray<FWarriorHeroSpecialAbilitySet>& InSpecialWeaponAbilities,
    int32 ApplyLevel,
    TArray<FGameplayAbilitySpecHandle>& OutGrantedAbilitySpecHandles);

void RemoveGrantedHeroWeaponAbilities(TArray<FGameplayAbilitySpecHandle>& InSpecHandlesToRemove);
```

Вызывается из BP-способностей экипировки (`GA_Hero_EquipAxe` / `GA_Hero_UnequipAxe`). Полученные
хендлы хранятся на самом оружии (`AWarriorHeroWeapon::AssignGrantedAbilitySpecHandles`), чтобы при
снятии оружия удалить ровно те же спеки. `RemoveGrantedHeroWeaponAbilities` очищает переданный
массив.

> Если `InDefaultWeaponAbilities` пуст, функция выходит **до** обработки
> `InSpecialWeaponAbilities` — оружие без обычных атак не получит и специальных.

### Активация по тегу

```cpp
bool TryActivateAbilityByTag(FGameplayTag AbilityTagToActivate);
```

Ищет все активируемые спеки со совпадающими тегами и запускает **случайную** из них. Это способ
получить вариативность: `Enemy.Ability.Melee` может быть выдан трижды (три разных монтажа), и AI
каждый раз бьёт по-разному. Используется в StateTree-таске `Active Ability By Tag`, в
BT-таске `BTTask_ActivateAbilityByTag` и при подборе камней.

## Базовые классы способностей

### UWarriorGameplayAbility

Политика активации:

```cpp
UENUM(BlueprintType)
enum class EWarriorAbilityActivationPolicy : uint8 { OnTriggered, OnGiven };
```

* `OnTriggered` (по умолчанию) — обычная активация по вводу/событию.
* `OnGiven` — способность активируется сразу в `OnGiveAbility` и **удаляется из ASC** в
  `EndAbility` (`ClearAbility`). Так делаются «пассивки-однократки»: например, спавн оружия при
  старте (`GA_Hero_SpawnAxe`, `GA_Shared_SpawnWeapon`) — сработала и исчезла.

Хелперы: `GetPawnCombatComponentFromActorInfo()`, `GetWarriorAbilitySystemComponentFromActorInfo()`,
`NativeApplyEffectSpecHandleToTarget()`, BP-версия с exec-пинами и
`ApplyGameplayEffectSpecHandleToHitResults()` — последняя проходит по массиву `FHitResult`,
фильтрует враждебные пешки, применяет спек и на успех посылает цели `Shared.Event.HitReact`.

### UWarriorHeroGameplayAbility

| Метод | Что делает |
|---|---|
| `GetHeroCharacterFromActorInfo()` | кэширующий каст аватара |
| `GetHeroControllerFromActorInfo()` | кэширующий каст контроллера |
| `GetHeroCombatComponentFromActorInfo()` | шорткат к `UHeroCombatComponent` |
| `GetHeroUIComponentFromActorInfo()` | шорткат к `UHeroUIComponent` |
| `MakeHeroDamageEffectSpecHandle(...)` | собирает спек урона: контекст + `Shared.SetByCaller.BaseDamage` + тег типа атаки со значением комбо |
| `GetAbilityRemainingCooldownByTag(...)` | запрос остатка кулдауна по тегу `Player.Cooldown.*` для UI |

### UWarriorEnemyGameplayAbility

`GetEnemyCharacterFromActorInfo()`, `GetEnemyCombatComponentFromActorInfo()` и
`MakeEnemyDamageEffectSpecHandle(EffectClass, FScalableFloat)` — урон врага берётся из
`FScalableFloat` по уровню способности, а не из данных оружия.

### UHeroGameplayAbility_PickUpStones

`ActivateAbility` бродкастит `HeroUIComponent->OnStoneInteracted(true)` (показать подсказку
клавиши), `EndAbility` — `false`.

* `CollectStones()` — `BoxTraceMultiForObjects` вниз от героя (`BoxTraceDistance = 50`,
  `BoxTraceSize = 100³`, канал задаётся в `StonesTraceChannel`), собирает `AWarriorStoneBase`
  в `CollectedStones`; если ничего не нашлось — отменяет способность.
* `ConsumeStones()` — вызывает `Consume(ASC, AbilityLevel)` у каждого камня.

### UHeroGameplayAbility_TargetLock

Подробно описана в [Combat.md](Combat.md#таргет-лок).

## Ability-таски

| Класс | Blueprint-нода | Что делает |
|---|---|---|
| `UAbilityTask_ExecuteTaskOnTick` | *Execute Task On Tick* | тикающий таск, бродкастит `OnAbilityTaskTick(DeltaTime)`; используется таргет-локом для обновления камеры |
| `UAbilityTask_WaitSpawnEnemies` | *Wait Gameplay Event And Spawn Enemies* | подписывается на gameplay-event, по получению асинхронно грузит класс врага и спавнит `NumToSpawn` штук в случайных достижимых точках навмеша в радиусе `RandomSpawnRadius` (+150 по Z), затем бродкастит `OnSpawnFinished` / `DidNotSpawn` |

`UAbilityTask_WaitSpawnEnemies` — механика призыва у босса: `GA_Enemy_SumonEnemies_Base` /
`GA_FrostGiant_SummonEnemies` ждут `Enemy.Event.SummonEnemies` (посылается anim-нотифаем из
монтажа), а результат отдают в `AWarriorSurvivalGameMode::RegisterSpawnedEnemies`, чтобы призванные
враги учитывались в счётчике волны.

## StartUp-данные

`Source/Warrior/Public/DataAssets/StartUpData/`

```
UDataAsset_StartUpDataBase
├── UDataAsset_HeroStartUpData
└── UDataAsset_EnemyStartUpData
```

`UDataAsset_StartUpDataBase` содержит:

| Поле | Смысл |
|---|---|
| `ActivateOnGivenAbilities` | способности, которые сами активируются при выдаче (политика `OnGiven`) |
| `ReactiveAbilities` | способности-реакции, срабатывающие по gameplay-событиям (hit react, смерть, блок) |
| `StartupGameplayEffects` | эффекты, применяемые к себе на старте (инициализация атрибутов) |

`GiveToAbilitySystemComponent(ASC, ApplyLevel)` выдаёт обе группы способностей и применяет
эффекты через `ApplyGameplayEffectToSelf(EffectCDO, ApplyLevel, MakeEffectContext())`.

Наследники добавляют своё:

* **герой** — `HeroStartUpAbilitySets`: массив `FWarriorHeroAbilitySet` (input-тег + класс
  способности), тег кладётся в `DynamicSpecSourceTags`, чтобы работала привязка ввода;
* **враг** — `EnemyCombatAbilities`: массив классов `UWarriorEnemyGameplayAbility` без input-тегов
  (их активирует AI по ability-тегу).

Ассеты: `DA_Hero`, `DA_Guardian`, `DA_Glacer`, `DA_FrostGiant`.

## Структуры данных оружия

`Source/Warrior/Public/WarriorTypes/WarriorStructTypes.h`

```cpp
struct FWarriorHeroAbilitySet          { FGameplayTag InputTag; TSubclassOf<UWarriorHeroGameplayAbility> AbilityToGrant; };
struct FWarriorHeroSpecialAbilitySet : FWarriorHeroAbilitySet {
    TSoftObjectPtr<UMaterialInterface> SoftAbilityIconMaterial;   // иконка в HUD
    FGameplayTag AbilityCooldownTag;                              // Player.Cooldown.*
};

struct FWarriorHeroWeaponData {
    TSubclassOf<UWarriorHeroLinkedAnimInstance> WeaponAnimLayerToLink;
    UInputMappingContext* WeaponInputMappingContext;
    TArray<FWarriorHeroAbilitySet>        DefaultWeaponAbilities;
    TArray<FWarriorHeroSpecialAbilitySet> SpecialWeaponAbilities;
    FScalableFloat WeaponBaseDamage;
    TSoftObjectPtr<UTexture2D> SoftWeaponIconTexture;
};
```

`FWarriorHeroWeaponData` лежит прямо на акторе оружия (`AWarriorHeroWeapon::HeroWeaponData`), то
есть один ассет оружия несёт всё: анимационный слой, свой mapping-контекст, набор способностей,
кривую урона (`CT_HeroWeaponStats`) и иконку.

## Сложность и уровень способностей

Уровень, с которым выдаются способности и применяются StartUp-эффекты, зависит от
`AWarriorBaseGameMode::CurrentGameDifficulty` — и **инвертирован** между героем и врагом:

| Сложность | Уровень героя | Уровень врага |
|---|---|---|
| `Easy` | 4 | 1 |
| `Normal` | 3 | 2 |
| `Hard` | 2 | 3 |
| `VeryHard` | 1 | 4 |

Поскольку StartUp-эффекты читают статы из curve table по уровню, одна и та же кривая даёт и
усиление героя на низкой сложности, и усиление врагов на высокой. Если game mode не является
`AWarriorBaseGameMode`, используется уровень `1`.

## Кулдауны

Кулдауны специальных способностей — обычные `UGameplayEffect` с granted-тегами
`Player.Cooldown.SpecialWeaponAbility.Light` / `.Heavy` (ассеты
`GA_Hero_Cooldown_AxeAbility_Light/Heavy`). HUD читает остаток через
`GetAbilityRemainingCooldownByTag`, который делает
`FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags` и берёт первую пару
`(RemainingTime, Duration)`. Тег кулдауна связан с иконкой через
`FWarriorHeroSpecialAbilitySet::AbilityCooldownTag` и делегат
`UHeroUIComponent::OnAbilityCooldownBegin`.

Для AI используется отдельный механизм, не связанный с GAS — см.
[AI.md](AI.md#кулдауны-ucooldownsubsystem).

## GameplayCues

Путь к кью задан в `Config/DefaultGame.ini`:

```ini
[/Script/GameplayAbilities.AbilitySystemGlobals]
bUseDebugTargetFromHud = true
GameplayCueNotifyPaths = "/Game/GameplayCues"
```

Теги кью объявлены не в C++, а в `Config/DefaultGameplayTags.ini`:
`GameplayCue.FX.MagixShield(.PerfectBlock/.SuccessfulBlock)`, `GameplayCue.FX.Rage.Activating`,
`GameplayCue.FX.UnbloackableWarning`, `GameplayCue.Sounds.MeleeHit.Axe/.Stick`,
`GameplayCue.Sounds.Death.FrostGiant/.Guardian`.
