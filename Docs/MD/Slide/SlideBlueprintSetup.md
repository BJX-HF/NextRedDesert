# DmcCameraCharacter 滑铲蓝图接入说明

## 1. 输入资源配置

你需要在 Unreal 编辑器里补一个滑铲输入资源，并把它绑定到角色。

### 1.1 创建 Input Action

- 在 `Content/Input/Actions/` 下创建一个新的 `Input Action`
- 建议命名为 `IA_Slide`
- Value Type 选择 `Boolean`

### 1.2 在 Mapping Context 中绑定按键

- 打开 `Content/Input/IMC_Default`
- 新增一条 `IA_Slide`
- 给它绑定你想要的滑铲按键
- 以后如果你想改按键，直接改这里即可，不需要改 C++

### 1.3 在角色蓝图中指定 SlideAction

- 打开 `Content/Characters/Blueprint/MainCharacter/BP_DMCameraCharacter`
- 选中角色蓝图根对象
- 在 Details 面板找到 `Input`
- 将 `SlideAction` 指向你刚创建的 `IA_Slide`

## 2. 角色蓝图中可调的滑铲参数

打开 `BP_DMCameraCharacter` 后，选中角色蓝图根对象，在 Details 面板中找到 `Movement|Slide` 分类。

### 2.1 触发相关

- `bSlideEnabled`
  - 是否启用滑铲功能
- `bCanSlideFromStandingStill`
  - 是否允许静止时触发滑铲
  - 如果关闭，则必须达到速度门槛后才能触发
- `SlideTriggerMinSpeed`
  - 按下滑铲按键后，允许进入滑铲的最低水平速度
  - 这是你要求的“达到一定速度才可触发”的核心参数
- `SlideCooldown`
  - 两次滑铲之间的冷却时间

### 2.2 位移和速度相关

- `SlideMinStartSpeed`
  - 滑铲开始时，最低会被修正到的起始速度
- `SlideInitialSpeedBoost`
  - 进入滑铲瞬间附加的速度增量
- `SlideMaxSpeed`
  - 滑铲期间允许达到的最大速度
- `SlideAcceleration`
  - 滑铲期间持续向前推进时使用的加速度
- `SlideBrakingDeceleration`
  - 滑铲期间的制动减速度
- `SlideGroundFriction`
  - 滑铲期间地面摩擦
- `SlideDuration`
  - 滑铲持续时间
- `SlideMinSpeedToEnd`
  - 当滑铲速度低于该值时会结束滑铲
- `SlideSteeringControl`
  - 滑铲期间可保留多少转向控制
  - 越小越“直冲”，越大越容易修正方向

### 2.3 胶囊体高度相关

- `SlideCapsuleHalfHeight`
  - 滑铲期间角色胶囊体的半高
  - 用于穿过低矮空间
  - 如果设置太小，虽然更容易穿过低通道，但可能更容易出现角色与模型表现不协调的问题

## 3. 动画蓝图需要接入的内容

你的要求是把滑铲做进基础动画蓝图状态机，而不是蒙太奇。当前 C++ 已经提供了适合状态机读取的变量。

你需要打开：

- `Content/Characters/Blueprint/MainCharacter/ABP_DMCameraCharacter`

### 3.1 在动画蓝图中读取角色状态

在 `Event Blueprint Update Animation` 中：

- 获取 `Try Get Pawn Owner`
- 转成 `BP_DMCameraCharacter` 或对应的角色类
- 读取以下变量并缓存到动画蓝图变量中

建议至少缓存：

- `bIsSliding`
- `HorizontalSpeed`

如果你还想做更细的朝向或姿态控制，也可以读取：

- `SlideDirection`

### 3.2 在状态机中新增 Slide 状态

在你的基础移动状态机里新增一个 `Slide` 状态。

该状态中放入：

- 你的滑铲动画序列
- 或者未来扩展成滑铲 BlendSpace

### 3.3 状态切换条件

建议这样设置：

- 从 Locomotion 或 Run 状态进入 `Slide`
  - 条件：`bIsSliding == true`
- 从 `Slide` 返回 Locomotion
  - 条件：`bIsSliding == false`

### 3.4 动画表现建议

如果你的滑铲动画本身带前进位移：

- 建议先确认是否使用 Root Motion
- 当前 C++ 位移已经负责滑铲推进
- 如果动画也带明显 Root Motion，可能会和 C++ 位移叠加

如果你暂时不打算使用 Root Motion：

- 让动画只负责表现
- 由当前 C++ 滑铲逻辑负责真实移动

## 4. 当前已经由 C++ 处理好的部分

这些内容你不需要再额外在蓝图里实现逻辑：

- 绑定 `SlideAction` 后按键可触发滑铲
- 滑铲必须满足速度条件才能触发
- 滑铲期间角色会降低胶囊体高度
- 滑铲结束后会尝试恢复站立
- 如果头顶空间不足，会继续保持低姿态，直到可以站起
- 滑铲期间会限制跳跃
- 滑铲期间会保留一定程度的转向能力
- 滑铲期间会朝滑行方向转身

## 5. 你在编辑器里最少需要完成的事项

如果只想先跑通功能，最少做这几步：

1. 创建 `IA_Slide`
2. 在 `IMC_Default` 中为 `IA_Slide` 绑定按键
3. 在 `BP_DMCameraCharacter` 中把 `SlideAction` 指向 `IA_Slide`
4. 在 `BP_DMCameraCharacter` 中调整 `Movement|Slide` 下的各项参数
5. 在 `ABP_DMCameraCharacter` 的状态机中新增 `Slide` 状态，并用 `bIsSliding` 作为切换条件

## 6. 推荐的第一版调参起点

如果你暂时没有明确数值，可以先从下面开始试：

- `SlideTriggerMinSpeed = 350`
- `SlideMinStartSpeed = 350`
- `SlideInitialSpeedBoost = 150`
- `SlideMaxSpeed = 900`
- `SlideAcceleration = 1000`
- `SlideBrakingDeceleration = 500`
- `SlideGroundFriction = 0.5`
- `SlideDuration = 0.8`
- `SlideMinSpeedToEnd = 180`
- `SlideSteeringControl = 0.25`
- `SlideCooldown = 0.25`
- `SlideCapsuleHalfHeight = 48`

## 7. 文件位置

本次滑铲相关 C++ 代码主要在：

- `Source/AITestProject/Character/New/DmcCameraCharacter.h`
- `Source/AITestProject/Character/New/DmcCameraCharacter.cpp`
