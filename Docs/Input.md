# Ввод

Проект использует **Enhanced Input**. Связка «клавиша → способность» построена на gameplay-тегах:
`UInputAction` сопоставляется с тегом в data-ассете, а тег кладётся в
`FGameplayAbilitySpec::DynamicSpecSourceTags` при выдаче способности. Благодаря этому C++ не знает
ни о конкретных клавишах, ни о конкретных способностях.

## Конфиг-ассет ввода

`Source/Warrior/Public/DataAssets/Input/DataAsset_InputConfig.h`

```cpp
struct FWarriorInputActionConfig {
    FGameplayTag  InputTag;      // meta = (Categories = "InputTag")
    UInputAction* InputAction;
    bool IsValid() const;        // тег валиден И экшен не null
};

class UDataAsset_InputConfig : public UDataAsset {
    UInputMappingContext* DefaultMappingContext;
    TArray<FWarriorInputActionConfig> NativeInputActions;   // обрабатываются C++-функциями
    TArray<FWarriorInputActionConfig> AbilityInputActions;  // транслируются в ASC
    UInputAction* FindNativeInputActionByTag(const FGameplayTag&) const;
};
```

Ассет проекта — `Content/PlayerCharacter/DA_InputConfig`, назначается в `BP_HeroCharacter`
(свойство `InputConfigDataAsset`).

Различие двух массивов принципиально:

* **`NativeInputActions`** — движение, камера, переключение цели, подбор камней. Каждый экшен
  привязывается вручную в C++ к конкретному методу и конкретному `ETriggerEvent`.
* **`AbilityInputActions`** — всё, что должно активировать способность. Привязываются пачкой, без
  упоминания конкретных способностей.

## UWarriorInputComponent

`Source/Warrior/Public/Components/Input/WarriorInputComponent.h` — наследник
`UEnhancedInputComponent` с двумя шаблонными методами (обе реализации в заголовке):

```cpp
template <class UserObject, typename CallbackFunc>
void BindNativeInputAction(const UDataAsset_InputConfig* Config, const FGameplayTag& InputTag,
                           ETriggerEvent TriggerEvent, UserObject* ContextObject, CallbackFunc Func);

template <class UserObject, typename CallbackFunc>
void BindAbilityInputAction(const UDataAsset_InputConfig* Config, UserObject* ContextObject,
                            CallbackFunc InputPressedFunc, CallbackFunc InputReleasedFunc);
```

`BindAbilityInputAction` проходит по всем `AbilityInputActions` и привязывает каждый экшен дважды:

| `ETriggerEvent` | Колбэк | Аргумент |
|---|---|---|
| `Started` | `InputPressedFunc` | `InputTag` экшена |
| `Completed` | `InputReleasedFunc` | `InputTag` экшена |

Тег передаётся как payload-аргумент `BindAction`, поэтому один метод обслуживает все способности.

Компонент подключён глобально в `Config/DefaultInput.ini`:

```ini
DefaultInputComponentClass=/Script/Warrior.WarriorInputComponent
DefaultPlayerInputClass=/Script/EnhancedInput.EnhancedPlayerInput
```

## Привязки героя

`AWarriorHeroCharacter::SetupPlayerInputComponent` — `Source/Warrior/Private/Characters/WarriorHeroCharacter.cpp`

```cpp
Subsystem->AddMappingContext(InputConfigDataAsset->DefaultMappingContext, 0);
```

| Input-тег | Trigger | Метод | Действие |
|---|---|---|---|
| `InputTag.Move` | `Triggered` | `Input_Move` | движение относительно yaw контроллера |
| `InputTag.Look` | `Triggered` | `Input_Look` | поворот камеры |
| `InputTag.SwitchTarget` | `Triggered` | `Input_SwitchTargetTriggered` | запоминает вектор направления в `SwitchDirection` |
| `InputTag.SwitchTarget` | `Completed` | `Input_SwitchTargetCompleted` | посылает `Player.Event.SwitchTarget.Left/Right` по знаку `X` |
| `InputTag.PickUp.Stones` | `Started` | `Input_PickUpStonesStarted` | посылает `Player.Event.ConsumeStones` |
| *все ability-экшены* | `Started` / `Completed` | `Input_AbilityInputPressed/Released` | `ASC->OnAbilityInputPressed/Released(тег)` |

