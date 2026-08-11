# AI

Логика врагов исторически была на **Behavior Tree**, сейчас переезжает на **StateTree**
(последний коммит — `feat: add StateTree to enemy`). В проекте одновременно живут оба стека:
BT-ассеты для Guardian / Glacer / FrostGiant и ST-ассеты для Guardian / Dummy. C++ предоставляет
кастомные ноды и для того, и для другого.

## AWarriorAIController

`Source/Warrior/Private/Controllers/WarriorAIController.cpp`

### Конструктор

```cpp
AWarriorAIController::AWarriorAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>("PathFollowingComponent"))
```

Подмена `PathFollowingComponent` на `UCrowdFollowingComponent` даёт detour crowd avoidance — толпа
врагов не слипается в одну точку.

Создаваемые компоненты:

| Компонент | Настройки |
|---|---|
| `UStateTreeAIComponent` | запускает StateTree-ассет врага |
| `UAISenseConfig_Sight` | `SightRadius 5000`, `LoseSightRadius 0`, `PeripheralVisionAngleDegrees 360`, детект только врагов (`bDetectEnemies = true`, friendlies/neutrals — `false`) |
| `UAIPerceptionComponent` | зрение как доминирующий сенс, подписка на `OnTargetPerceptionUpdated` |

Команда: `SetGenericTeamId(FGenericTeamId(1))`.

> `LoseSightRadius = 0` и обзор 360° означают, что враг, единожды увидев игрока, фактически не
> теряет его из вида — «забывание» цели реализовано не через восприятие, а через условия
> прерывания логики.

### Отношение к другим акторам

```cpp
ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const
{
    // Hostile, если TeamId контроллера другого актора < собственного (0 < 1 => игрок враждебен)
    // иначе Friendly
}
```

Метод не проверяет результат каста `Cast<const APawn>(&Other)` перед разыменованием — см.
[KnownIssues.md](KnownIssues.md).

### BeginPlay: настройка обхода толпы

Из `UPROPERTY(EditDefaultsOnly)`:

| Свойство | По умолчанию | Смысл |
|---|---|---|
| `bEnableDetourCrowdAvoidance` | `true` | вкл/выкл crowd simulation |
| `DetourCrowdAvoidanceQuality` | `4` | 1 → `Low`, 2 → `Medium`, 3 → `Good`, 4 → `High` |
| `CollisionQueryRange` | `600` | `SetCrowdCollisionQueryRange` |

Группа обхода и группа для обхода — обе `1` (враги обходят только друг друга).
Параметры менеджера толпы (`MaxAgents = 50`, `MaxAgentRadius = 300`, набор `AvoidanceConfig`) заданы
в `Config/DefaultEngine.ini`, секция `[/Script/AIModule.CrowdManager]`.

### Обнаружение цели

```cpp
void OnEnemyPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
```

Пишет `Actor` в блэкборд-ключ `TargetActor`, **только если ключ ещё пуст** и стимул успешен. То
есть первая замеченная цель фиксируется до конца жизни врага (или до сброса ключа из дерева).
Ключ используется BT-нодами; StateTree получает цель другим путём — через
`USTT_GetEnemyTargetInfo`.

## StateTree-ноды (C++)

### Таски — `AI/StateTree/EnemyTasksUtility.h`

| Структура | DisplayName | Instance data | Поведение |
|---|---|---|---|
| `FStateTreeGetEnemyInfoTask` | *Get Enemy Info* | `Character` (Context), `DefaultMaxWalkSpeed` (Output) | в `EnterState` запоминает исходный `MaxWalkSpeed` и отдаёт его как output-биндинг; тик всегда `Running` |
| `FStateTreeOrientRotationToTargetActorTask` | *Orient Rotation To Target Actor* | `OwnerPawn` (Context), `TargetActor` (Input), `RotationInterpSpeed` | каждый тик доворачивает пешку к цели через `RInterpTo`; всегда `Running` |
| `FStateTreeActiveAbilityByTagTask` | *Active Ability By Tag* | `OwnerPawn` (Context), `AbilityTagToActivate` | в `EnterState` вызывает `ASC->TryActivateAbilityByTag(...)` и возвращает `Succeeded` |

