# UE5 墙跑系统引擎内编辑操作指南

## 1. 当前方案概览

当前墙跑系统使用两部分协作：

- 角色 `ADMCameraCharacter`
  - 挂有 `WallRunDetector`，用于检测墙跑区域重叠
- 墙面组件 `UWallRunSurfaceComponent`
  - 任意墙体 Actor 或蓝图只要挂上这个组件，就可以成为可墙跑区域

这套方案不再依赖普通墙体的 Tag 检测，也不依赖普通墙体命中法线。
墙跑区域由关卡显式标记，法线和方向也由组件自身定义，适合可控关卡设计。

## 2. 你需要在引擎里做的操作

### 2.1 给墙体添加墙跑组件

1. 打开你的墙体蓝图，或者选中关卡里的某个墙体 Actor
2. 添加组件 `WallRunSurfaceComponent`
3. 调整这个组件的：
   - 位置
   - 旋转
   - Box Extent
4. 确保这个 Box 覆盖角色允许进入墙跑的区域

### 2.2 调整角色上的检测盒

角色已经在 C++ 中创建了 `WallRunDetector`。
默认参数：

- Box Extent: `65, 75, 95`
- Relative Location: `0, 0, 10`

如果你的角色体型不同，建议在角色蓝图里调整 `WallRunDetector` 的大小，原则是：

- 能稳定重叠到墙跑区域
- 又不要大到穿透过多无关区域

### 2.3 打开调试

如果要启用墙跑调试，在角色蓝图里打开：

- `bDebugWallRun`

如果要查看墙跑组件自己的方向箭头，可在 `WallRunSurfaceComponent` 上打开：

- `bDebugDraw`

## 3. 如何定义墙面法线

`UWallRunSurfaceComponent` 使用自身朝向来定义墙面法线。
关键参数：

- `NormalAxis`

可选值：

- `Forward`
- `Backward`
- `Right`
- `Left`

含义：

- `Forward`：组件 `ForwardVector` 作为墙法线
- `Backward`：组件 `-ForwardVector` 作为墙法线
- `Right`：组件 `RightVector` 作为墙法线
- `Left`：组件 `-RightVector` 作为墙法线

当前默认值：

- `Forward`

推荐做法：

- 让组件的前向箭头朝墙外
- 保持 `NormalAxis = Forward`

如果方向不对：

- 检查组件旋转是否正确
- 检查 `NormalAxis` 是否与你的摆放方式一致
- 如有需要，打开 `bDebugDraw` 观察组件方向箭头

箭头说明：

- 蓝色箭头：墙法线
- 绿色箭头：正向沿墙方向

## 4. 如何控制墙跑方向

墙跑组件上有参数：

- `AllowedDirection`

可选值：

- `Bidirectional`
- `PositiveOnly`
- `NegativeOnly`

含义：

- `Bidirectional`
  - 允许沿墙双向墙跑
  - 系统会根据当前情况自动选择最终沿墙方向
- `PositiveOnly`
  - 只允许沿组件定义的正向沿墙方向墙跑
- `NegativeOnly`
  - 只允许沿组件定义的反向沿墙方向墙跑

## 5. 角色如何进入墙跑

当前进入墙跑的大致顺序：

1. 角色在空中
2. `WallRunDetector` 与一个或多个 `WallRunSurfaceComponent` 重叠
3. 角色在 `Falling`
4. 角色水平速度达到 `MinSpeedToStartWallRun`
5. 角色仍处于 `JumpToWallGraceTime` 的宽限时间内
6. 当前墙面法线角度合法
7. 当前接近角合法
8. 方向限制合法
9. 切换到 `MOVE_Custom + CMOVE_WallRun`

## 6. 接近角现在是怎么定义的

当前版本中，接近角已经改为基于角色当前正方向，而不是基于角色水平速度方向。

公式：

- `HorizontalForward = GetActorForwardVector().GetSafeNormal2D()`
- `IntoWallDirection = (-WallNormal).GetSafeNormal2D()`
- `ApproachAngle = acos(dot(HorizontalForward, IntoWallDirection))`

含义：

- `0` 度：角色正对着墙
- `90` 度：角色朝向与墙面平行
- `180` 度：角色背对着墙

因此：

- `MinApproachAngleDeg`
- `MaxApproachAngleDeg`

现在表示的是“角色朝向相对于朝墙方向的允许偏差范围”。

常用建议：

- 偏严格的正面切入：`0 ~ 45`
- 中等宽松：`0 ~ 60`
- 更自由的切入：`0 ~ 90`

## 7. 左右墙跑与左右墙跳

墙跑和墙跳都区分左右侧，便于动画蓝图驱动。

角色可读接口：

- `GetWallRunSide()`
- `IsWallRunningOnLeft()`
- `IsWallRunningOnRight()`
- `GetWallJumpSide()`
- `IsWallJumpFromLeft()`
- `IsWallJumpFromRight()`

含义：

- `Left`：墙在角色左侧
- `Right`：墙在角色右侧

当前左右侧的判定规则：

- 不再直接依赖角色当前的 `RightVector`
- 而是基于“最终沿墙跑方向”与“角色到墙最近点方向”的关系来判断
- 这样即使角色朝向在墙跑或墙跳后发生变化，只要沿墙跑方向没变，同一面墙的左右侧结果也会更稳定

额外说明：

- 现在墙跑过程中不会对同一面墙每帧反复重算左右侧
- 只有在真正切换到另一块墙跑组件，或者当前侧别还是 `None` 时，才会重新计算

## 8. 动画蓝图如何配置

