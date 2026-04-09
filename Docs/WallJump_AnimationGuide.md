# UE5 WallJump 动画配置指南

## 1. 当前可直接用于 AnimBP 的接口

从 `ADMCameraCharacter` 读取这些接口：

- `IsWallJumping()`
- `WasWallJumpRequestedThisFrame()`
- `GetWallJumpSide()`
- `IsWallJumpFromLeft()`
- `IsWallJumpFromRight()`
- `IsWallRunning()`
- `GetWallRunSide()`
- `IsWallRunningOnLeft()`
- `IsWallRunningOnRight()`

这些接口已经适合在 AnimBP 的 Event Graph 中每帧缓存。

## 2. 建议在 AnimBP 缓存的变量

建议新增：

- `bWallJump`
- `WallJumpSide`
- `bWallJumpFromLeft`
- `bWallJumpFromRight`
- `bIsWallRunning`
- `WallRunSide`

推荐赋值方式：

- `bWallJump = Character->IsWallJumping()`
- `WallJumpSide = Character->GetWallJumpSide()`
- `bWallJumpFromLeft = Character->IsWallJumpFromLeft()`
- `bWallJumpFromRight = Character->IsWallJumpFromRight()`
- `bIsWallRunning = Character->IsWallRunning()`
- `WallRunSide = Character->GetWallRunSide()`

## 3. WallJump 触发条件

当前代码中，WallJump 只会在这些条件都满足时触发：

- `bEnableWallJump == true`
- 当前角色确实处于墙跑中
- 当前移动模式是 `CMOVE_WallRun`
- 当前存在有效的墙跑 Surface
- 当前墙法线和沿墙方向有效

也就是说：

- 普通地面跳跃不会触发 WallJump
- 空中 Falling 但未进入墙跑时也不会触发 WallJump
- 只有墙跑生效期间再次按 Jump，才会触发 WallJump

## 4. 推荐的状态机结构

建议新增这些状态：

- `WallRunStart`
- `WallRunLoop_Left`
- `WallRunLoop_Right`
- `WallJump_Left`
- `WallJump_Right`
- `WallRunEnd`

## 5. 建议的状态切换

### `Falling -> WallRunStart`

- 条件：`bIsWallRunning == true`

### `WallRunStart -> WallRunLoop_Left`

- 条件：`IsWallRunningOnLeft() == true`

### `WallRunStart -> WallRunLoop_Right`

- 条件：`IsWallRunningOnRight() == true`

### `WallRunLoop_Left -> WallJump_Left`

- 条件：`bWallJump == true && bWallJumpFromLeft == true`

### `WallRunLoop_Right -> WallJump_Right`

- 条件：`bWallJump == true && bWallJumpFromRight == true`

### `WallJump_Left / Right -> Falling`

- 动画播完后返回 `Falling`

### `WallRunLoop_Left / Right -> WallRunEnd`

- 条件：`bIsWallRunning == false && bWallJump == false`

## 6. 为什么建议左右分开做 WallJump

因为当前代码已经区分：

- 左墙跑起跳
- 右墙跑起跳

所以你可以给角色做：

- 左脚主发力的左墙跳
- 右脚主发力的右墙跳

动画上会更自然，也更容易和墙跑过渡对齐。

## 7. 最简单接法

如果你暂时不想做完整状态机，也可以先：

- 用 `Blend Poses by Bool` 判断 `bWallJump`
- 再用 `Blend Poses by Enum` 或两个 Bool 区分左右墙跳

例如：

- `bWallJumpFromLeft == true` 播放左墙跳
- `bWallJumpFromRight == true` 播放右墙跳

## 8. 动画通知建议

如果你后续想做更强的表现，可以在 WallJump 动画里加 Anim Notify：

- 起跳帧播放音效
- 起跳帧播放脚底粒子
- 起跳帧短暂开镜头震动

## 9. 当前与代码一致的结论

当前 WallJump 已经满足：

- 仅在墙跑生效时，Jump 才会触发 WallJump
- 左右墙跳已区分
- WallJump 触发状态可以直接供 AnimBP 使用
