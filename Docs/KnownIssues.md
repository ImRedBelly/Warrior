# Известные проблемы и техдолг

Список составлен **по чтению кода**, без прогона в редакторе. Часть пунктов — реальные баги, часть —
хрупкие места, которые пока не проявляются из-за того, как код вызывается сегодня. Ничего из
перечисленного не исправлялось при написании документации.

## Возможные падения

### `Player.Ability.Block`: тег объявлен под одним именем, определён под другим

`Source/Warrior/Public/WarriorGameplayTags.h:37` объявляет `Player_Ability_Block`, а
`Source/Warrior/Private/WarriorGameplayTags.cpp:36` определяет `Player_Ability_Blocking` с той же
строкой `"Player.Ability.Block"`.

Сейчас это компилируется и линкуется, потому что `Player_Ability_Block` **ни разу не используется в
C++**. Как только на него сошлётся любой код — получите ошибку линковки. Сам тег в рантайме
зарегистрирован, поэтому Blueprint-и работают нормально.

Исправление: привести имена к одному (`Player_Ability_Block`) — правки в Blueprint не нужны, строка
тега не меняется.

### `GetTeamAttitudeTowards` разыменовывает результат неудачного каста

`Source/Warrior/Private/Controllers/WarriorAIController.cpp`

```cpp
const APawn* PawnToCheck = Cast<const APawn>(&Other);
const IGenericTeamAgentInterface* OtherTeamAgent = Cast<IGenericTeamAgentInterface>(PawnToCheck->GetController());
```

Если `Other` — не пешка (а перцепция вполне может спрашивать про обычные акторы), `PawnToCheck`
равен `nullptr` и происходит разыменование. Нужна проверка `if (!PawnToCheck) return ETeamAttitude::Neutral;`.

### `ApplyGameplayEffectSpecHandleToHitResults`: `CastChecked` на произвольном хите

`Source/Warrior/Private/AbilitySystem/Abilities/WarriorGameplayAbility.cpp`

```cpp
if (APawn* HitPawn = CastChecked<APawn>(Hit.GetActor()))
```

`CastChecked` не возвращает `nullptr`, а падает. Если в массив `FHitResult` попадёт стена или
любой не-пешка (типичный результат сферического/боксового трейса AOE-способности) — assert в
Development-сборке. Само `if` при этом бессмысленно. Нужен `Cast` с проверкой.

### `AWarriorSurvivalGameMode::OnEnemyDestroyed` после последней волны

`OnEnemyDestroyed` → `ShouldKeepSpawnEnemies()` → `GetCurrentWaveSpawnerTableRow()`, а тот падает с
`checkf`, если строки `WaveN` нет. После завершения последней волны `CurrentWaveCount` уже больше
числа строк, поэтому уничтожение любого «отставшего» врага (например, призванного боссом и
зарегистрированного через `RegisterSpawnedEnemies`) приведёт к падению. Нужна проверка
`HasFinishedAllWaves()` в начале `OnEnemyDestroyed`.

### Асинхронная загрузка StartUp-данных врага захватывает `this` без защиты

`Source/Warrior/Private/Characters/WarriorEnemyCharacter.cpp`

```cpp
UAssetManager::GetStreamableManager().RequestAsyncLoad(
    CharacterStartUpData.ToSoftObjectPath(),
    FStreamableDelegate::CreateLambda([this, AbilityApplyLevel]() { ... }));
```

Хендл загрузки нигде не сохраняется, лямбда держит сырой `this`. Если враг будет уничтожен до
завершения загрузки (реальный сценарий в режиме выживания), колбэк обратится к мёртвому объекту.
Лечится `CreateWeakLambda(this, ...)` или хранением `TSharedPtr<FStreamableHandle>` с отменой в
`EndPlay`.

### `USTT_GetEnemyTargetInfo`: обращение к контроллеру без проверки