### 8.1 建议在 AnimBP 缓存的变量

每帧从角色读取：

- `bIsWallRunning = Character->IsWallRunning()`
- `bWallRunLeft = Character->IsWallRunningOnLeft()`
- `bWallRunRight = Character->IsWallRunningOnRight()`
- `WallRunSide = Character->GetWallRunSide()`
- `WallRunSpeed = Character->GetWallRunSpeed()`
- `bWallJump = Character->IsWallJumping()`
- `WallJumpSide = Character->GetWallJumpSide()`
- `bWallJumpFromLeft = Character->IsWallJumpFromLeft()`
- `bWallJumpFromRight = Character->IsWallJumpFromRight()`

### 8.2 建议的状态机

建议增加这些状态：

- `WallRunStart`
- `WallRunLoop_Left`
- `WallRunLoop_Right`
- `WallRunEnd`
- `WallJump`

### 8.3 建议的切换条件

- `Falling -> WallRunStart`
  - `bIsWallRunning == true`
- `WallRunStart -> WallRunLoop_Left`
  - `bWallRunLeft == true`
- `WallRunStart -> WallRunLoop_Right`
  - `bWallRunRight == true`
- `WallRunLoop -> WallJump`
  - `bWallJump == true`
- `WallRunLoop_Left / Right -> WallRunEnd`
  - `bIsWallRunning == false && bWallJump == false`
- `WallRunEnd -> Falling / Locomotion`
  - 根据是否还在空中切回

### 8.4 最简单接法

如果你暂时不想做复杂状态机，可以先这样：

- 用 `Blend Poses by Bool` 或 `Blend Poses by Enum`
- 左墙跑播左墙跑动画
- 右墙跑播右墙跑动画
- 左墙跳播左墙跳动画
- 右墙跳播右墙跳动画

## 9. 调试信息说明

当 `bDebugWallRun = true` 时，系统会输出文字调试。

### 9.1 文字调试

- `Overlapped!`
  - `WallRunDetector` 已进入某个 `WallRunSurfaceComponent` 的范围
- `Character Falling!`
  - 当前角色在 `Falling`，且水平速度已经达到进入墙跑的最低速度
- `Angle Error! Current: xx, Allowed: xx - xx`
  - 当前接近角不在允许范围内
- `Wall Angle Error! Current: xx, Allowed: xx - xx`
  - 当前墙面法线角度不在允许范围内
- `WallRun Blocked: invalid state or surface`
  - 当前状态或候选墙跑组件无效
- `WallRun Blocked: not falling or surface disabled`
  - 当前不在 `Falling`，或者墙跑组件被禁用
- `WallRun Blocked: jump grace expired (Current: xx, Allowed: xx)`
  - 跳起后进入墙跑判定的时机太晚
- `WallRun Blocked: speed too low (Current: xx, Required: xx)`
  - 当前水平速度不足
- `WallRun Blocked: invalid wall run direction`
  - 当前沿墙方向无效，或者不满足组件方向限制
- `WallRun Ready: all conditions passed`
  - 当前候选组件已通过全部进入判定
- `WallRun: No valid surface candidate`
  - 当前没有找到可用的墙跑组件候选

## 10. 多个墙跑组件候选时如何选择

如果角色同时重叠多个 `WallRunSurfaceComponent`，系统会综合选择更优候选。
主要会参考：

1. 组件是否有效且启用
2. 候选沿墙方向是否有效
3. 候选沿墙方向与当前移动方向的匹配程度
4. 角色到组件最近点的距离
5. 接近角合理性

## 11. 墙跳后重新吸附保护

当前版本增加了“墙跳后短时间禁止重新吸附”的保护，避免出现：

- 左墙跑时按跳
- 角色刚离墙又立刻被重新吸回墙跑
- 然后因为重新判定左右侧，导致同一面墙看起来从左侧墙跑变成右侧墙跑

新增参数：

- `WallJumpReattachBlockTime`

默认值：

- `0.25`

作用：

- 墙跳触发后，短时间内禁止重新吸附回刚刚那块墙跑组件
- 这样能避免墙跳动作被立刻打断，也能避免左右侧状态在同一面墙上来回翻转

如果你感觉墙跳后还是太容易被吸回原墙：

- 可以适当提高 `WallJumpReattachBlockTime`

如果你希望墙跳后能更快重新挂墙：

- 可以适当降低 `WallJumpReattachBlockTime`

## 12. 推荐的关卡制作方式

### 方式 A：直接给现有墙体挂组件

适合：

- 正式项目
- 已有很多墙体蓝图
- 希望任何墙都可以通过挂组件支持墙跑

做法：

1. 打开墙体蓝图
2. 添加 `WallRunSurfaceComponent`
3. 调整位置、旋转和范围

### 方式 B：使用包装 Actor

项目里仍保留了 `WallRunSurfaceActor` 作为可选包装器。
如果你更喜欢在关卡里直接摆一个独立对象，也可以继续用它。

## 13. 如果以后想升级成独立碰撞通道

当前默认是通用可运行方案：

- `WallRunSurfaceComponent` 使用 `ECC_WorldDynamic`
- `WallRunDetector` 对 `ECC_WorldDynamic` 做 `Overlap`

如果以后想更干净：

1. 在项目设置里新增专用碰撞通道，例如 `WallRunZone`
2. 把 `WallRunSurfaceComponent` 的 Object Type 改成这个通道
3. 把 `WallRunDetector` 只对这个通道做 `Overlap`

这样可以减少其他 `WorldDynamic` 组件误参与墙跑检测。
