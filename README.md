# FileSystemPkg
# UEFI 開發筆記：常用 Protocol 與函數使用指南

本文件整理自 UEFI Specification 中的核心 Protocol 使用方法，包含文字輸入輸出、裝置路徑定位及檔案系統操作。

---

## 1. 文字輸入 (Text Input)

### `EFI_SIMPLE_TEXT_INPUT_PROTOCOL.ReadKeyStroke()`

讀取輸入裝置的下一個按鍵輸入 。

* **原型 (Prototype):**
```c
typedef EFI_STATUS (EFIAPI *EFI_INPUT_READ_KEY) (
   IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
   OUT EFI_INPUT_KEY *Key
);

```

* **參數 (Parameters):**
* `This`: 指向 `EFI_SIMPLE_TEXT_INPUT_PROTOCOL` 實例的指標 。


* `Key`: 指向用來存放按鍵資訊（ScanCode 與 UnicodeChar）的緩衝區指標 。




* **回傳值 (Return Values):**
* `EFI_SUCCESS`: 成功讀取到按鍵 。


* `EFI_NOT_READY`: 目前沒有按鍵輸入 。


* `EFI_DEVICE_ERROR`: 硬體錯誤 。





### `EFI_BOOT_SERVICES.WaitForEvent()`

暫停執行直到指定事件被觸發（常用於等待按鍵） 。

* **原型 (Prototype):**
```c
typedef EFI_STATUS (EFIAPI *EFI_WAIT_FOR_EVENT) (
   IN UINTN NumberOfEvents,
   IN EFI_EVENT *Event,
   OUT UINTN *Index
);

```


* **參數 (Parameters):**
* `NumberOfEvents`: `Event` 陣列中的事件數量 。


* `Event`: `EFI_EVENT` 類型的陣列 。


* `Index`: 指向觸發事件在陣列中索引值的指標 。




* **注意:** 必須在 `TPL_APPLICATION` 優先級別下呼叫 。



---

## 2. 文字輸出 (Text Output)

### `EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.SetMode()`

設定輸出裝置的顯示模式 。

* **參數 (Parameters):**
* `ModeNumber`: 欲設定的文字模式編號 。




* **功能:** 設定成功後會清除螢幕，並將游標重置於 (0,0) 。



### `EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.ClearScreen()`

清除螢幕並填入背景色 。

* **功能:** 清除顯示畫面，游標位置設為 (0,0) 。



### `EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.SetCursorPosition()`

設定游標座標 。

* **參數 (Parameters):**
* `Column`, `Row`: 游標目標位置（從 0,0 開始） 。





### `EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.EnableCursor()`

設定游標的可見性 。

* **參數 (Parameters):**
* `Visible`: `TRUE` 為顯示，`FALSE` 為隱藏 。

---

## 3. 裝置與路徑 (Device & Path)

### `EFI_BOOT_SERVICES.LocateDevicePath()`

在裝置路徑上尋找支援特定 Protocol 的裝置 Handle 。

* **原型 (Prototype):**
```c
typedef EFI_STATUS (EFIAPI *EFI_LOCATE_DEVICE_PATH) (
   IN EFI_GUID *Protocol,
   IN OUT EFI_DEVICE_PATH_PROTOCOL **DevicePath,
   OUT EFI_HANDLE *Device
);

```

* **功能:**
* 可用來從包含檔案路徑的完整 Device Path 中分離出檔案系統的 Handle 。

* 若找到 Handle，`DevicePath` 指標會前進到剩餘的路徑部分 。

---

## 4. 檔案系統操作 (File System)

### `EFI_SIMPLE_FILE_SYSTEM_PROTOCOL`

提供存取裝置上檔案系統的介面 。

* **OpenVolume()**: 開啟儲存區卷冊的根目錄 。

* 回傳根目錄的 `EFI_FILE_PROTOCOL` Handle 。

* 之後所有的檔案操作皆透過此 Root Handle 進行 。

---

## 5. 檔案操作 (File Protocol)

*`EFI_FILE_PROTOCOL` 提供檔案的開啟、讀寫、刪除與關閉功能 。

### `Open()` - 開啟或建立檔案

相對於目前 Handle (如目錄) 的位置開啟新檔案 。

* **參數 (Parameters):**
* `NewHandle`: 傳回新開啟檔案的 Handle 。

