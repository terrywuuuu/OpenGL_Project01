# OpenGL 機器人動畫與特效展示

以 C++、OpenGL 與 Dear ImGui 製作的互動式電腦圖學專案。程式載入由多個 OBJ 零件組成的機器人，提供動作播放、模型編輯、材質調整，以及場景與後處理效果控制。

## 功能

- 載入機器人身體、頭部、手臂、手掌、腿部與腳等分件模型，可分別調整關節。
- 播放待機、走路、仰臥起坐、伏地挺身、Hopak 舞蹈、阿帕茲與多重影分身等動作。
- 使用 ImGui 編輯動作影格與模型姿勢，動作資料以 JSON 儲存。
- 調整模型材質，切換實心/線框顯示，設定分身數量與排列模式。
- 控制水面、波浪與反射，並切換模糊、量化、馬賽克、Motion Blur、Environment Map 與 Toon Shader 等效果。

## 開發環境

- Windows 10 或更新版本
- Visual Studio 2022，安裝「使用 C++ 的桌面開發」工作負載
- Windows SDK 10.0
- 可支援 OpenGL 4.6 的顯示卡與驅動程式

專案已附上 GLFW、GLEW、GLM、Dear ImGui、nlohmann/json，以及音訊與影像載入所需的程式碼/函式庫。Visual Studio 專案目前的 `Debug | x64` 使用 v142 工具集，`Release | x64` 使用 v143；若電腦未安裝對應工具集，請在專案屬性中改成已安裝的 MSVC 工具集。

## 建置與執行

1. 使用 Visual Studio 開啟 `cg-gui.sln`。
2. 選擇 `Debug | x64` 或 `Release | x64`，並將 `cg-gui` 設為啟始專案。
3. 建置方案後按 F5 執行，或從 `build/Debugx64/`、`build/Releasex64/` 執行 `cg-gui.exe`。

程式以 `build/<組態>/` 作為工作目錄，並從相對路徑 `../../res/` 載入模型、動作 JSON、音樂與 shader。執行時請保留對應目錄中的 `res/` 資料夾及所需 DLL；不要只複製執行檔到其他位置啟動。

## 專案結構

```text
cg-gui/       Visual Studio 專案與程式進入點
src/          應用程式、場景、GUI、特效、水面與工具程式碼
res/          模型、材質、shader、動作資料、音樂與場景資源
include/      GLFW、GLEW、GLM、Dear ImGui、nlohmann/json 等標頭檔
lib/ dll/     連結函式庫與執行期 DLL
build/        Visual Studio 輸出與執行時使用的資源副本
```

## 操作介面

啟動後使用 `Control` 視窗開啟材質、特效與水面設定；`Editor` 視窗用來選擇動作及編輯模型影格。視窗中的選項會直接套用至目前場景。
