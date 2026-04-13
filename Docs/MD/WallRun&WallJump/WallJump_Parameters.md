# UE5 WallJump 参数与说明

## 1. 入口开关

### `bEnableWallJump`

- 类型：`bool`
- 默认值：`true`
- 分类：`Movement|WallRun|WallJump`
- 作用：是否允许墙跑中按 Jump 触发 WallJump

说明：

- 关闭时，墙跑期间按 Jump 不会触发 WallJump
- 打开时，只有在墙跑实际生效期间，Jump 才会触发 WallJump

## 2. 力度参数

### `WallJumpHorizontalStrength`

- 类型：`float`
- 默认值：`520`
- 分类：`Movement|WallRun|WallJump`
- 作用：沿墙法线反推的水平力度

建议：

- 想让角色更明显地离墙弹开，就调大
- 如果弹得太远，调小

### `WallJumpVerticalStrength`

- 类型：`float`
- 默认值：`650`
- 分类：`Movement|WallRun|WallJump`
- 作用：向上的起跳力度

建议：

- 增大后更像强力蹬墙跳
- 过大可能会让角色跳得太高

### `WallJumpForwardBoost`

- 类型：`float`
- 默认值：`140`
- 分类：`Movement|WallRun|WallJump`
- 作用：额外沿角色正方向给一段推进

建议：

- 如果希望动作更有“扑出去”的感觉，可以适当调大
- 如果只想强调离墙反弹，调小

## 3. 动画辅助参数

### `WallJumpAnimFlagDuration`

- 类型：`float`
- 默认值：`0.2`
- 分类：`Movement|WallRun|WallJump`
- 作用：`bWallJumpTriggered` 保持为真的时长

说明：

- 这个参数主要用于 AnimBP 捕捉墙跳状态
- 如果动画机容易漏判墙跳，可以适当调大
- 如果状态停留太久，调小

## 4. 重新吸附保护

### `WallJumpReattachBlockTime`

- 类型：`float`
- 默认值：`0.25`
- 分类：`Movement|WallRun|WallJump`
- 作用：墙跳后禁止重新吸附回刚刚那块墙跑组件的保护时间

说明：

- 这是为了避免墙跳刚触发，角色立刻又被吸回原墙
- 也能减少同一面墙上的左右侧状态翻转

建议：

- 如果仍然容易被吸回原墙，调大
- 如果希望墙跳后更快能重新挂回墙，调小

## 5. 当前 WallJump 的生效条件

当前代码中，WallJump 只有在 `CanPerformWallJump()` 为真时才会触发。

核心条件包括：

- `bEnableWallJump == true`
- 当前角色正在墙跑
- 当前移动模式是 `CMOVE_WallRun`
- 当前有有效墙跑 Surface
- 当前墙法线有效
- 当前沿墙方向有效

这意味着：

- 地面 Jump 仍然是普通跳跃
- Falling 状态但未进入墙跑时，不会触发 WallJump
- 只有墙跑期间再次按 Jump，才会执行蹬墙跳

## 6. 当前动画相关接口

WallJump 相关接口：

- `IsWallJumping()`
- `WasWallJumpRequestedThisFrame()`
- `GetWallJumpSide()`
- `IsWallJumpFromLeft()`
- `IsWallJumpFromRight()`
- `CanPerformWallJump()`

这些接口都可以直接在蓝图中读取。

## 7. 推荐调参起点

如果你想先得到一个比较稳定的第三人称动作感，可以从这里开始：

- `bEnableWallJump = true`
- `WallJumpHorizontalStrength = 520`
- `WallJumpVerticalStrength = 650`
- `WallJumpForwardBoost = 140`
- `WallJumpAnimFlagDuration = 0.2`
- `WallJumpReattachBlockTime = 0.25`

然后按你的手感目标再微调：

- 更偏平台动作：提高 `VerticalStrength`
- 更偏忍者横向蹬墙：提高 `HorizontalStrength`
- 更偏前扑冲刺：提高 `ForwardBoost`
