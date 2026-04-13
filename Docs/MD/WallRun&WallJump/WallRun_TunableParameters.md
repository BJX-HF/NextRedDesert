# UE5 墙跑系统可调参数说明

## 一、角色上的墙跑参数

以下参数在 `ADMCameraCharacter` 上调整。

### `bDebugWallRun`

- 类型：`bool`
- 默认值：`false`
- 作用：是否启用墙跑调试输出

启用后会输出：

- 文字调试信息

### `MinSpeedToStartWallRun`

- 类型：`float`
- 默认值：`550`
- 作用：进入墙跑所需的最低水平速度

### `MinSpeedToMaintainWallRun`

- 类型：`float`
- 默认值：`400`
- 作用：维持墙跑所需的最低速度

### `MinWallAngleFromUpDeg`

- 类型：`float`
- 默认值：`65`
- 作用：墙面法线与世界 `Up` 的最小夹角
- 用于排除更像地面的表面

### `MaxWallAngleFromUpDeg`

- 类型：`float`
- 默认值：`115`
- 作用：墙面法线与世界 `Up` 的最大夹角
- 用于排除更像天花板的表面

### `MinApproachAngleDeg`

- 类型：`float`
- 默认值：`15`
- 可调范围：`0 ~ 180`
- 作用：允许进入墙跑的最小接近角

### `MaxApproachAngleDeg`

- 类型：`float`
- 默认值：`75`
- 可调范围：`0 ~ 180`
- 作用：允许进入墙跑的最大接近角

### 接近角当前的定义

当前版本中，接近角已经改为基于角色朝向，而不是基于角色速度方向。

公式：

- `HorizontalForward = GetActorForwardVector().GetSafeNormal2D()`
- `IntoWallDirection = (-WallNormal).GetSafeNormal2D()`
- `ApproachAngle = acos(dot(HorizontalForward, IntoWallDirection))`

含义：

- `0` 度：角色正对着墙
- `90` 度：角色朝向与墙面平行
- `180` 度：角色背对着墙

调参建议：

- 严格正面切入：`0 ~ 45`
- 中等宽松：`0 ~ 60`
- 更自由的切入：`0 ~ 90`

### `JumpToWallGraceTime`

- 类型：`float`
- 默认值：`0.30`
- 作用：按下 Jump 后，允许进入墙跑的时间窗口

### `WallRunSpeed`

- 类型：`float`
- 默认值：`900`
- 作用：墙跑目标速度

### `WallRunAcceleration`

- 类型：`float`
- 默认值：`2200`
- 作用：墙跑过程中速度逼近目标速度的加速强度

### `WallRunGravityScale`

- 类型：`float`
- 默认值：`0.25`
- 作用：墙跑期间重力缩放
- 越小越“轻”

### `WallAttractionForce`

- 类型：`float`
- 默认值：`350`
- 作用：把角色吸附向墙面的力度

### `WallRunMaxDuration`

- 类型：`float`
- 默认值：`1.25`
- 作用：墙跑最大持续时间

### `LostWallForgivenessTime`

- 类型：`float`
- 默认值：`0.12`
- 作用：短暂丢失墙面后的容错时间

### `WallJumpHorizontalStrength`

- 类型：`float`
- 默认值：`520`
- 作用：墙跳时沿墙法线反推的水平力度

### `WallJumpVerticalStrength`

- 类型：`float`
- 默认值：`650`
- 作用：墙跳时向上的力度

### `WallJumpForwardBoost`

- 类型：`float`
- 默认值：`140`
- 作用：墙跳时额外给予角色前向推进

### `WallRunRotationInterpSpeed`

- 类型：`float`
- 默认值：`12`
- 作用：墙跑时角色朝向插值速度

### `WallJumpAnimFlagDuration`

- 类型：`float`
- 默认值：`0.2`
- 作用：墙跳动画标记持续时长

### `WallJumpReattachBlockTime`

- 类型：`float`
- 默认值：`0.25`
- 作用：墙跳后禁止重新吸附回刚刚那块墙跑组件的保护时间

说明：

- 这个参数用于避免墙跳后立刻重新吸回原墙
- 同时也能减少“同一面墙左右侧状态突然翻转”的问题
- 如果墙跳后仍然太容易被重新吸回原墙，可以适当调大
- 如果你希望墙跳后更快允许重新挂墙，可以适当调小

## 二、运行时状态和动画蓝图接口

### 主要运行时状态

- `bIsWallRunning`
- `CurrentWallNormal`
- `CurrentWallRunDirection`
- `CurrentWallRunSide`
- `WallRunStartTime`
- `LastJumpPressedTime`
- `bWallJumpRequested`
- `LastWallJumpSide`
- `OverlappingWallRunSurfaces`

