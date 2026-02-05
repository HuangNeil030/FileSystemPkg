# FileSystemPkg
UEFI 核心函數詳解手冊 (教科書註釋版)
本手冊涵蓋輸入處理 (SimpleTextInput)、事件等待 (BootServices) 與檔案系統操作 (FileProtocol) 的底層機制。

第一章：使用者輸入 (Input)
1.1 讀取按鍵 ReadKeyStroke
此函數用於「非阻塞 (Non-blocking)」地查詢鍵盤緩衝區。這意味著如果沒有按鍵，它會立刻返回，不會卡住程式。

[函數原型 Prototype]
typedef
EFI_STATUS
(EFIAPI *EFI_INPUT_READ_KEY) (
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *This,  // 呼叫者自身的 Protocol 實例指標
  OUT EFI_INPUT_KEY                   *Key    // 用於接收按鍵資料的結構指標
  );
  [教科書級參數詳解]
IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This

意義: 指向目前標準輸入裝置的介面實例。

實作: 在標準應用程式中，通常傳入 gST->ConIn (全域系統表中的 Console Input)。

層級: 這是物件導向概念在 C 語言的實現（相當於 C++ 的 this 指標）。

OUT EFI_INPUT_KEY *Key

意義: 這是由呼叫者 (Caller) 分配好記憶體空間，傳入指標讓函數「填寫」資料。

結構內容 (EFI_INPUT_KEY):

UINT16 ScanCode: 掃描碼。用於非列印字元 (如 F1~F12, 方向鍵)。例如 SCAN_UP (0x01)。

CHAR16 UnicodeChar: Unicode 字元。用於可列印字元 (如 'a', '1', '@')。

邏輯: 若 ScanCode 為 0，則看 UnicodeChar；反之亦然。

[狀態碼 (Status Codes) 意義]
EFI_SUCCESS (0):

意義: 成功讀取到一個按鍵，資料已填入 Key 結構中。

EFI_NOT_READY:

意義: 極重要。表示鍵盤緩衝區是空的 (User 沒按鍵)。因為是非阻塞函數，這是「正常」現象，通常程式會繼續迴圈等待。

EFI_DEVICE_ERROR:

意義: 硬體故障，鍵盤控制器失去回應。

1.2 等待事件 WaitForEvent
為了避免 CPU 在 while(TRUE) 迴圈中不斷查詢按鍵而造成 100% 佔用率，我們使用此函數讓 CPU 進入「等待」狀態，直到硬體中斷觸發。

[函數原型 Prototype]
// 屬於 Boot Services (gBS) 的一部分
typedef
EFI_STATUS
(EFIAPI *EFI_WAIT_FOR_EVENT) (
  IN  UINTN      NumberOfEvents, // 要等待的事件總數量
  IN  EFI_EVENT  *Event,         // 事件陣列 (Array of Events)
  OUT UINTN      *Index          // 回傳哪一個事件被觸發了
  );
  [教科書級參數詳解]
IN UINTN NumberOfEvents

意義: 告訴韌體我們要同時監聽幾個事件。

範例: 如果只等鍵盤，填 1。

IN EFI_EVENT *Event

意義: 事件陣列的起始位址。

實作: 若等鍵盤，傳入 &gST->ConIn->WaitForKey。這是 UEFI 核心預先建立好，當鍵盤有訊號時會被 Signal 的事件。

OUT UINTN *Index

意義: 輸出參數。當函數返回時，這個變數會存放「觸發事件在陣列中的索引值」。

用途: 如果同時等「鍵盤」和「計時器」，此 Index 告訴你是誰叫醒了 CPU。

第二章：檔案系統入口 (File System Entry)
在 UEFI 中，所有檔案操作必須從「根目錄 (Root)」開始。

2.1 開啟卷冊 OpenVolume
從單純的檔案系統介面 (SimpleFileSystem) 轉換為可操作的檔案介面 (FileProtocol)。

[函數原型 Prototype]
typedef
EFI_STATUS
(EFIAPI *EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME) (
  IN  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *This, // 檔案系統 Protocol 實例
  OUT EFI_FILE_PROTOCOL                **Root // [重點] 指標的指標，用來接收根目錄 Handle
  );
  [教科書級參數詳解]
OUT EFI_FILE_PROTOCOL **Root

為什麼是兩顆星 (**)?:

我們需要函數「修改」一個指標變數的值，讓它指向新的記憶體位址（新的 Handle）。

在 C 語言中，要修改 int 需傳 int*；要修改 EFI_FILE_PROTOCOL* 就需傳 EFI_FILE_PROTOCOL**。

產出: 執行成功後，*Root 會變成一個指向根目錄 (\) 的有效 Handle。

第三章：檔案操作 (File Protocol)
3.1 開啟/建立檔案 Open
這是最複雜的函數，兼具「開啟舊檔」與「建立新檔」功能。

