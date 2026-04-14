# RD 索敌相机系统说明

本文档说明当前 `URDTargetingCameraComponent` 的职责、运行流程和可配置参数。

源码位置：

```text
Source/AITestProject/Character/New/RDTargetingCameraComponent.h
Source/AITestProject/Character/New/RDTargetingCameraComponent.cpp
Source/AITestProject/Character/New/RDBasecharacter.h
Source/AITestProject/Character/New/RDBasecharacter.cpp
```

## 一、系统拆解

### `ARDBasecharacter`

角色类现在只保留相机实体组件和输入转发：

- `CameraBoom`
- `FollowCamera`
- `TargetingCameraComponent`

角色在构造阶段创建 `TargetingCameraComponent`，并把 `CameraBoom`、`FollowCamera` 传进去。

角色输入处理：

- `Look`：转发到 `TargetingCameraComponent->HandleLookInput`
- `ToggleLockOn`：转发到 `TargetingCameraComponent->ToggleLockOn`
- `SwitchTarget`：转发到 `TargetingCameraComponent->SwitchTarget`

角色 Tick 中调用：

```text
TargetingCameraComponent->TickCamera(DeltaSeconds)
UpdateCharacterFacing(DeltaSeconds)
```

`UpdateCharacterFacing` 仍留在角色里，因为它需要和滑铲、墙跑的角色朝向优先级协作。

### `URDTargetingCameraComponent`

索敌相机组件负责：

- 管理相机状态：探索、战斗自由、锁定、解锁恢复
- 搜索可锁定目标
- 进入和退出锁定
- 切换锁定目标
- 根据状态计算目标 ArmLength、SocketOffset、FOV、ControlRotation
- 把计算结果插值应用到 `SpringArm`、`CameraComponent`、`PlayerController`

当前锁定状态下鼠标 Look 输入会被吞掉：

```text
CameraState == LockOn 时，HandleLookInput 直接 return
```

也就是说进入索敌状态后，玩家无法继续用鼠标旋转相机朝向。

## 二、状态说明

### `ECombatCameraState`

| 状态 | 含义 |
| --- | --- |
| `Explore` | 探索状态。没有附近战斗目标时使用。 |
| `CombatFree` | 战斗自由状态。附近存在可战斗目标，但没有锁定目标时使用。 |
| `LockOn` | 锁定状态。存在 `CurrentLockTarget` 时使用。 |
| `Recover` | 解锁恢复状态。退出锁定后短暂保持战斗镜头，随后回到 `CombatFree` 或 `Explore`。 |

状态更新逻辑：

```text
CurrentLockTarget 有效 -> LockOn
CurrentLockTarget 无效 -> ExitLockOn
UnlockRecoveryRemaining > 0 -> Recover
HasNearbyCombatTarget -> CombatFree
否则 -> Explore
```

### `ELockOnZone`

| 区域 | 含义 |
| --- | --- |
| `Center` | 目标接近相机中心。 |
| `Buffer` | 目标略微偏离中心。 |
| `Edge` | 目标接近画面边缘。 |
| `Offscreen` | 目标偏离角度超过 `EdgeAngle`。 |

当前区域由目标 yaw 和当前控制旋转 yaw 的差值计算：

```text
DeltaYaw <= CenterAngle -> Center
DeltaYaw <= BufferAngle -> Buffer
DeltaYaw <= EdgeAngle -> Edge
否则 -> Offscreen
```

## 三、目标搜索与锁定

### 目标必须满足

当前目标检测只查找 `ECC_Pawn` 对象，并额外检查：

- 目标 Actor 有 `LockTargetTag`
- 目标不是自己
- 目标没有 PendingKill
- 目标 2D 距离不超过 `LockAcquireRadius`
- 目标和玩家的高度差不超过 `MaxLockVerticalDelta`

### 初次锁定目标评分

`FindBestLockTarget` 会在候选目标中评分：

```text
Score = Dot(CameraForward, ToTarget) * 1000 - Distance
如果目标在前方，再额外 +200
```

结果：

- 越靠近镜头前方，分数越高
- 越近，分数越高
- 在镜头前半球内有额外加分

### 切换目标评分

`FindSwitchTarget` 根据输入方向选择左侧或右侧目标。

方向判断：

```text
Side = Dot(CameraRight, ToTarget)
DirectionSign > 0 -> 只接受右侧目标
DirectionSign < 0 -> 只接受左侧目标
```

评分：

```text
Score = Front * 1000 - Distance + Abs(Side) * 100
```

切换后会进入 `TargetSwitchCooldown = 0.2` 秒冷却，避免连续快速切换。

## 四、蓝图访问方式

在 `BP_RDBaseCharacter` 里可以直接从角色拿常用状态：