### AnimBP 常用接口

- `IsWallRunning()`
- `IsWallJumping()`
- `WasWallJumpRequestedThisFrame()`
- `GetWallRunSide()`
- `IsWallRunningOnLeft()`
- `IsWallRunningOnRight()`
- `GetWallJumpSide()`
- `IsWallJumpFromLeft()`
- `IsWallJumpFromRight()`
- `GetWallRunDirection()`
- `GetWallNormal()`
- `GetWallRunSpeed()`

## 三、墙跑组件 `UWallRunSurfaceComponent` 参数

### `bEnabled`

- 类型：`bool`
- 默认值：`true`
- 作用：是否启用该墙跑区域

### `NormalAxis`

- 类型：`EWallRunNormalAxis`
- 默认值：`Forward`
- 作用：用组件的哪个朝向轴作为墙法线

可选值：

- `Forward`
- `Backward`
- `Right`
- `Left`

### `AllowedDirection`

- 类型：`EWallRunAllowedDirection`
- 默认值：`Bidirectional`
- 作用：限制墙跑方向

可选值：

- `Bidirectional`
- `PositiveOnly`
- `NegativeOnly`

含义：

- `Bidirectional`
  - 可双向墙跑
- `PositiveOnly`
  - 只允许沿正向墙跑
- `NegativeOnly`
  - 只允许沿反向墙跑

### `WallRunSpeedOverride`

- 类型：`float`
- 默认值：`0`
- 作用：覆盖角色默认墙跑速度
- `0` 表示不覆盖

### `WallRunMaxDurationOverride`

- 类型：`float`
- 默认值：`0`
- 作用：覆盖角色默认墙跑持续时长
- `0` 表示不覆盖

### `bDebugDraw`

- 类型：`bool`
- 默认值：`false`
- 作用：是否绘制 `WallRunSurfaceComponent` 自己的方向箭头

说明：

- 蓝色箭头：墙法线
- 绿色箭头：正向沿墙方向
- 这个调试只属于组件自身
- 不会恢复角色侧的墙跑判定箭头

## 四、角色如何进入墙跑

当前进入墙跑时会检查：

1. 当前必须在 `Falling`
2. 当前水平速度必须达到 `MinSpeedToStartWallRun`
3. 必须仍在 `JumpToWallGraceTime` 的宽限时间内
4. 必须存在有效的 `WallRunSurfaceComponent` 候选
5. 墙面法线角度必须在合法范围内
6. 接近角必须在合法范围内
7. 最终沿墙方向必须合法

## 五、左右墙跑与左右墙跳

### `EWallRunSide`

取值：

- `None`
- `Left`
- `Right`

用途：

- `Left`：墙在角色左边，用于驱动左墙跑 / 左墙跳动画
- `Right`：墙在角色右边，用于驱动右墙跑 / 右墙跳动画

当前侧别判定规则：

- 不再直接使用角色当前 `RightVector` 判断左右
- 当前实现改为基于“最终沿墙跑方向”和“角色到墙最近点方向”的关系来判断左右侧
- 这样同一面墙在角色朝向变化时，左右侧结果会更稳定

额外稳定性处理：

- 墙跑过程中，如果还是同一块 `WallRunSurfaceComponent`
- 系统不会每一帧都重新刷新左右侧
- 只有切换墙面，或者当前侧别还是 `None` 时，才会重新计算

### 推荐的 AnimBP 缓存变量

- `bIsWallRunning`
- `bWallRunLeft`
- `bWallRunRight`
- `WallRunSide`
- `WallRunSpeed`
- `bWallJump`
- `WallJumpSide`
- `bWallJumpFromLeft`
- `bWallJumpFromRight`

## 六、Debug 信息说明

### 文字调试

- `Overlapped!`
  - 角色检测盒已进入墙跑区域
- `Character Falling!`
  - 当前在 `Falling`，且速度已达标
- `Angle Error! Current: xx, Allowed: xx - xx`
  - 接近角不合法
- `Wall Angle Error! Current: xx, Allowed: xx - xx`
  - 墙面法线角度不合法
- `WallRun Blocked: invalid state or surface`
  - 当前状态或候选组件无效
- `WallRun Blocked: not falling or surface disabled`
  - 当前不在 Falling，或组件被禁用
- `WallRun Blocked: jump grace expired (Current: xx, Allowed: xx)`
  - 起跳后进入墙跑的时机太晚
- `WallRun Blocked: speed too low (Current: xx, Required: xx)`
  - 当前水平速度不够
- `WallRun Blocked: invalid wall run direction`
  - 沿墙方向不合法或不被允许
- `WallRun Ready: all conditions passed`
  - 全部进入条件已满足
- `WallRun: No valid surface candidate`
  - 当前没有可用候选墙跑组件