* `FileName`: 檔案名稱（可含路徑修飾符如 `.` 或 `..`） 。

* `OpenMode`:
* `EFI_FILE_MODE_READ` (0x01)
* `EFI_FILE_MODE_WRITE` (0x02)
* `EFI_FILE_MODE_CREATE` (0x8000000000000000) 。

* `Attributes` (僅在 Create 模式下有效): 如 Read Only, Hidden, Directory 等 。

### `Read()` - 讀取檔案

讀取資料或目錄項目 。

* **行為:**
* 若為**檔案**: 讀取指定位元組資料至 Buffer，檔案位置隨之增加 。


* 若為**目錄**: 讀取目錄項目 (`EFI_FILE_INFO` 結構) 。若 Buffer 太小會回傳 `EFI_BUFFER_TOO_SMALL` 並更新所需的 `BufferSize` 。





### `Write()` - 寫入檔案

寫入資料至目前檔案位置 。

* **注意:** 若空間不足但需要寫入，檔案大小會自動增長 。不支援直接寫入已開啟的目錄 。

### `Delete()` - 刪除檔案

關閉並刪除檔案 。

* **注意:** 無論成功與否，該 File Handle 都會被關閉 。若刪除失敗則回傳 `EFI_WARN_DELETE_FAILURE` 。

### `Close()` - 關閉檔案

關閉檔案 Handle 並將所有快取資料寫入裝置 (Flush) 。

這是一份為您整理的 **「系統架構與主選單流程 (System Architecture & Menu Flow)」** 教科書級技術說明文件。

這份文件非常適合作為專案的 README、開發報告（Report）或簡報（PPT）的基礎素材，幫助您清晰地解釋這個 UEFI 應用程式的底層運作邏輯。

---

# 系統架構與主選單流程

## 壹、 系統架構 (System Architecture)

在 UEFI (Unified Extensible Firmware Interface) 環境下，應用程式無法直接操作硬體（例如直接讀寫硬碟磁區或控制顯示卡），而是必須透過 UEFI 韌體提供的 **「協議 (Protocols)」** 與 **「服務 (Services)」** 來進行溝通。

本 File System Utility 的架構由上到下可分為三個主要層級：

### 1. 應用程式層 (Application Layer)

這是我們撰寫的 `FileSystem.c` 核心邏輯。負責處理使用者介面 (UI)、狀態機 (State Machine) 以及商業邏輯（如：合併兩個檔案時，先讀 A 再讀 B 的流程）。

### 2. UEFI 協議與服務層 (Protocol & Service Layer)

應用程式透過全域指標 `gST` (System Table) 與 `gBS` (Boot Services) 呼叫底層介面。本系統依賴以下四大核心組件：

* **輸入與輸出 (Console I/O)**:
* `EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL` (`gST->ConOut`): 負責清空畫面 (`ClearScreen`)、設定顏色 (`SetAttribute`)、移動游標 (`SetCursorPosition`)。
* `EFI_SIMPLE_TEXT_INPUT_PROTOCOL` (`gST->ConIn`): 負責讀取使用者的鍵盤輸入 (`ReadKeyStroke`)，包含方向鍵與一般字元。


* **檔案系統 (File System)**:
* `EFI_SIMPLE_FILE_SYSTEM_PROTOCOL`: 作為進入點，用於尋找儲存裝置並「開啟根目錄卷冊 (`OpenVolume`)」。
* `EFI_FILE_PROTOCOL`: 檔案操作的核心 Handle。所有的建立 (`Open` + Create)、刪除 (`Delete`)、讀取 (`Read`)、寫入 (`Write`) 都是針對此 Protocol 的實例進行。


* **啟動服務 (Boot Services)**:
* `gBS->WaitForEvent`: 負責「事件驅動 (Event-driven)」機制，讓 CPU 進入休眠等待按鍵，避免 100% 佔用率。
* `gBS->LocateProtocol`: 用於在系統中尋找支援檔案系統的硬體裝置。



### 3. 硬體 / 韌體層 (Hardware / Firmware Layer)

UEFI 核心韌體與硬碟的 FAT32 檔案系統。當我們呼叫 `Write` 時，UEFI 韌體會將請求轉換為實際的 Block I/O 操作，將資料寫入隨身碟或硬碟中。

---

