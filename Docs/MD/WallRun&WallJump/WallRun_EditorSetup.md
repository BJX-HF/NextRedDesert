# DmcCameraCharacter 墙跑功能需要手动配置的内容

## 1. 给可墙跑表面加 Tag

- 默认实现用 `Actor Tag` 或 `Component Tag` 过滤，名称是 `WallRun`。
- 在关卡里的墙体 Actor 上添加 `WallRun`，或在对应静态网格组件上添加 `WallRun`。
- 如果你后续想改成专用碰撞通道，可以保留 `IsWallRunnableHit()` 的结构，把 Tag 判定替换成碰撞响应判定。

## 2. 冲刺状态接入

- 当前类里新增了 `bIsSprintActive`，这是占位变量。
- 如果你的项目已有真实冲刺系统，请把 `ADMCameraCharacter::IsSprintStateActive()` 改成读取真实状态。
- 如果暂时还没有冲刺系统，但想先测试墙跑，可以直接在角色蓝图实例里让 `bIsSprintActive = true`。

## 3. 角色蓝图默认参数建议

- `bSprintOnlyCanWallRun = true`
- `MinSpeedToStartWallRun = 550`
- `MinSpeedToMaintainWallRun = 400`
- `WallDetectDistance = 90`
- `WallDetectHeightOffset = 55`
- `MinWallAngleFromUpDeg = 65`
- `MaxWallAngleFromUpDeg = 115`
- `MinApproachAngleDeg = 15`
- `MaxApproachAngleDeg = 75`
- `JumpToWallGraceTime = 0.30`
- `WallRunSpeed = 900`
- `WallRunAcceleration = 2200`
- `WallRunGravityScale = 0.25`
- `WallAttractionForce = 350`
- `WallRunMaxDuration = 1.25`
- `LostWallForgivenessTime = 0.12`
- `WallJumpHorizontalStrength = 520`
- `WallJumpVerticalStrength = 650`
- `WallJumpForwardBoost = 140`
- `WallRunRotationInterpSpeed = 12`
- `WallTraceSphereRadius = 16`
- `WallJumpAnimFlagDuration = 0.20`

## 4. 角色类替换确认

- 该实现会让 `ADMCameraCharacter` 使用新的 `UDmcCameraCharacterMovementComponent`。
- 如果你有基于该角色的 BP 子类，重新编译后确认 `Character Movement` 组件没有被别的自定义类覆盖掉。

## 5. 动画蓝图可读取变量

- `IsWallRunning()`
- `IsWallJumping()`
- `GetWallRunSide()`
- `GetWallRunDirection()`
- `GetWallNormal()`
- `GetWallRunSpeed()`

## 6. 可选碰撞建议

- 当前默认 `WallTraceChannel = Visibility`。
- 如果你的墙体不会阻挡 `Visibility`，需要把该参数改到能命中墙体的通道。
- 如果某些不该墙跑的物体也挡住了该通道，优先继续用 `WallRun` Tag 细分过滤。