```cpp
if (auto EnemyPerceptionComponent = EnemyCharacter->GetController<AAIController>()->GetAIPerceptionComponent())
```

И в `EnterState`, и в `ExitState`. Если пешка не заposseс-нута (или уже отпущена контроллером на
момент выхода из состояния) — разыменование `nullptr`.

### `NativeGetWarriorASCFromActor` использует `CastChecked`

```cpp
return CastChecked<UWarriorAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InActor));
```

Любой вызов `NativeDoesActorHaveTag` / `AddGameplayTagToActorIfNone` для актора без ASC — падение.
Сегодня все вызовы идут по враждебным пешкам, у которых ASC есть, но функция небезопасна как
общая утилита (например, при добавлении разрушаемых объектов в трейсы боя).

## Логические проблемы

### `GrantHeroWeaponAbilities` пропускает спец-способности

```cpp
if (InDefaultWeaponAbilities.IsEmpty()) { return; }   // выход ДО обработки спец-способностей
```

Оружие без обычных атак не получит и специальных. Проверка должна быть отдельной для каждого
массива.

### `GetAbilityRemainingCooldownByTag` читает неинициализированные out-параметры

```cpp
if (!TimeRemainingAndDuration.IsEmpty()) { RemainingCooldownTime = ...; TotalCooldownTime = ...; }
return RemainingCooldownTime > 0.f;
```

Если активных эффектов кулдауна нет, обе ссылки не записываются, а возврат читает то, что было в
переменной вызывающей стороны. Из Blueprint это безопасно (локальные переменные обнуляются), из
C++ — неопределённое поведение. Нужно обнулять в начале функции.

### `FStateTreeGetEnemyInfoTask` не восстанавливает скорость

`EnterState` запоминает `DefaultMaxWalkSpeed`, `ExitState` получает instance data и ничего с ней не
делает. Если во время состояния кто-то менял `MaxWalkSpeed`, восстановления не будет — значение
существует только как output-биндинг для дерева.

### AI-цель фиксируется навсегда

`AWarriorAIController::OnEnemyPerceptionUpdated` пишет в блэкборд, только если ключ `TargetActor`
пуст, а `AISenseConfig_Sight` настроен с `LoseSightRadius = 0` и обзором 360°. В сумме — враг
никогда не теряет и никогда не меняет цель. Для одиночной игры это работает, но для сценариев с
несколькими целями логику придётся переписать.

### Таргет-лок не фильтрует цели

`GetAvailableActorsToLock()` добавляет в список **все** попавшие в трейс акторы, кроме самого героя
— без проверки враждебности и без проверки `Shared.Status.Death`. Фильтрация целиком возложена на
настройку `BoxTraceChannel` в Blueprint; мёртвая (но ещё не удалённая) цель может быть захвачена, и
только на следующем тике способность сама себя отменит.

### `UEnemyCombatComponent`: устаревший TODO

Комментарий `//TODO implement block check` стоит прямо над реализованной проверкой блока — можно
удалять.

## Мёртвый код и гигиена

| Место | Что |
|---|---|
| `AI/StateTree/EnemyEvaluatorsUtility.h/.cpp` | пустая заглушка, `#include ...generated.h` закомментирован — файлы можно удалить |
| `WarriorAttributeSet.cpp` | локальная `DebugString` собирается через `FString::Printf` на каждое получение урона и не используется |
| `PawnCombatComponent::ToggleCurrentEquippedWeaponCollision` | пустая ветка `if (bShouldEnable) {}` |
| `Warrior.Build.cs` | `AIModule` указан в зависимостях дважды |
| `WarriorHeroCharacter::SetupPlayerInputComponent` | `checkf(InputConfigDataAsset, TEXT(""))` — пустое сообщение об ошибке |
| `AWarriorHeroCharacter::BeginPlay` | только вызов `Super::` |
| `HeroCombatComponent.h` | `OnHitTargetActor` / `OnWeaponPulledFromTargetActor` объявлены без `override` |
| StateTree-ноды | несколько `FInstanceDataType& InstanceData = ...` получаются и не используются (предупреждения компилятора) |
| `PawnCombatComponent::CharacterCarriedWeaponMap`, `UEnemyUIComponent::EnemyDrawnWidgets` | контейнеры с сырыми указателями на `UObject` без `UPROPERTY` — не участвуют в сборе мусора; сейчас объекты удерживаются миром/вьюпортом, но это хрупко |
| `WarriorGameplayTags.cpp` | часть `UE_DEFINE_GAMEPLAY_TAG` без завершающей `;` (макрос это допускает, но выглядит непоследовательно) |