`FStateTreeActiveAbilityByTagTask` возвращает `Succeeded` сразу, не дожидаясь окончания
способности, — ожидание нужно организовывать в самом дереве (например, условием на тег состояния).

### Условия — `AI/StateTree/EnemyContidionsUtility.h`

| Структура | DisplayName | Что проверяет |
|---|---|---|
| `FStateTreeShouldAboardAllLogicCondition` | *Should Aboard All Logic* | `true`, если цель или сам враг имеют `Shared.Status.Death`, либо `DistanceToTarget ≈ 0` |
| `FStateTreeComputeSuccessChanceCondition` | *Compute Success Chance* | случайное значение в `[SuccessChanceMin, SuccessChanceMax]` используется как вес для `RandomBoolWithWeight` |
| `FStateTreeCooldownCondition` | *Cooldown* | `UCooldownSubsystem::HasTimePassed(владелец, Name, Time)` |
| `FStateTreeDoesActorHaveTagCondition` | *Does Actor Have Tag* | наличие тега у пешки, с флагом `InverseConditionCheck` |

Все условия помечены `STATETREE_POD_INSTANCEDATA` и имеют `GetDescription` под `WITH_EDITOR` —
поэтому в редакторе нода подписана осмысленно (например, `Cooldown: 3.0s`).

### Blueprint-таск: USTT_GetEnemyTargetInfo

`Source/Warrior/Public/AI/StateTree/Tasks/STT_GetEnemyTargetInfo.h`

Единственный таск, унаследованный от `UStateTreeTaskBlueprintBase`, — именно потому, что нужна
подписка на делегат (в структурных нодах StateTree это невозможно; комментарий в коде это прямо
указывает).

| Свойство | Категория | Смысл |
|---|---|---|
| `EnemyCharacter` | Context | владелец |
| `PlayerCharacter` | Output | замеченная цель |
| `PlayerLocation` | Output | позиция цели |
| `DistanceToTarget` | Output | дистанция до цели |

`EnterState` подписывается на `OnTargetPerceptionUpdated` перцепции контроллера и сразу считает
значения; `Tick` пересчитывает `PlayerLocation` / `DistanceToTarget`; `ExitState` снимает подписку
(`RemoveAll(this)`). Эти output-поля биндятся в остальные ноды дерева — это «источник истины» о
цели для StateTree.

## Behavior Tree-ноды (C++)

### UBTService_OrientToTargetActor

Сервис с `Interval = 0` (каждый кадр) и `RotationInterpSpeed = 5` по умолчанию. Читает
блэкборд-ключ `InTargetActorKey` (фильтр — `AActor`) и доворачивает пешку к цели через `RInterpTo`.
`GetStaticDescription` показывает имя ключа в редакторе.

### UBTTask_RotateToFaceTarget

Латентный таск с собственной памятью узла:

```cpp
struct FRotateToFaceTargetTaskMemory {
    TWeakObjectPtr<APawn>  OwningPawn;
    TWeakObjectPtr<AActor> TargetActor;
    bool IsValid() const; void Reset();
};
```

* `AnglePrecision = 10°`, `RotationInterpSpeed = 5`, `bCreateNodeInstance = false`
  (память вместо инстанса — дешевле при массовом спавне);
* `ExecuteTask` — заполняет память, при невалидных данных `Failed`, при уже достигнутой точности
  `Succeeded`, иначе `InProgress`;
* `TickTask` — доворачивает и завершает по достижении `AnglePrecision`;
* `HasReachedAnglePrecision` — угол через `DegAcos(dot(forward, направление на цель))`.

## Кулдауны: UCooldownSubsystem

`Source/Warrior/Private/Subsystems/GameInstance/CooldownSubsystem.cpp`

`UGameInstanceSubsystem` с универсальными именованными таймерами — используется AI там, где
полноценный GAS-кулдаун избыточен (например, «не пытайся призывать чаще, чем раз в 20 секунд»).

