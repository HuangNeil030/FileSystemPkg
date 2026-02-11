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
* 
`NewHandle`: 傳回新開啟檔案的 Handle 。


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

---

cd /d D:\BIOS\MyWorkSpace\edk2

edksetup.bat Rebuild

chcp 65001

set PYTHONUTF8=1

set PYTHONIOENCODING=utf-8

rmdir /s /q Build\FileSystemPkg

build -p FileSystemPkg\FileSystemPkg.dsc -a X64 -t VS2019 -b DEBUG