## 貳、 主選單流程 (Main Menu Flow)

主程式 (`UefiMain`) 採用了一個典型的 **無限迴圈狀態機 (Infinite Loop State Machine)** 架構。在此架構中，程式會不斷地「繪製畫面 -> 等待輸入 -> 處理邏輯」，直到使用者觸發退出條件。

### 流程圖 (Flowchart)

```text
[UefiMain 進入點]
       │
       ▼
(隱藏游標，初始化變數 Running = TRUE, Index = 0)
       │
       ▼
 ┌───> [1. 繪製選單 (DrawMenu)] ────────────────────────┐
 │     利用 SetAttribute 標示目前的 Index (反白顯示)    │
 │     │                                                │
 │     ▼                                                │
 │   [2. 等待事件 (WaitForEvent)]                       │
 │     CPU 暫停，直到鍵盤發發中斷訊號 (按下任意鍵)      │
 │     │                                                │
 │     ▼                                                │
 │   [3. 讀取按鍵 (ReadKeyStroke)]                      │
 │     取得 ScanCode (方向鍵) 或 UnicodeChar (字元/Enter)│
 │     │                                                │
 │     ▼                                                │
 │   [4. 按鍵邏輯判斷 (Switch-Case)]                    │
 │     ├─ 按下 UP   : if (Index > 0) Index--            │
 │     ├─ 按下 DOWN : if (Index < Max) Index++          │
 │     ├─ 按下 ESC  : 變數 Running = FALSE ───────┐     │
 │     └─ 按下 ENTER:                             │     │
 │          │                                     │     │
 │          ▼                                     │     │
 │        [5. 執行對應功能]                       │     │
 │          - 清除畫面，顯示游標                  │     │
 │          - 根據 Index 呼叫對應函式             │     │
 │            (DoCreate, DoCopy, DoMerge...)      │     │
 │          - 執行完畢後，顯示 "Press any key..." │     │
 │          │                                     │     │
 │          ▼                                     │     │
 │        [6. 重置與返回]                         │     │
 │          - Reset 鍵盤緩衝區 (避免殘留按鍵)     │     │
 │          - WaitForEvent 等待返回鍵             │     │
 │          - 隱藏游標                            │     │
 └──────────┘                                     │     │
                                                  │     │
 [程式結束] <─────────────────────────────────────┴─────┘
       │
       ▼
(恢復螢幕預設狀態，顯示游標)
return EFI_SUCCESS;

```

### 流程步驟詳解：

1. **UI 繪製與狀態更新 (`DrawMenu`)**
* 程式依賴 `SelectedIndex` 變數來追蹤目前游標停留的選項。
* 每次迴圈開始時，先呼叫 `ClearScreen` 清除舊畫面，然後逐行印出選單。當印到的行數與 `SelectedIndex` 相同時，將背景色改為藍色 (`EFI_BACKGROUND_BLUE`) 以達成「高亮 (Highlight)」的視覺效果。


2. **事件驅動等待 (`WaitForEvent`)**
* 這是教科書級的標準做法。如果不使用 `WaitForEvent` 而直接用 `while` 迴圈狂問 `ReadKeyStroke`，會導致 CPU 使用率飆升至 100%（俗稱 Busy Polling 或 Busy Waiting）。
* 使用 `WaitForEvent(&gST->ConIn->WaitForKey)` 可以讓程式休眠，直到硬體通知「有按鍵進來了」才喚醒 CPU。


3. **防呆與緩衝區清理 (`Reset`)**
* 在執行完特定功能（如輸入檔名、複製檔案）後，使用者的鍵盤緩衝區可能還殘留著多餘的按鍵（例如多按了幾下 Enter）。
* 此時程式會呼叫 `gST->ConIn->Reset(gST->ConIn, FALSE)` 強制清空緩衝區，確保「Press any key to return menu」這個提示能確實攔截到使用者下一次「全新」的按鍵，防止畫面一閃而過就跳回主選單。

---

cd /d D:\BIOS\MyWorkSpace\edk2

edksetup.bat Rebuild

chcp 65001

set PYTHONUTF8=1

set PYTHONIOENCODING=utf-8

rmdir /s /q Build\FileSystemPkg

build -p FileSystemPkg\FileSystemPkg.dsc -a X64 -t VS2019 -b DEBUG
