# 智能果蔬冷链溯源系统 (Smart Cold-Chain Traceability System)

这是一个基于 **C++ 高并发 Reactor 网络框架** 与 **Qt6 / QML** 打造的现代化物联网物流追踪系统。本项目模拟了商用冷链物流场景中，前端设备数据采集、后端高并发接入与转发、以及管理端实时大屏监控的完整闭环。非常适合作为 C++ 软件开发工程师的面试展示项目。

---

## 🌟 一、 系统架构设计

本系统采用经典的三端解耦架构：

```text
[A_client_app (Qt6 移动端)]    [RelayServer (C++ Reactor)]    [A_manger_app (Qt6 桌面大屏)]
        (数据生产者)                    (高并发消息枢纽)                  (数据消费者)
             |                               |                                |
  本地 SQLite 维护任务列表       ---->   TCP 长连接高频接收    ---->   TCP 长连接实时推送 JSON
  定时生成 GPS 与温度数据                多路复用 (Epoll)              解析 JSON，驱动高德地图更新
  5%概率触发温度异常(故障)               广播至所有 Manager 端         多页面联动更新订单状态
             |                               |                                |
  (QTcpSocket / QJsonDocument)         (Epoll / ThreadPool)         (QtWebEngine / JS API / QML)
```

1. **司机采集端 (`A_client_app`)**：使用 Qt Quick (QML) 编写。内置 SQLite 数据库管理订单，通过定时器与随机数算法生成高频的 GPS 轨迹与车厢温度数据（支持故障模拟注入），并通过 TCP 发送给后端。
2. **高并发中转枢纽 (`RelayServer`)**：基于 C++11 实现的 `One Loop Per Thread` 模式的 Reactor 网络框架。负责海量设备的 TCP 长连接维持，将接收到的物联网 JSON 数据广播分发给所有在线的管理员。**（已支持线程安全的 MySQL 连接池与心跳保活断线检测机制）**
3. **全局监控大屏 (`A_manger_app`)**：使用 QtWebEngine 混合开发，内嵌高德地图 Web API。实时解析后端推送的 JSON 数据，动态绘制多辆冷藏车的平滑移动轨迹，并提供全局订单数据大盘**与 ECharts 温度异常告警系统**。

---

## 🌟 核心商业化功能亮点

*   **📦 容器化部署**：提供 `Dockerfile` 和 `docker-compose.yml`，一键拉起 C++ 服务端与 MySQL 数据库。
*   **🔌 数据库连接池**：基于 `std::queue` 与互斥锁实现的自定义 MySQL 连接池，大幅提升高并发下的数据库持久化吞吐量。
*   **💗 心跳保活机制**：服务端支持 `ping/pong` 心跳协议，结合 `timerfd` 定时器，自动清理超过 80 秒无响应的死连接。
*   **📶 断网本地缓存与续传**：司机端在无网络环境下，会将位置与温度数据存入本地 SQLite，待网络恢复后自动将堆积的数据打包重传。
*   **🤖 AI 降级容灾策略**：当智谱大模型 API 调用失败或超时，自动降级为本地算法生成合理的波动温度，保障系统不崩溃。
*   **📊 数据可视化与告警**：管理员大屏采用赛博朋克暗色 UI 设计，结合 ECharts 绘制订单历史温度折线图，并在出现异常数据时全局红屏弹窗告警。

---

## 🛠️ 二、 开发与运行环境要求

### 1. 后端环境 (RelayServer)
* **操作系统**：Linux (推荐 Ubuntu 20.04/22.04) 或 Windows 下的 WSL/Git Bash。
* **编译器**：GCC/G++ 4.8 及以上版本（需完全支持 C++11 标准）。
* **数据库**：MySQL 5.7+ (需安装 `libmysqlclient-dev`)。
* **构建工具**：Make。

### 2. 客户端/管理端环境 (Qt Apps)
* **开发工具**：Qt Creator 10.0+ 或 Visual Studio Code (配合 CMake 插件)。
* **Qt 版本**：**Qt 6.2 及以上版本**（推荐 Qt 6.5 LTS）。
* **必备组件**：安装 Qt 时必须勾选以下模块：
  * `Qt Quick` / `Qt QML`
  * `Qt Network`
  * `Qt Sql`
  * `Qt WebEngine` (仅管理端需要)
  * `Qt Positioning` (仅司机端需要，用于获取真实硬件 GPS)

