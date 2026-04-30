# swaynav

[English](#english) | [中文](#chinese)

---

<a name="english"></a>
## English

Keyboard-driven mouse control for Wayland. A keynav clone for Wayland compositors.

> **Keywords**: mouseless, keyboard-only, pointer control, cursor navigation, RSI, ergonomics, accessibility, keynav alternative, warpd alternative, tiling wm tools, wayland mouse control, sway mouse, hyprland mouse, vim-style mouse

### Compatibility

Works on any Wayland compositor that supports `gtk-layer-shell`:
- **Sway** (full support)
- **Hyprland** (core features)
- **river** (core features)
- **labwc** (core features)
- **wayfire** (core features)

| Feature | Sway | Other Compositors |
|---------|------|-------------------|
| Grid navigation | ✓ | ✓ |
| Mouse click/drag | ✓ | ✓ |
| Multi-monitor | ✓ | ✓ |
| Custom keybindings | ✓ | ✓ |
| Macro recording | ✓ | ✓ |
| `windowzoom` | ✓ | ✗ |
| `cursorzoom` | ✓ | ✗ |

### Features

- Vim-style keyboard navigation for mouse pointer
- Grid-based cursor positioning
- Multi-monitor support (GDK-based, universal)
- Customizable keybindings
- Macro recording and playback
- Undo/redo for grid operations

### Requirements

- Wayland compositor with gtk-layer-shell support
- Dependencies: `libgtk-3-dev`, `libgtk-layer-shell-dev`, `libjson-c-dev`
- Runtime: `ydotool` (for mouse control)

### Installation

```bash
# Install dependencies (Debian/Ubuntu)
sudo apt install libgtk-3-dev libgtk-layer-shell-dev libjson-c-dev ydotool

# Build
make

# System-wide install
sudo make install

# Or user install
./install.sh
```

### Usage

1. Start the ydotoold daemon:
```bash
sudo ydotoold
```

2. Configure keybinding in your compositor:

**Sway** (`~/.config/sway/config`):
```
bindsym ctrl+semicolon exec swaynav toggle
```

**Hyprland** (`~/.config/hypr/hyprland.conf`):
```
bind = CTRL, semicolon, exec, swaynav toggle
```

**river** (`~/.config/river/init`):
```
riverctl map normal Ctrl Semicolon spawn "swaynav toggle"
```

3. Press `Ctrl+;` to activate swaynav

### Autostart

**Option 1: Compositor config** (recommended for compositor-specific setups)

Add to your compositor config:

**Sway**: `exec swaynav`

**Hyprland**: `exec-once = swaynav`

**river**: `riverctl spawn "swaynav"`

**Option 2: systemd** (universal, works with any compositor)

```bash
# Install ydotoold service (required dependency)
sudo cp data/ydotoold.service /etc/systemd/system/
sudo systemctl enable --now ydotoold

# Setup swaynav user service
mkdir -p ~/.config/systemd/user/
cp data/swaynav.service ~/.config/systemd/user/
systemctl --user enable --now swaynav
```

### Default Keybindings

| Key | Action |
|-----|--------|
| `h/j/k/l` | Cut selection (shrink) |
| `Shift+h/j/k/l` | Move selection |
| `y/u/b/n` | Cut diagonally |
| `Space/Enter` | Warp cursor and click |
| `w` | Warp cursor only |
| `Escape` | End |
| `t` | Zoom to window (Sway only) |
| `a` | Undo |
| `q` | Record macro |
| `Shift+@` | Playback macro |

### Configuration

Copy example config:
```bash
mkdir -p ~/.config/swaynav
cp data/config.example ~/.config/swaynav/config
```

Config file: `~/.config/swaynav/config`

### Custom Colors

Colors are defined in `src/overlay.c`. To customize, edit these macros and rebuild:

```c
/* Example: change to your preferred color */
#define GRUVBOX_BLUE_R 0.27  /* Red component (0.0-1.0) */
#define GRUVBOX_BLUE_G 0.53  /* Green component */
#define GRUVBOX_BLUE_B 0.53  /* Blue component */
```

Popular color presets:

| Theme | RGB Values | Hex |
|-------|------------|-----|
| Gruvbox Blue | 0.27, 0.53, 0.53 | #458588 |
| Gruvbox Green | 0.72, 0.73, 0.15 | #b8bb26 |
| Gruvbox Orange | 0.84, 0.37, 0.06 | #d65d0e |
| Catppuccin Blue | 0.51, 0.51, 0.67 | #89b4fa |
| Nord Blue | 0.44, 0.55, 0.68 | #81a1c1 |

After editing:
```bash
make && cp bin/swaynav ~/.local/bin/swaynav
```

### Commands

CLI commands:
- `swaynav` - Start daemon
- `swaynav start` - Show overlay
- `swaynav end` - Hide overlay
- `swaynav toggle` - Toggle overlay
- `swaynav quit` - Quit daemon

Config commands: `cut-*`, `move-*`, `warp`, `click N`, `drag`, `windowzoom` (Sway), `cursorzoom` (Sway), `history-back`, `record`, `playback`, `sh`

### License

MIT License

---

<a name="chinese"></a>
## 中文

适用于 Wayland 的键盘驱动鼠标控制工具。keynav 的 Wayland 版本。

> **关键词**: 无鼠标操作, 键盘控制鼠标, 光标导航, 辅助功能, 无障碍, RSI, 腱鞘炎预防, keynav替代, sway鼠标控制, hyprland鼠标控制, vim风格鼠标, 网格定位鼠标

### 兼容性

支持任何实现了 `gtk-layer-shell` 的 Wayland 合成器：
- **Sway** (完整支持)
- **Hyprland** (核心功能)
- **river** (核心功能)
- **labwc** (核心功能)
- **wayfire** (核心功能)

| 功能 | Sway | 其他合成器 |
|------|------|-----------|
| 网格导航 | ✓ | ✓ |
| 鼠标点击/拖拽 | ✓ | ✓ |
| 多显示器 | ✓ | ✓ |
| 自定义快捷键 | ✓ | ✓ |
| 宏录制 | ✓ | ✓ |
| `windowzoom` | ✓ | ✗ |
| `cursorzoom` | ✓ | ✗ |

### 功能特性

- Vim 风格的键盘导航鼠标指针
- 基于网格的精确定位
- 多显示器支持 (基于 GDK，通用)
- 可自定义快捷键
- 宏录制和回放
- 撤销/重做操作

### 依赖

- 支持 gtk-layer-shell 的 Wayland 合成器
- 编译依赖: `libgtk-3-dev`, `libgtk-layer-shell-dev`, `libjson-c-dev`
- 运行时依赖: `ydotool` (用于鼠标控制)

### 安装

```bash
# 安装依赖 (Debian/Ubuntu)
sudo apt install libgtk-3-dev libgtk-layer-shell-dev libjson-c-dev ydotool

# 编译
make

# 系统安装
sudo make install

# 或用户安装
./install.sh
```

### 使用方法

1. 启动 ydotoold 守护进程:
```bash
sudo ydotoold
```

2. 在合成器中配置快捷键:

**Sway** (`~/.config/sway/config`):
```
bindsym ctrl+semicolon exec swaynav toggle
```

**Hyprland** (`~/.config/hypr/hyprland.conf`):
```
bind = CTRL, semicolon, exec, swaynav toggle
```

**river** (`~/.config/river/init`):
```
riverctl map normal Ctrl Semicolon spawn "swaynav toggle"
```

3. 按 `Ctrl+;` 激活 swaynav

### 开机自启动

**方式一：合成器配置** (推荐，针对特定合成器)

添加到合成器配置文件：

**Sway**: `exec swaynav`

**Hyprland**: `exec-once = swaynav`

**river**: `riverctl spawn "swaynav"`

**方式二：systemd** (通用，适用于任何合成器)

```bash
# 安装 ydotoold 服务 (必需依赖)
sudo cp data/ydotoold.service /etc/systemd/system/
sudo systemctl enable --now ydotoold

# 配置 swaynav 用户服务
mkdir -p ~/.config/systemd/user/
cp data/swaynav.service ~/.config/systemd/user/
systemctl --user enable --now swaynav
```

### 默认快捷键

| 按键 | 功能 |
|-----|------|
| `h/j/k/l` | 切割选区 (缩小) |
| `Shift+h/j/k/l` | 移动选区 |
| `y/u/b/n` | 对角切割 |
| `空格/回车` | 移动光标并点击 |
| `w` | 仅移动光标 |
| `Esc` | 结束 |
| `t` | 缩放到窗口 (仅 Sway) |
| `a` | 撤销 |
| `q` | 录制宏 |
| `Shift+@` | 回放宏 |

### 配置

复制示例配置:
```bash
mkdir -p ~/.config/swaynav
cp data/config.example ~/.config/swaynav/config
```

配置文件: `~/.config/swaynav/config`

### 自定义颜色

颜色定义在 `src/overlay.c` 中。自定义颜色需修改以下宏并重新编译：

```c
/* 示例：更改为你喜欢的颜色 */
#define GRUVBOX_BLUE_R 0.27  /* 红色分量 (0.0-1.0) */
#define GRUVBOX_BLUE_G 0.53  /* 绿色分量 */
#define GRUVBOX_BLUE_B 0.53  /* 蓝色分量 */
```

常用颜色预设：

| 主题 | RGB 值 | Hex |
|------|--------|-----|
| Gruvbox 蓝 | 0.27, 0.53, 0.53 | #458588 |
| Gruvbox 绿 | 0.72, 0.73, 0.15 | #b8bb26 |
| Gruvbox 橙 | 0.84, 0.37, 0.06 | #d65d0e |
| Catppuccin 蓝 | 0.51, 0.51, 0.67 | #89b4fa |
| Nord 蓝 | 0.44, 0.55, 0.68 | #81a1c1 |

修改后重新编译：
```bash
make && cp bin/swaynav ~/.local/bin/swaynav
```

### 命令

CLI 命令:
- `swaynav` - 启动守护进程
- `swaynav start` - 显示覆盖层
- `swaynav end` - 隐藏覆盖层
- `swaynav toggle` - 切换覆盖层
- `swaynav quit` - 退出守护进程

配置命令: `cut-*`, `move-*`, `warp`, `click N`, `drag`, `windowzoom` (仅 Sway), `cursorzoom` (仅 Sway), `history-back`, `record`, `playback`, `sh`

### 许可证

MIT License