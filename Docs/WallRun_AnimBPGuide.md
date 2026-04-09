# DmcCameraCharacter 墙跑功能 AnimBP 接法建议

## 1. 在 AnimBP Event Graph 里同步变量

每帧从拥有者角色 `ADMCameraCharacter` 读取：

- `bIsWallRunning = Character->IsWallRunning()`
- `WallRunSide = Character->GetWallRunSide()`
- `WallRunSpeed = Character->GetWallRunSpeed()`
- `bWallJump = Character->IsWallJumping()`

建议同时保留已有：

- `Speed`
- `bIsInAir`
- `Direction`

## 2. 推荐状态机结构

新增一个 `WallRun` 子状态区域，最少包含以下状态：

- `WallRunStart`
- `WallRunLoop_Left`
- `WallRunLoop_Right`
- `WallRunEnd`
- `WallJump`

## 3. 过渡条件建议

### Falling -> WallRunStart

- 条件：`bIsWallRunning == true`

### WallRunStart -> WallRunLoop_Left

- 条件：`bIsWallRunning == true && WallRunSide == Left`
- 可额外加：`TimeRemaining(WallRunStart) < 0.1`

### WallRunStart -> WallRunLoop_Right

- 条件：`bIsWallRunning == true && WallRunSide == Right`
- 可额外加：`TimeRemaining(WallRunStart) < 0.1`

### WallRunLoop_Left -> WallRunLoop_Right

- 条件：`bIsWallRunning == true && WallRunSide == Right`

### WallRunLoop_Right -> WallRunLoop_Left

- 条件：`bIsWallRunning == true && WallRunSide == Left`

### WallRunLoop -> WallJump

- 条件：`bWallJump == true`

### WallRunLoop -> WallRunEnd

- 条件：`bIsWallRunning == false && bWallJump == false`

### WallRunEnd -> Falling / Locomotion

- 条件：`TimeRemaining(WallRunEnd) < 0.1`
- 再根据 `bIsInAir` 选择回 `Falling` 或 `Locomotion`

## 4. BlendSpace / 方向建议

- `WallRunStart` 可做单独起始蒙太奇或 Sequence。
- `WallRunLoop_Left` / `WallRunLoop_Right` 建议分别使用左右版本循环动画。
- `WallRunSpeed` 可作为播放速率驱动，常见范围可映射到 `0.8 ~ 1.2`。
- 如果你想让角色上半身更稳定，可以在墙跑循环里叠加少量 Aim Offset 或 Spine 修正。

## 5. bWallJump 的使用建议

- `bWallJump` 是短时脉冲标记，不建议做持久状态。
- 适合只用于 `WallRunLoop -> WallJump` 的瞬时切换。
- `WallJump` 动画播完后，正常回到 `Falling`。

## 6. 额外建议

- 如果你的 AnimBP 已经在读 `MovementMode`，也可以额外判断 `CustomMovementMode == CMOVE_WallRun` 做调试。
- 若要区分“左墙跑”和“右墙跑”的倾斜姿态，建议在 Anim Graph 里用 `WallRunSide` 驱动一个左右倾斜的 `Blend Poses by Enum`。