```text
Get Current Lock Target
Get Previous Lock Target
Get Combat Camera State
Get Current Lock On Zone
Get Targeting Camera Component
```

如果要访问组件上的完整参数：

```text
选中 BP_RDBaseCharacter
选中组件面板里的 TargetingCameraComponent
在 Details 面板查看 Camera|Detect、Camera|LockOn、Camera|Interp 等分类
```

## 五、可配置参数

以下参数都在 `TargetingCameraComponent` 上配置。

### Camera|Detect

| 参数 | 默认值 | 作用 | 调整建议 |
| --- | ---: | --- | --- |
| `CombatDetectRadius` | `1800` | 用于判断附近是否有战斗目标。影响 `Explore` 和 `CombatFree` 的切换。 | 想更早进入战斗自由镜头就调大。 |
| `LockAcquireRadius` | `2200` | 允许锁定目标的最大 2D 距离。也用于切换目标候选范围。 | 太大会锁到远处敌人，太小会导致锁不上。 |
| `MaxLockVerticalDelta` | `500` | 玩家和目标之间允许的最大高度差。 | 立体关卡可调大；平面战斗可调小。 |
| `LockTargetTag` | `LockTarget` | 只有带这个 Tag 的 Actor 才能被锁定。 | 需要在敌人或可锁定目标上添加同名 Tag。 |

### Camera|State

| 参数 | 默认值 | 作用 | 调整建议 |
| --- | ---: | --- | --- |
| `UnlockRecoveryDelay` | `1.0` | 退出锁定后保持 `Recover` 状态的时间。 | 想快速回到探索镜头就调小；想退出锁定后镜头更稳就调大。 |
| `UnlockRecoveryRemaining` | `0` | 运行时剩余恢复时间。 | 运行时状态，不建议手动调。 |

### Camera|Explore

| 参数 | 默认值 | 作用 | 调整建议 |
| --- | ---: | --- | --- |
| `ExploreArmLength` | `420` | 探索状态下 SpringArm 长度。 | 越大镜头越远。 |
| `ExploreSocketOffset` | `(0, 50, 65)` | 探索状态下相机末端偏移。 | `Y` 控制肩部左右偏移，`Z` 控制高度。 |
| `ExploreFOV` | `60` | 探索状态下 FOV。 | 越大视野越广，但变形更明显。 |
| `ExploreMinPitch` | `-50` | 探索状态下最低俯仰角。 | 更负可以看得更低。 |
| `ExploreMaxPitch` | `25` | 探索状态下最高俯仰角。 | 更大可以看得更高。 |

### Camera|CombatFree

| 参数 | 默认值 | 作用 | 调整建议 |
| --- | ---: | --- | --- |
| `CombatArmLength` | `390` | 战斗自由状态下 SpringArm 长度。 | 通常比探索略近，让战斗更紧凑。 |
| `CombatSocketOffset` | `(0, 35, 70)` | 战斗自由状态下相机末端偏移。 | 可略微降低肩部偏移，减少近战遮挡。 |
| `CombatFOV` | `58` | 战斗自由状态下 FOV。 | 通常略小于探索，画面更稳。 |
| `CombatMinPitch` | `-25` | 战斗自由状态下最低俯仰角。 | 限制低头角度，避免战斗中镜头过度下压。 |
| `CombatMaxPitch` | `15` | 战斗自由状态下最高俯仰角。 | 限制抬头角度，保持战斗可读性。 |

### Camera|LockOn 距离分段

| 参数 | 默认值 | 作用 | 调整建议 |
| --- | ---: | --- | --- |
| `NearDistance` | `350` | 近距离锁定阈值。 | 小于等于该距离时使用 Near 配置。 |
| `MidDistance` | `800` | 中距离锁定阈值。 | 小于等于该距离时使用 Mid 配置；超过则使用 Far。 |
| `LockNearArmLength` | `430` | 近距离锁定时 SpringArm 长度。 | 近战太压迫可调大。 |
| `LockMidArmLength` | `380` | 中距离锁定时 SpringArm 长度。 | 影响最常见锁定距离的镜头距离。 |
| `LockFarArmLength` | `360` | 远距离锁定时 SpringArm 长度。 | 远距离想看清双方可适当调大。 |
| `LockNearSocketOffset` | `(0, 20, 80)` | 近距离锁定时相机偏移。 | 提高 `Z` 可减少角色遮挡目标。 |
| `LockMidSocketOffset` | `(0, 15, 75)` | 中距离锁定时相机偏移。 | 通常作为主调参数。 |
| `LockFarSocketOffset` | `(0, 10, 70)` | 远距离锁定时相机偏移。 | 远距离可减少侧偏移，让目标更居中。 |
| `LockNearFOV` | `60` | 近距离锁定时 FOV。 | 近战可略高，保留周边信息。 |
| `LockMidFOV` | `57` | 中距离锁定时 FOV。 | 常用锁定 FOV。 |
| `LockFarFOV` | `55` | 远距离锁定时 FOV。 | 远距离略低可以聚焦目标。 |
| `LockMinPitch` | `-20` | 锁定状态最低俯仰角。 | 控制锁定时镜头向下看目标的极限。 |
| `LockMaxPitch` | `18` | 锁定状态最高俯仰角。 | 控制锁定时镜头向上看目标的极限。 |

