# Warrior

Экшен-слэшер от третьего лица на Unreal Engine 5.6: герой с топором, ближний и дальний бой,
таргет-лок, блок, парирование, ярость (Rage), волновой режим выживания и AI-противники на
StateTree / Behavior Tree.

Проект построен по схеме **«каркас на C++ — наполнение на Blueprint»**: весь фреймворк
(персонажи, компоненты, GAS-обвязка, AI-ноды, игровые режимы) реализован в C++, а конкретные
способности, эффекты, монтажи и настройки — в ассетах.

## Требования

| | |
|---|---|
| Движок | Unreal Engine **5.6** |
| Целевая платформа | Windows (DX12, SM6) |
| Компилятор | Visual Studio 2022 (C++20, `BuildSettingsVersion.V5`) |
| Плагины | `GameplayAbilities`, `MotionWarping`, `GameplayStateTree`, `ModelingToolsEditorMode` (только редактор) |

## Сборка и запуск

Сгенерировать проектные файлы (ПКМ по `Warrior.uproject` → *Generate Visual Studio project files*), затем:

```bash
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" WarriorEditor Win64 Development -Project="D:\UnrealEngine\Projects\Warrior\Warrior.uproject" -WaitMutex
```

Запуск редактора:

```bash
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" "D:\UnrealEngine\Projects\Warrior\Warrior.uproject"
```

Стартовая карта по умолчанию — `Content/Maps/CombatTestMap`, game mode по умолчанию —
`BP_BaseGameMode`, game instance — `BP_WarriorGameInstance` (см. `Config/DefaultEngine.ini`).

## Карты

| Карта | Назначение |
|---|---|
| `CombatTestMap` | основная песочница для боя (карта по умолчанию) |
| `FeatureDevMap` | карта для разработки отдельных механик |
| `GameModeTestMap` | тесты игровых режимов |
| `MainMenuMap` | главное меню |
| `SurvivalGameModeMap` | режим выживания с волнами |

## Структура репозитория

```
Config/                     ini-конфиги (движок, ввод, gameplay-теги)
Content/
  Assets/                   меши, текстуры, звуки, Niagara, анимации
  EnemyCharacter/           враги: BP, AIC, BT/ST, способности, монтажи
  GameModes/                BP_BaseGameMode, BP_SurvivalGameMode, DT_EnemyWaveSpawner
  GameplayCues/             GameplayCue-нотифаи (звук/VFX)
  Generic/                  BP_WarriorGameInstance
  Items/Stones/             подбираемые камни и их эффекты
  Maps/                     уровни
  PlayerCharacter/          герой: BP, ввод, способности, эффекты, оружие, анимации
  Shared/                   общие способности, эффекты, нотифаи, базовый снаряд
  Widgets/                  UMG-виджеты (HUD, меню, полоски здоровья)
Source/Warrior/
  Public/ | Private/        единственный runtime-модуль проекта
```

## Документация

| Документ | О чём |
|---|---|
| [Архитектура](Docs/Architecture.md) | модуль, иерархии классов, порядок инициализации, потоки данных |
| [Ability System (GAS)](Docs/AbilitySystem.md) | атрибуты, расчёт урона, базовые способности, StartUp-данные, сложность |
| [Боевая система](Docs/Combat.md) | оружие, хитбоксы, блок, hit react, снаряды, камни, таргет-лок |
| [Ввод](Docs/Input.md) | Enhanced Input, input-теги, конфиг-ассет, mapping-контексты |
| [AI](Docs/AI.md) | контроллер, восприятие, команды, StateTree- и BT-ноды, кулдауны |
| [Игровой цикл](Docs/GameFlow.md) | game modes, волны выживания, сложность, сохранения, уровни |
| [UI](Docs/UI.md) | UI-компоненты, виджеты, делегаты |
| [Справочник gameplay-тегов](Docs/GameplayTags.md) | все нативные теги и их назначение |
| [Рецепты](Docs/Cookbook.md) | как добавить врага, способность, оружие, волну, камень |
| [Известные проблемы](Docs/KnownIssues.md) | найденные баги, опечатки, заглушки, техдолг |
