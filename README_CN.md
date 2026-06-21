# OE_Engine — 基于 OpenGL 的实时渲染引擎

<p align="center">
  <img src="Porsche_911_Turbo_PBR_IBL_1.png" alt="Porsche 911 Turbo — PBR + IBL 渲染效果" width="100%"/>
</p>

> 一个使用 **C++17 / OpenGL 4.6** 从零搭建的实时渲染引擎，支持 **PBR（基于物理的渲染）** 与 **Blinn-Phong** 双渲染管线，并提供 **延迟渲染（Deferred Rendering）** 与 **前向渲染（Forward Rendering）** 混合渲染路径。

---

## 📖 项目简介

**OE_Engine** 是一个在学习图形学过程中，基于 **[LearnOpenGL](https://learnopengl.com/)** 教程从零搭建的轻量级 OpenGL 实时渲染引擎。项目旨在将教程中的知识点整合为一个完整的渲染引擎框架，并在此基础上实践更多的现代渲染技术。

项目包含两个主要部分：

| 模块 | 说明 |
|------|------|
| **OE_Engine** | 引擎核心，编译为静态库（`.lib`），封装了渲染管线、光照系统、阴影、模型加载等功能 |
| **Simple_Render_Application** | 基于引擎的渲染示例应用，包含多个渲染实验场景 |

> [!WARNING]
> 当前为**初代版本**，仍存在许多已知问题和待优化之处，将在后续持续迭代改进。

> [!CAUTION]
> **macOS 不受支持**：本项目重度使用了 OpenGL 4.5 的 DSA (Direct State Access) 特性。由于 Apple 已停止更新 OpenGL 且最高仅支持至 OpenGL 4.1，该项目在 macOS 下虽能编译，但会在运行时因调用缺失 API 而崩溃。请在 Windows (或 Linux) 环境下运行本项目。

---

## ✨ 渲染特性

### 🎨 PBR 渲染管线（Cook-Torrance Metallic-Roughness Workflow）

- **金属度-粗糙度工作流**
  - 支持 Albedo、Metallic、Roughness、AO、Normal、Emissive 六张贴图
  - 支持独立纹理通道与打包 MRA 纹理（Metallic=B / Roughness=G / AO=R）
  - 各贴图支持 Factor 因子控制（`albedoFactor`, `metallicFactor`, `roughnessFactor`）
- **Cook-Torrance BRDF 反射模型**
  - **法线分布函数（NDF）**：GGX / Trowbridge-Reitz
  - **几何遮蔽函数（Geometry）**：Smith's Schlick-GGX（`k = (roughness+1)²/8`）
  - **菲涅尔方程（Fresnel）**：Fresnel-Schlick 近似 + 粗糙度修正版本（用于 IBL）
- **基于图像的光照（IBL — Image Based Lighting）**
  - 漫反射辐照度图（Irradiance Map，DDS 格式，通过 GLI 加载）
  - 镜面反射预滤波环境贴图（Pre-filtered Radiance Map，多级 Mipmap，LOD = roughness × 7.0）
  - BRDF 积分查找表（BRDF LUT，RGB16F）
  - 基于 Split-Sum 近似的环境光计算
- **折射 / 透射（Transmission）**
  - 支持 glTF `KHR_materials_transmission` 扩展
  - 基于屏幕空间的折射效果（IOR 控制，默认 1.5）
  - 全内反射（TIR）检测
  - 粗糙度衰减的背景采样
- **Alpha Cutoff / Masked 渲染**：`cutOff` 阈值控制的 Alpha 遮罩剔除

### 💡 Blinn-Phong 渲染管线

> [!CAUTION]
> Blinn-Phong 管线目前处于**半弃用状态**，可能无法正常工作。建议使用 **PBR 管线**进行渲染。

- **Blinn-Phong 光照模型**（Ambient + Diffuse + Specular）
- 支持法线贴图（TBN 切线空间变换）
- 延迟渲染 + 前向渲染双路径
- Gamma 校正（输入线性化 `pow(color, 2.2)`）

### 🔦 光源系统

| 光源类型 | 最大数量 | 说明 |
|----------|----------|------|
| **方向光（Directional Light）** | 1 | 支持开关控制，含颜色配置与阴影投射 |
| **点光源（Point Light）** | 4 | 支持距离衰减（`1/(c + l·d + q·d²)`） |
| **聚光灯（Spot Light）** | 4 | 支持内外锥角、距离衰减、方向控制 |
| **IBL 环境光** | 1 | 辐照度 + 预滤波环境贴图 + BRDF LUT |

### 🌑 阴影系统

- **方向光阴影贴图**（Directional Shadow Map）
  - 正交投影阴影捕获
  - PCF（Percentage Closer Filtering）3×3 核软阴影
  - 自适应斜率偏移（`max(0.005 × (1 - dot(n, l)), 0.0005)`）
  - 阴影贴图边界钳制（`GL_CLAMP_TO_BORDER`）
  - 可配置阴影分辨率（默认 1024×1024）
- **点光源全方向阴影贴图**（Omnidirectional Shadow Map）
  - 基于 Cube Map 的深度渲染
  - 几何着色器单 Pass 渲染 6 个面（`gl_Layer`）
  - 线性深度存储（`length(lightPos - fragPos) / farPlane`）
  - 20 方向采样的 PCF 软阴影
  - 延迟重捕获：仅当光源位置变化时重新生成阴影贴图

### 🌫️ 屏幕空间环境光遮蔽（SSAO）

- 64 个半球随机采样核心（加速分布，靠近表面密集采样）
- 4×4 随机旋转噪声纹理（`GL_RGB16F`）
- 基于噪声的 TBN 旋转 + Gram-Schmidt 正交化
- `smoothstep` 距离范围检测（防止远处几何体贡献）
- 4×4 Box Blur 降噪
- 仅影响环境光分量（`ambient × ao × ssao`）

### 🔀 混合渲染架构（Deferred + Forward）

引擎采用 **混合延迟/前向渲染** 架构：

| 渲染路径 | 适用对象 | 说明 |
|----------|----------|------|
| **延迟渲染** | Opaque / Masked 物体 | G-Buffer 多目标渲染（MRT） |
| **前向渲染** | Transparent / TransparentMasked 物体 | 按距离排序的半透明渲染 |

**G-Buffer 布局**（6 个颜色附件，`GL_RGB16F` / `GL_RGBA16F`）：

| 附件 | 内容 |
|------|------|
| 0 | 世界空间位置 |
| 1 | 光空间位置（阴影计算用） |
| 2 | 法线向量（`n×0.5+0.5` 编码） |
| 3 | Albedo（RGB）+ Alpha |
| 4 | Metallic(R) / Roughness(G) / AO(B) |
| 5 | Emissive（RGB） |

**渲染流程**：
1. **G-Buffer Pass** → 渲染不透明 / 遮罩物体到 G-Buffer
2. **SSAO Pass** → 从位置 + 法线 G-Buffer 计算 SSAO
3. **SSAO Blur Pass** → 4×4 Box Blur 平滑
4. **延迟光照 Pass** → 全屏四边形读取 G-Buffer + SSAO + 阴影贴图，计算完整 PBR/Phong 光照
5. **深度缓冲 Blit** → `glBlitFramebuffer` 拷贝深度信息
6. **天空盒渲染**
7. **前向渲染 Pass** → 半透明物体按距离由远到近排序渲染（开启混合，关闭深度写入）

### 🌌 天空盒（SkyBox）

- 6 面立方体贴图加载（`px/nx/py/ny/pz/nz`）
- 支持标准图片格式与 **HDR 格式**（`.hdr`，`GL_RGB16F`）
- 无限远渲染技巧（`gl_Position = pos.xyww` + `GL_LEQUAL`）
- 视图矩阵去平移（`mat4(mat3(view))`）

### 🖼️ 后处理与渲染管线

- **HDR 渲染**：所有渲染目标使用 16 位浮点格式（`GL_RGB16F` / `GL_RGBA16F`）
- **Reinhard 色调映射**（HDR → LDR）
- **Gamma 校正**（输入纹理线性化 `pow(color, 2.2)`）
- **帧缓冲对象（FBO）** 离屏渲染
- **多颜色附件（MRT）** G-Buffer 支持

---

## 🏗️ 引擎架构

### 核心模块

```
OE_Engine/src/
├── Renderer              # 渲染器 — 窗口创建、视口管理、混合/深度/模板测试控制
├── Camera                # 相机系统 — 透视/正交投影，互斥锁保护
├── CameraController      # 相机控制器 — 键盘 WASD 移动 + 鼠标旋转，可配置按键映射
├── Shader                # Shader 管理 — 编译/链接/Uniform 缓存，支持 VS/FS/GS
├── Model                 # 模型加载 — 基于 Assimp，支持 glTF/OBJ 等格式
├── Object                # 渲染对象 — 几何体 + 材质 + 变换，6 属性顶点布局
├── Material              # 材质系统 — Opaque/Masked/Translucent 三种混合模式
├── Texture               # 纹理管理 — 引用计数 + 缓存，支持 2D/HDR/Cubemap/DDS
├── RenderTarget          # 帧缓冲（FBO）— 最多 6 个颜色附件 + 深度/模板附件
├── PBRPipeline           # PBR 渲染管线（延迟+前向混合）
├── PhongLight            # Blinn-Phong 渲染管线（延迟+前向混合）
├── ShadowMapDirection    # 方向光阴影（正交投影 + PCF）
├── ShadowMapPoint        # 点光源阴影（CubeMap + 几何着色器）
├── SkyBox                # 天空盒（标准格式 + HDR 格式）
├── EngineConfig          # 引擎配置 — 管线选择、纹理槽位分配（26 个保留槽位）
└── Helper                # 工具函数 — MikkTSpace 切线计算、基础几何体生成
```

### 关键技术点

- **MikkTSpace 切线计算**：使用 `mikktspace` 库计算切线空间，确保法线贴图正确性
- **材质分类渲染**：自动将物体按 Opaque → Masked → Transparent 分类渲染
- **纹理缓存与引用计数**：避免重复加载 GPU 纹理，引用归零自动释放
- **Instanced Rendering**：支持 GPU 实例化渲染（小行星带演示：7777 个实例）
- **glTF PBR 材质全面支持**：Albedo/Metallic/Roughness/AO/Normal/Emissive/Transmission + 默认值回退
- **现代 OpenGL 特性**：使用 DSA（`glBindTextureUnit`, `glProgramUniform*`）等 OpenGL 4.5+ 特性
- **编译期管线切换**：通过 `#define` 在 PBR 与 Blinn-Phong 管线间切换

### 渲染示例场景

| 场景 | 管线 | 说明 |
|------|------|------|
| **PBRLighting** | PBR | 1975 Porsche 911 Turbo（glTF）+ 木地板 + IBL + 阴影 |
| **AdvancedLighting** | Blinn-Phong | 角色模型 + 方向光阴影 + HDR + 延迟渲染 |
| **InstanceExperiment** | Blinn-Phong | 行星 + 7777 个小行星 GPU 实例化渲染 |
| **RenderTargetExperiment** | Blinn-Phong | 背包模型 + 天空盒立方体贴图反射 + 后处理 |
| **StencilTestExperiment** | Blinn-Phong | 模板测试轮廓描边 + 深度可视化 |
| **DrawASimpleHouseUsingGS** | Blinn-Phong | 几何着色器演示（4 顶点 → 房屋） |

---

## 🛠️ 构建指南

### 环境要求

| 依赖 | 说明 |
|------|------|
| **CMake** | ≥ 3.15 |
| **C++ 编译器** | 支持 C++17（推荐 MSVC / GCC / Clang） |
| **vcpkg** | 包管理器，用于安装第三方依赖 |

### 第三方依赖

**通过 vcpkg 安装：**

| 库 | 用途 |
|----|------|
| **GLFW3** | 窗口管理与输入处理 |
| **GLM** | 数学库（向量、矩阵运算） |
| **stb** | 图像加载（stb_image） |
| **Dear ImGui** | 即时模式调试 UI |
| **Assimp** | 3D 模型导入（支持 glTF、OBJ、FBX 等格式） |
| **MikkTSpace** | 切线空间计算 |

**本地依赖（位于 `Dependences/` 目录）：**

| 库 | 用途 |
|----|------|
| **GLAD** | OpenGL 函数加载器 |
| **GLI** | OpenGL 纹理图像库（DDS 格式 Cubemap 加载） |

### 构建步骤

#### 1. 克隆项目

```bash
git clone <repository-url>
cd OE_Engine
```

#### 2. 安装 vcpkg（如尚未安装）

```bash
# 在项目的上级目录中安装 vcpkg（CMakeLists.txt 默认搜索 ../vcpkg/）
cd ..
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.bat   # Windows
# 或
./bootstrap-vcpkg.sh    # Linux / macOS
cd ../OE_Engine
```

#### 3. 使用 CMake 配置与构建

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目（CMake 会自动通过 vcpkg 工具链安装依赖）
cmake .. -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake"

# 构建
cmake --build . --config Release
```

#### 4. 使用 Visual Studio（Windows 推荐）

```bash
# 生成 Visual Studio 解决方案
cmake .. -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake"

# 打开 build/OE_Engine.sln
```

> [!TIP]
> 工作目录已在 CMake 中配置为 `Simple_Render_Application/` 目录，程序运行时会自动从该目录下的 `res/` 文件夹读取着色器、纹理和模型资源。使用 Visual Studio 调试时无需手动设置工作目录。

---

## 🖼️ 渲染效果展示

使用 PBR 渲染管线（延迟 + 前向混合）配合 IBL（基于图像的光照）渲染的 **1975 Porsche 911 Turbo** 模型：

<p align="center">
  <img src="Porsche_911_Turbo_PBR_IBL_1.png" alt="PBR + IBL 渲染效果" width="80%"/>
  <br/>
  <em>▲ PBR + IBL 渲染</em>
</p>

<p align="center">
  <img src="Porsche_911_Turbo_PBR_IBL_2.png" alt="PBR + IBL 渲染效果" width="80%"/>
  <br/>
  <em>▲ PBR + IBL 渲染</em>
</p>

---

## 🗺️ 后续计划

- [ ] 优化引擎结构与渲染性能，修复已知 Bug
- [ ] 引入抗锯齿（Anti-Aliasing）
- [ ] 换用 **Vulkan** 替代 OpenGL
- [ ] 光线追踪（Ray Tracing）（待定）

---

## 🙏 致谢

### 学习资源

- **[LearnOpenGL](https://learnopengl.com/)** — 本项目的主要学习参考教程

### 免费素材

示例场景中使用的免费资源，感谢以下作者和平台的慷慨分享：

| 资源 | 来源 |
|------|------|
| 🚗 Porsche 911 Turbo 车辆模型 | [Sketchfab — Free 1975 Porsche 911 (930) Turbo](https://sketchfab.com/3d-models/free-1975-porsche-911-930-turbo-8568d9d14a994b9cae59499f0dbed21e) |
| 🪵 木地板 PBR 材质 | [FreePBR — Bare Wood 1](https://freepbr.com/product/bare-wood1/) |
| 🌅 环境光 HDR 贴图 | [Poly Haven — Mirrored Hall](https://polyhaven.com/a/mirrored_hall) |