## Опечатки в именах

Исправлять их — значит трогать имена типов и путей ассетов, поэтому потребуются записи в
`[CoreRedirects]` (в `Config/DefaultEngine.ini` уже накоплено шесть таких записей от предыдущих
переименований).

| Как сейчас | Как правильно |
|---|---|
| `EnemyContidionsUtility.h/.cpp` | `EnemyConditionsUtility` |
| `Enemy_Status_Unbloackable` / `Enemy.Status.Unbloackable` | `Unblockable` |
| «Should Aboard All Logic» (`FStateTreeShouldAboardAllLogicCondition`) | «Should Abort All Logic» |
| `GameplayCue.FX.MagixShield` | `MagicShield` |
| `GA_Enemy_SumonEnemies_Base` | `Summon` |
| `GA_Enemy_MelleAttack_Base` | `Melee` |
| `checkf` в `RegisterSpawnedWeapon`: «A named named %s has already been added» | текст сообщения |
| `checkf` в `AWarriorSurvivalGameMode::BeginPlay`: «valid datat table» | текст сообщения |
| `WarriorWidgetBase.cpp`: «Failed to extrac an EnemyUIComponent» | текст сообщения |
| `WarriorWeaponBase.cpp`: «Forgot to assign an instiagtor» | текст сообщения |

## Заглушки и незавершённое

* **Экран загрузки** — используется `FLoadingScreenAttributes::NewTestLoadingScreenWidget()`, то
  есть тестовый виджет движка вместо собственного.
* **Двойной AI-стек** — Behavior Tree (Guardian, Glacer, FrostGiant) и StateTree (Guardian, Dummy)
  живут параллельно; часть C++ нод (`UBTService_OrientToTargetActor`, `UBTTask_RotateToFaceTarget`)
  дублирует функциональность StateTree-нод (`Orient Rotation To Target Actor`). После завершения
  миграции половину можно удалить.
* **Состояние `PlayerDied`** в `EWarriorSurvivalGameModeState` из C++ никогда не выставляется — им
  управляет только Blueprint.
* **`AWarriorPickUpBase::OnPickUpCollisionSphereBeginOverlap`** — пустая реализация-хук.
* **Сохранения** хранят единственное поле (сложность) и всегда пересоздают объект — прогресс,
  настройки и статистика не сохраняются.
* **Сеть** — репликация не рассматривалась: `FastReplication=False` для тегов, атрибуты без
  `ReplicatedUsing`, ASC в режиме по умолчанию, весь ввод обрабатывается локально.

## Производительность

* `AWarriorHeroCharacter::PossessedBy` грузит `CharacterStartUpData` через `LoadSynchronous()` —
  блокирующая загрузка на старте уровня.
* `UHeroGameplayAbility_TargetLock::GetAvailableActorsToLock` делает боксовый трейс с боксом
  `5000×5000×300` на каждое переключение цели; при большом количестве акторов на канале это
  недёшево.
* `AWarriorSurvivalGameMode` тикает каждый кадр только чтобы накапливать `TimePastSinceStart` —
  напрашиваются таймеры (`FTimerManager`).
* `UCooldownSubsystem::TimeMap` растёт по мере появления акторов; запись удаляется только по
  `OnDestroyed`, стабилизирующей чистки stale weak-указателей нет.