`Input_Move` строит направление от `FRotator(0, ControlRotation.Yaw, 0)` и раздельно добавляет
forward/right по `MovementVector.Y` / `.X`.

## Соглашения по input-тегам

Иерархия тегов несёт поведение — см. [AbilitySystem.md](AbilitySystem.md#привязка-ввода-к-способностям):

| Родительский тег | Поведение |
|---|---|
| `InputTag.MustBeHeld` | способность отменяется при отпускании клавиши (`InputTag.MustBeHeld.Block`) |
| `InputTag.Toggleable` | повторное нажатие отменяет активную способность (`InputTag.Toggleable.TargetLock`, `InputTag.Toggleable.Rage`) |
| остальные | обычная активация по нажатию |

Чтобы способность стала «удерживаемой» или «переключаемой», достаточно выдать её с тегом из нужной
ветки — код менять не нужно.

## Полный список input-тегов

| Тег | Назначение |
|---|---|
| `InputTag.Move` | движение |
| `InputTag.Look` | камера |
| `InputTag.EquipAxe` / `InputTag.UnequipAxe` | экипировка / снятие топора |
| `InputTag.LightAttack.Axe` / `InputTag.HeavyAttack.Axe` | лёгкая / тяжёлая атака |
| `InputTag.Roll` | перекат |
| `InputTag.SwitchTarget` | переключение цели при таргет-локе |
| `InputTag.SpecialWeaponAbility.Light` / `.Heavy` | специальные способности оружия |
| `InputTag.PickUp.Stones` | подбор камней |
| `InputTag.MustBeHeld` → `.Block` | блок (удержание) |
| `InputTag.Toggleable` → `.TargetLock`, `.Rage` | таргет-лок, ярость (переключение) |

## Mapping-контексты и приоритеты

| Контекст | Приоритет | Когда добавляется |
|---|---|---|
| `IMC_Default` | 0 | `SetupPlayerInputComponent` |
| `IMC_Axe` | задаётся в BP | при экипировке оружия (`FWarriorHeroWeaponData::WeaponInputMappingContext`) |
| `IMC_TargetLock` | 3 | `UHeroGameplayAbility_TargetLock::InitTargetLockMappingContext`, снимается в `EndAbility` |

Такая схема даёт контекстно-зависимое управление: атаки топором появляются только с топором в
руках, а переключение цели — только в режиме таргет-лока.

## Ассеты Input Action

`Content/PlayerCharacter/Input/Actions/`:

`IA_Move`, `IA_Look`, `IA_Block`, `IA_EquipAxe`, `IA_UnequipAxe`, `IA_LightAttack_Axe`,
`IA_HeavyAttack_Axe`, `IA_Roll`, `IA_Rage`, `IA_TargetLock`, `IA_SwitchTarget`,
`IA_SpecialWeaponAbility_Light`, `IA_SpecialWeaponAbility_Heavy`,
`IA_SpecialWeaponAbility_Trigger`, `IA_PickUpStones`, `IA_PauseMenu`.

> Конкретные клавиши/кнопки задаются в mapping-контекстах (`IMC_*`) и в C++ нигде не фигурируют.

## Переключение режима ввода для UI

```cpp
UWarriorFunctionLibrary::ToggleInputMode(WorldContextObject, EWarriorInputMode);
```

* `GameOnly` → `FInputModeGameOnly`, курсор скрыт;
* `UIOnly` → `FInputModeUIOnly`, курсор показан.

Работает с `World->GetFirstPlayerController()`; используется виджетами меню и паузы
(`WBP_MainMenu`, `WBP_PauseScreen`, `WBP_OptionsMenu`).