### Camera|LockOn 区域角度

| 参数 | 默认值 | 作用 | 调整建议 |
| --- | ---: | --- | --- |
| `CenterAngle` | `15` | 判定目标处于中心区的 yaw 差阈值。 | 越小越严格，越容易进入 Buffer。 |
| `BufferAngle` | `35` | 判定目标处于缓冲区的 yaw 差阈值。 | 用于过渡到更快的锁定旋转速度。 |
| `EdgeAngle` | `65` | 判定目标处于边缘区的 yaw 差阈值。 | 超过该值视为 Offscreen。 |
| `LockRotationHysteresisAngle` | `3` | 锁定相机旋转启停滞回角度。 | 用于减少 `CenterAngle` 边界附近的反复启停和抽搐。 |

当 `RotationInterpSpeed_Lock_Center = 0` 时，系统会使用 `LockRotationHysteresisAngle` 防止边界抖动。以默认值为例：

```text
CenterAngle = 15
LockRotationHysteresisAngle = 3

开始旋转阈值 = 15 + 3 = 18
停止旋转阈值 = 15 - 3 = 12
```

也就是说，目标偏离到 `18` 度以上才会启动相机旋转；旋转开始后，会持续把目标拉回到 `12` 度以内才停止。这样可以避免目标在 `15` 度附近来回穿越时，出现一帧开、一帧关的抽搐。

`Offscreen` 时当前逻辑会额外调整：

```text
DesiredArmLength += 30
DesiredSocketOffset.Z += 10
```

### Camera|LockOn|Debug

| 参数 | 默认值 | 作用 | 调整建议 |
| --- | ---: | --- | --- |
| `bDebugDrawLockAngle` | `false` | 是否在游戏中绘制锁定角度 Debug。 | 调试 `CenterAngle`、`BufferAngle`、`EdgeAngle` 时打开，正式游玩时关闭。 |
| `DebugDrawLength` | `500` | Debug 箭头长度。 | 场景尺度较大时可调大。 |

打开 `bDebugDrawLockAngle` 后，进入锁定状态时会绘制：

| 图形 | 含义 |
| --- | --- |
| 青色箭头 | 从玩家 Pivot 出发，表示当前 `Controller / 相机` 的水平朝向。 |
| 彩色箭头 | 从玩家 Pivot 出发，表示 `玩家 -> 索敌目标` 的水平连线方向。 |
| 彩色直线 | 玩家 Pivot 到目标 Pivot 的水平连线。 |
| 青色球 | 玩家 Pivot 调试点。 |
| 彩色球 | 目标 Pivot 调试点。 |
| 文本 | 当前 `Lock DeltaYaw`、`CenterAngle`、`BufferAngle`、`EdgeAngle`、`RotActive`。 |

颜色对应当前 `ELockOnZone`：

| 颜色 | Zone |
| --- | --- |
| 绿色 | `Center` |
| 黄色 | `Buffer` |
| 橙色 | `Edge` |
| 红色 | `Offscreen` |

`Lock DeltaYaw` 的计算含义：

```text
Lock DeltaYaw = Abs(FindDeltaAngleDegrees(CurrentControlYaw, TargetYaw))
```

其中：

- `CurrentControlYaw` 是当前 `PlayerController.ControlRotation.Yaw`
- `TargetYaw` 是 `玩家 Pivot -> 目标 Pivot` 方向的 Yaw

也就是说，Debug 中的两条箭头在水平面上的夹角，就是当前用于判断 `Center / Buffer / Edge / Offscreen` 的角度。

`RotActive` 表示当前锁定相机是否正在允许旋转。开启 `RotationInterpSpeed_Lock_Center = 0` 和滞回后，可以用它观察相机是否在启停边界反复切换。

### Camera|Interp