---

## 🚀 三、 服务端与数据库启动教程

本项目提供了**两种**启动方式：推荐使用 Docker 一键启动，或者手动编译启动。

### 方式 A：Docker 容器化一键启动 (推荐)
通过 Docker 和 docker-compose，无需配置 C++ 和 MySQL 环境，一键拉起所有后端服务：
1. 确保电脑已安装 [Docker Desktop](https://www.docker.com/products/docker-desktop/)。
2. 在项目根目录下，执行一键拉起命令：
   ```bash
   docker-compose up -d --build
   ```
3. 等待数据库初始化完成后（约 10 秒），后端服务器将在 `0.0.0.0:8888` 上提供服务。
4. 可以通过 `docker-compose logs -f relay_server` 查看实时日志。

### 方式 B：手动编译配置启动

#### 步骤 1：准备 MySQL 数据库
本项目使用 MySQL 进行数据的持久化存储，启动前请务必配置好数据库。
1. 在 Linux/WSL 中安装 MySQL：
   ```bash
   sudo apt update
   sudo apt install mysql-server libmysqlclient-dev
   ```
2. 登录 MySQL 并创建业务数据库：
   ```sql
   CREATE DATABASE cold_chain_db DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
   ```
3. **注意**：程序默认使用 `root` 用户和 `123456` 密码连接。如果您的本地配置不同，请在 `relay_main.cpp` 中修改 `DatabaseHelper::getInstance().connect()` 的参数。程序启动时会自动创建 `orders` 和 `transport_logs` 表。

### 步骤 2：启动 Reactor 中转服务器
1. 打开 Linux/WSL 终端，进入项目根目录。
2. 执行编译命令：
   ```bash
   make relay_server
   ```
3. 启动服务器，指定监听的 IP 和端口：
   ```bash
   ./relay_server 127.0.0.1 8888
   ```
   *启动成功后，终端会打印出 MySQL 连接成功以及表结构初始化的日志。*

### 步骤 3：启动 管理员监控大屏 (`A_manger_app`)
1. 使用 Qt Creator 打开 `A_manger_app` 目录下的 `CMakeLists.txt`。
2. 配置构建套件：**必须选择 Desktop 版套件**（如 `Desktop Qt 6.x.x MinGW 64-bit` 或 `MSVC`）。*注意：包含 WebEngine 的项目无法直接编译为移动端。*
3. 点击左下角的绿色 **Run (运行)** 按钮。
4. 程序启动后，会自动尝试连接 `127.0.0.1:8888`，右上角状态会变为“已连接到服务器”。

### 步骤 4：启动 司机采集端 (`A_client_app`)
1. 使用 Qt Creator 打开 `A_client_app` 目录下的 `CMakeLists.txt`。
2. 配置构建套件：选择 Desktop 版套件进行本机调试。
3. **【重要】API 密钥配置**：本项目使用了智谱大模型(GLM-4)根据真实坐标生成天气与环境数据。请在 `A_client_app/LLMService.h` 中配置您的智谱 API Key。
4. 点击 **Run (运行)** 按钮。
5. **操作流程**：
   * 在登录页点击“登录”。
   * 在“创建新订单”页面，输入货物名称（如：烟台苹果）和目标温度，点击“生成订单并开始运输”。
   * 程序将调用系统定位模块获取真实 GPS 坐标，并每隔一分钟向大模型发送 Prompt 请求。
6. **见证奇迹**：
   * 司机端屏幕上将每分钟打印出带有真实天气、经纬度以及 AI 生成的波动的温度 JSON 数据。
   * 同时，这段数据会被发送到后端永久存入 MySQL。
   * **管理员大屏**的地图上会出现一辆小货车开始移动，并在“全网订单大盘”页面中，对应订单的状态会实时跳动更新！

---

## 📱 四、 如何将司机端 (`A_client_app`) 编译成 Android APK

司机端不包含 WebEngine 模块，完美支持编译为安卓 APP 在真机上运行。

### 1. 准备 Android 开发环境
在 Qt Creator 中编译 APK，你需要配置以下环境：
* **JDK (Java Development Kit)**: 推荐 JDK 11 或 17。
* **Android SDK**: 通过 Android Studio 的 SDK Manager 下载（推荐 Android 11/12 (API 30/31) 的 SDK Platform）。
* **Android NDK**: 推荐使用 NDK r22b 或 r23c。
* **Android Build Tools**: 推荐版本 31.0.0。

*配置路径：打开 Qt Creator -> `编辑 (Edit)` -> `首选项 (Preferences)` -> `设备 (Devices)` -> `Android`，将上述路径填入，确保所有打勾项均为绿色。*

### 2. Qt 安装 Android 扩展
如果你在安装 Qt 时没有勾选 Android 支持，需要打开 `Qt Maintenance Tool`，在你的 Qt 版本下勾选 **`Android` (如 Android ARM64-v8a)**。

### 3. 配置与编译步骤
1. 在 Qt Creator 中打开 `A_client_app/CMakeLists.txt`。
2. 点击左侧的 **`项目 (Projects)`** 按钮。
3. 在 `Build & Run` 中，启用你刚刚安装的 **`Android Qt 6.x.x Clang arm64-v8a`** 构建套件。
4. **修改服务器 IP**：
   * 打开 `A_client_app/DataSimulator.cpp`，找到 `m_tcpSocket->connectToHost("127.0.0.1", 8888);` 这一行。
   * **极其重要**：因为手机和电脑不在同一个 Localhost，你需要将 `127.0.0.1` 修改为你电脑（运行 RelayServer）的 **局域网 IP 地址**（例如 `192.168.1.100`）。
   * 确保手机和电脑连接在同一个 Wi-Fi 网络下。
5. 在左下角的构建套件选择器（电脑图标）中，切换到 Android 套件。
6. 点击 **`构建 (Build)`**（锤子图标）。编译成功后，在构建目录下会生成 `android-build/build/outputs/apk/debug/android-build-debug.apk`。

### 4. 真机调试 (USB 调试)
1. 拿出一台安卓手机，进入 `设置` -> `关于手机`，连续点击 `版本号` 7次开启开发者模式。
2. 进入 `开发者选项`，开启 **`USB 调试`**。
3. 用数据线将手机连接到电脑，允许电脑调试。
4. 在 Qt Creator 的设备列表中，应该能看到你的手机型号。
5. 直接点击 **`运行 (Run)`** 按钮，Qt 会自动将 APK 编译、打包、推送到你的手机上并启动。

---

## 🐞 五、 常见调试技巧 (Debugging)

### 1. 网络连接失败？
* **现象**：客户端或管理端显示“连接错误: Connection refused”。
* **排查**：
  1. 确认 Linux/WSL 中的 `relay_server` 是否正在运行（是否有因为端口被占用而闪退）。
  2. 如果手机端连接失败，请检查电脑的**防火墙设置**，确保入站规则放行了 TCP `8888` 端口。
  3. 检查手机端填写的 IP 是否是电脑正确的局域网 IPv4 地址。

### 2. 地图出不来 / 白屏？
* **现象**：管理端的右侧地图区域白屏，左侧面板正常。
* **排查**：
  1. 确认运行管理端的电脑是否连接了互联网（加载高德 JS API 需要外网）。
  2. 检查 `A_manger_app/map.html` 中的高德 Key 是否有效。如果控制台报错 `USER_KEY_RECYCLED`，请前往[高德开放平台](https://lbs.amap.com/)申请你自己的 Web 端 (JS API) Key 并替换。
  3. 检查 `CMakeLists.txt` 中是否正确包含了 `RESOURCES map.html`。

### 3. QML 界面布局错乱？
* 如果在不同分辨率的屏幕上运行，可以通过调整 `Layout.preferredWidth` 或使用 `anchors.centerIn` 来适应屏幕。目前项目采用的是弹性布局 (`RowLayout`/`ColumnLayout`)，具备良好的自适应能力。

---
*Developed by Qt & C++ Reactor. 2026.*