```cpp
bool HasTimePassed(const AActor* Actor, FName Key, float Interval);
```

Логика:

| Ситуация | Результат |
|---|---|
| актор невалиден / помечен на уничтожение | `false` |
| `Interval < 0` | `false` |
| нет мира | `false` |
| **первый вызов** для пары (актор, ключ) | `true`, время фиксируется |
| прошло `>= Interval` | `true`, время обновляется |
| прошло меньше | `false` |

Хранилище — `TMap<TWeakObjectPtr<const AActor>, TMap<FName, float>>`. При первом обращении к
актору подписывается на его `OnDestroyed`, чтобы удалить запись (`OnActorDestroyed`).

Обёртки: C++ условие `FStateTreeCooldownCondition` и BP-ассет `STC_Cooldown`.

## Ассеты AI

### Общие

| Ассет | Тип |
|---|---|
| `AIC_Enemy_Base` | базовый AI-контроллер (BP от `AWarriorAIController`) |
| `BB_Enemy_Base` | блэкборд (ключ `TargetActor` и др.) |
| `BP_EnemyCharacter_Base`, `ABP_Enemy_Base` | базовый враг и его AnimBP |
| `BPI_Enemy_Death` | Blueprint-интерфейс смерти |

### Behavior Tree

| Категория | Ассеты |
|---|---|
| Деревья | `BT_Guardian`, `BT_Glacer`, `BT_FrostGiant`, `BT_Dummy` |
| Декораторы | `BTDecorator_CheckCurrentHealthPercent`, `BTDecorator_ComputeSuccessChance`, `BTDecorator_DoesActorHaveTag`, `BTDecorator_ShouldAboardAllLogic`, enum `EWarriorDecoratorOperationType` |
| Сервисы | `BTService_GetDistanceToTarget`, `BTService_UpdateMotionWarpAttackTarget` |
| Таски | `BTTask_EnemyBase`, `BTTask_ActivateAbilityByTag`, `BTTask_ToggleStrafingState` |

### StateTree

| Категория | Ассеты |
|---|---|
| Деревья | `ST_Guardian`, `ST_Dummy` |
| Условия | `STC_Cooldown` |
| Таски | `STT_ToggleStrafingState`, `STT_Debug`, `STT_DebugEnemyTarget` |

### EQS

`EQS_FindStrafingLocation` (позиция для кружения вокруг игрока),
`EQS_FindShootProjectileLocation` (позиция для стрельбы у Glacer),
`EQSContext_TargetActor`, `EQS_TestPawn`.

## Состояния врага (теги)

| Тег | Смысл |
|---|---|
| `Enemy.Status.Strafing` | кружит вокруг цели (переключается таском `ToggleStrafingState`, влияет на blend space `BS_Guardian_Strafing`) |
| `Enemy.Status.UnderAttack` | получает урон (эффект `GE_Enemy_UnderAttack`) |
| `Enemy.Status.Unbloackable` | текущая атака пробивает блок игрока |
| `Shared.Status.Death` | смерть — прерывает всю логику через *Should Aboard All Logic* |

Ability-теги врага: `Enemy.Ability.Melee`, `Enemy.Ability.Ranged`, `Enemy.Ability.SummonEnemies`,
`Enemy.Ability.SpawnStone` — именно они передаются в *Active Ability By Tag*.

## Типы врагов

| Враг | Особенности |
|---|---|
| **Guardian** (`BP_Gruntling_Guardian`) | ближний бой, две атаки, стрейфинг, спавн камня |
| **Glacer** (`BP_Gruntling_Glacer`) | дальний бой снарядами (`BP_Projectile_Glacer`), EQS для позиции стрельбы |
| **FrostGiant** (`BP_FrostGiant`) | босс: три мели-атаки, комбо, призыв врагов, полоска босса (`GA_FrostGiant_DrawBossBar`) |
| **DummyGuardian** (`BP_DummyGuardian`) | манекен для тестов |