| 参数 | 默认值 | 作用 | 调整建议 |
| --- | ---: | --- | --- |
| `ArmInterpSpeed` | `8` | SpringArm 长度插值速度。 | 越大距离变化越快。 |
| `OffsetInterpSpeed` | `8` | SocketOffset 插值速度。 | 越大偏移变化越快。 |
| `RotationInterpSpeed_Explore` | `8` | 探索状态下控制旋转插值速度。 | 探索状态通常不需要过快。 |
| `RotationInterpSpeed_Combat` | `10` | 战斗自由和恢复状态下控制旋转插值速度。 | 战斗中可略快。 |
| `RotationInterpSpeed_Lock_Center` | `5` | 目标在中心区时锁定旋转速度。 | 设为 `0` 时会停止相机旋转，适合实现中心区不追踪。 |
| `RotationInterpSpeed_Lock_Buffer` | `7` | 目标在缓冲区时锁定旋转速度。 | 中等追踪速度。 |
| `RotationInterpSpeed_Lock_Edge` | `11` | 目标在边缘区时锁定旋转速度。 | 边缘区需要更快拉回。 |
| `RotationInterpSpeed_Lock_Offscreen` | `15` | 目标出屏时锁定旋转速度。 | 越大越快把目标拉回画面。 |
| `FOVInterpSpeed` | `8` | FOV 插值速度。 | 太大变化突兀，太小响应慢。 |

当前组件对 `RotationInterpSpeed <= 0` 做了特殊处理：相机会保留当前 `ControlRotation`，不会调用 `FMath::RInterpTo`。这是为了让 `0` 可以表达“停止旋转”。UE 原生 `RInterpTo` 在 `InterpSpeed <= 0` 时会直接跳到目标值，因此不要把这两者混淆。

### Camera|Orbit

这些参数是锁定状态下手动 Orbit 输入的保留参数。

当前版本进入 `LockOn` 后鼠标 Look 输入会被禁止，因此默认不会再改变 Orbit。

| 参数 | 默认值 | 当前用途 |
| --- | ---: | --- |
| `LockOrbitYawMax` | `20` | 保留。限制锁定 Orbit Yaw 最大角度。 |
| `LockOrbitPitchMax` | `8` | 保留。限制锁定 Orbit Pitch 最大角度。 |
| `LockOrbitYawInputScale` | `2.5` | 保留。锁定 Orbit Yaw 输入缩放。 |
| `LockOrbitPitchInputScale` | `1.2` | 保留。锁定 Orbit Pitch 输入缩放。 |
| `LockOrbitDecaySpeed` | `5` | 当前仍会把已有 Orbit 值衰减回 0。 |
| `HeightPitchFactor` | `0.03` | 仍在使用。根据目标和玩家高度差调整锁定 Pitch。 |

如果以后想恢复“锁定时允许轻微鼠标偏移”，需要在 `HandleLookInput` 的 `LockOn` 分支里重新累加：

```text
PendingOrbitYawInput
PendingOrbitPitchInput
```

## 六、运行时状态

这些变量由系统运行时维护，不建议手动配置：

| 变量 | 含义 |
| --- | --- |
| `CurrentLockTarget` | 当前锁定目标。 |
| `PreviousLockTarget` | 上一个锁定目标。退出或切换时更新。 |
| `CurrentZone` | 当前锁定目标所在区域。 |
| `UnlockRecoveryRemaining` | 解锁恢复剩余时间。 |
| `DesiredArmLength` | 当前帧计算出的目标 SpringArm 长度。 |
| `DesiredSocketOffset` | 当前帧计算出的目标 SocketOffset。 |
| `DesiredControlRotation` | 当前帧计算出的目标控制旋转。 |
| `DesiredFOV` | 当前帧计算出的目标 FOV。 |
| `TargetSwitchCooldown` | 切换目标冷却时间。 |

## 七、常见问题

### 蓝图里找不到 `CurrentLockTarget`

`CurrentLockTarget` 已经从角色变量移动到 `TargetingCameraComponent`。

现在有两种拿法：

```text
As BP_RDBaseCharacter -> Get Current Lock Target
As BP_RDBaseCharacter -> Get Targeting Camera Component -> Get Current Lock Target
```

### 锁不上目标

检查：

- 目标 Actor 是否带有 `LockTarget` Tag
- 目标是否是 `ECC_Pawn`
- 目标是否超过 `LockAcquireRadius`
- 目标和玩家高度差是否超过 `MaxLockVerticalDelta`
- 输入是否正确调用了 `ToggleLockOn`

### 锁定后鼠标不能转镜头

这是当前设计。

锁定状态下 `HandleLookInput` 会直接返回，不处理鼠标 Look 输入。

### 退出锁定后镜头没有立刻回探索状态

这是 `UnlockRecoveryDelay` 的作用。

退出锁定后会先进入 `Recover`，持续 `UnlockRecoveryDelay` 秒，然后再根据附近是否有目标进入 `CombatFree` 或 `Explore`。