[函數原型 Prototype]
typedef
EFI_STATUS
(EFIAPI *EFI_FILE_OPEN) (
  IN  EFI_FILE_PROTOCOL  *This,       // 父目錄 Handle (從哪裡開啟)
  OUT EFI_FILE_PROTOCOL  **NewHandle, // 輸出：新檔案的 Handle
  IN  CHAR16             *FileName,   // 檔名字串 (Unicode)
  IN  UINT64             OpenMode,    // 開啟模式 (讀/寫/新建)
  IN  UINT64             Attributes   // 檔案屬性 (僅新建時有效)
  );
  [教科書級參數詳解]
IN UINT64 OpenMode (位元遮罩 Bitmask)

EFI_FILE_MODE_READ (0x01): 唯讀。

EFI_FILE_MODE_WRITE (0x02): 可寫。

EFI_FILE_MODE_CREATE (0x80...00): 關鍵。若檔案不存在則建立；若存在則覆寫（除非搭配 Read/Write 邏輯）。

教學重點: 欲建立檔案，必須寫 READ | WRITE | CREATE。缺少 WRITE 會導致邏輯矛盾（建立了一個不能寫的空檔）。

IN UINT64 Attributes

0: 標準檔案。

EFI_FILE_DIRECTORY: 建立的是一個資料夾。

EFI_FILE_HIDDEN: 隱藏檔。

3.2 讀取與寫入 Read / Write
這兩個函數共用相似的參數邏輯，最重要的是 BufferSize 的「輸入輸出」特性。

[函數原型 Prototype]
typedef
EFI_STATUS
(EFIAPI *EFI_FILE_READ) ( // Write 的原型完全相同
  IN     EFI_FILE_PROTOCOL  *This,       // 檔案 Handle
  IN OUT UINTN              *BufferSize, // [重點] 既是輸入也是輸出
  OUT    VOID               *Buffer      // 資料緩衝區
  );
  [教科書級參數詳解]
IN OUT UINTN *BufferSize

輸入時 (IN): 告訴函數「我的 Buffer 有多大」或「我想讀寫多少 Bytes」。

Write 範例: 設為 100，表示我要寫入 100 Bytes。

輸出時 (OUT): 告訴呼叫者「實際處理了多少 Bytes」。

Read 範例: 你想讀 100 Bytes，但檔案只剩 10 Bytes。函數返回後，此變數會變成 10。

EOF 判斷 (End of File): 在 Read 時，若狀態為 EFI_SUCCESS 但 *BufferSize 變為 0，表示已讀到檔案尾端。

OUT VOID *Buffer

通用指標 (void*): 因為檔案系統不知道你要讀寫的是字串、結構體還是二進位資料，所以使用通用指標。使用前通常需要轉型或確保記憶體已配置 (AllocatePool)。

[狀態碼 (Status Codes) 意義]
EFI_BUFFER_TOO_SMALL:

發生情境: 讀取目錄 (Directory) 資訊時特別常見。

意義: 你提供的 Buffer 不夠放一個 EFI_FILE_INFO 結構。

補救: 此時 *BufferSize 會被更新為「需要的正確大小」。你需要重新配置記憶體後再呼叫一次。

3.3 刪除檔案 Delete
這個函數有一個非常特殊的行為模式：「自殺式」關閉。

[函數原型 Prototype]
typedef
EFI_STATUS
(EFIAPI *EFI_FILE_DELETE) (
  IN EFI_FILE_PROTOCOL *This  // 欲刪除的檔案 Handle
  );
  [教科書級教學重點]
Handle 自動關閉:

規則: 呼叫 Delete() 之後，無論回傳值是 EFI_SUCCESS (成功刪除) 還是 EFI_WARN_DELETE_FAILURE (刪除失敗)，傳入的 Handle (This) 都會被關閉且失效。

禁忌: 呼叫 Delete() 後，絕對不能再對該 Handle 呼叫 Close() 或 Read()，否則系統會崩潰 (System Hang)。

EFI_WARN_DELETE_FAILURE:

意義: 檔案無法刪除（通常因為屬性是 Read Only），但 Handle 已經被關閉了。

3.4 關閉檔案 Close
資源回收與資料同步。

[函數原型 Prototype]
typedef
EFI_STATUS
(EFIAPI *EFI_FILE_CLOSE) (
  IN EFI_FILE_PROTOCOL *This // 欲關閉的 Handle
  );
  [教科書級教學重點]
Flush (資料沖刷):

呼叫 Close 時，系統會強制將所有還在快取 (Cache) 中的資料寫入實體磁碟。這是確保資料不遺失的最後防線。

資源釋放:

UEFI 韌體中可開啟的 Handle 數量有限。若只開不關 (Memory Leak)，最終會導致 EFI_OUT_OF_RESOURCES 錯誤，無法再開啟任何檔案。
____________________________________________________________
cd /d D:\BIOS\MyWorkSpace\edk2

edksetup.bat Rebuild

chcp 65001

set PYTHONUTF8=1

set PYTHONIOENCODING=utf-8

rmdir /s /q Build\FileSystemPkg

build -p FileSystemPkg\FileSystemPkg.dsc -a X64 -t VS2019 -b DEBUG
