/** @file
  FileSystem Utility based on provided requirements and UI photo.
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/ShellLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Library/ShellLib.h>

// 定義與照片一致的 UI 常數
#define MENU_ITEM_COUNT 6
#define MAX_INPUT_LEN   100

CHAR16 *mMenuItems[] = {
  L"1. Create file",
  L"2. Delete file",
  L"3. Read file",
  L"4. Copy file",
  L"5. Merge two file",
  L"6. Exit"
};

// 輔助函數宣告
EFI_STATUS GetUserInput(IN CHAR16 *Prompt, OUT CHAR16 *InputBuffer, IN UINTN MaxLen);
EFI_STATUS OpenRootVolume(OUT EFI_FILE_PROTOCOL **RootHandle);

// 功能實作宣告
VOID DoCreateFile();
VOID DoDeleteFile();
VOID DoReadFile();
VOID DoCopyFile();
VOID DoMergeFile();

//
// 畫面與選單邏輯
//
VOID
DrawMenu (
  IN UINTN SelectedIndex
  )
{
  UINTN Index;

  // 清除螢幕 [cite: 194]
  gST->ConOut->ClearScreen(gST->ConOut);

  // 繪製綠色標題 "File System Utility" (參考照片)
  // 使用 SetAttribute 設定背景色與前景色
  gST->ConOut->SetAttribute(gST->ConOut, EFI_BACKGROUND_GREEN | EFI_WHITE);
  Print(L"File System Utility           \n");
  
  // 恢復預設顏色
  gST->ConOut->SetAttribute(gST->ConOut, EFI_BACKGROUND_BLACK | EFI_LIGHTGRAY);

  // 繪製選單項目
  for (Index = 0; Index < MENU_ITEM_COUNT; Index++) {
    if (Index == SelectedIndex) {
      // 選中項目顯示藍色背景 (參考照片)
      gST->ConOut->SetAttribute(gST->ConOut, EFI_BACKGROUND_BLUE | EFI_WHITE);
    } else {
      gST->ConOut->SetAttribute(gST->ConOut, EFI_BACKGROUND_BLACK | EFI_LIGHTGRAY);
    }
    Print(L"%s\n", mMenuItems[Index]);
  }

  // 恢復並顯示底部操作提示
  gST->ConOut->SetAttribute(gST->ConOut, EFI_BACKGROUND_BLACK | EFI_LIGHTGRAY);
  gST->ConOut->SetCursorPosition(gST->ConOut, 0, 15); // [cite: 201]
  gST->ConOut->SetAttribute(gST->ConOut, EFI_GREEN);
  Print(L"Up: ");
  gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE);
  Print(L"Up   ");
  
  gST->ConOut->SetAttribute(gST->ConOut, EFI_GREEN);
  Print(L"Down: ");
  gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE);
  Print(L"Down   ");

  gST->ConOut->SetAttribute(gST->ConOut, EFI_GREEN);
  Print(L"Enter: ");
  gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE);
  Print(L"Select   ");

  gST->ConOut->SetAttribute(gST->ConOut, EFI_GREEN);
  Print(L"Esc: ");
  gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE);
  Print(L"Return\n");
}

/**
  主程式進入點
**/
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS      Status;
  EFI_INPUT_KEY   Key;
  UINTN           SelectedIndex = 0;
  BOOLEAN         Running = TRUE;

  // 隱藏游標 [cite: 210]
  gST->ConOut->EnableCursor(gST->ConOut, FALSE);

  while (Running) {
    DrawMenu(SelectedIndex);

    // 等待按鍵事件 [cite: 172]
    gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, NULL);
    
    // 讀取按鍵 
    Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
    if (EFI_ERROR(Status)) continue;

    switch (Key.ScanCode) {
      case SCAN_UP: // [cite: 166]
        if (SelectedIndex > 0) SelectedIndex--;
        break;
      case SCAN_DOWN:
        if (SelectedIndex < MENU_ITEM_COUNT - 1) SelectedIndex++;
        break;
      case SCAN_ESC:
        Running = FALSE;
        break;
      default:
        if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
          // 清除畫面準備執行功能
          gST->ConOut->SetAttribute(gST->ConOut, EFI_BACKGROUND_BLACK | EFI_LIGHTGRAY);
          gST->ConOut->ClearScreen(gST->ConOut);
          gST->ConOut->EnableCursor(gST->ConOut, TRUE); // 執行功能時顯示游標

          switch (SelectedIndex) {
            case 0: DoCreateFile(); break; // Create
            case 1: DoDeleteFile(); break; // Delete
            case 2: DoReadFile(); break;   // Read
            case 3: DoCopyFile(); break;   // Copy
            case 4: DoMergeFile(); break;  // Merge
            case 5: Running = FALSE; break; // Exit
          }

          // 執行完畢，暫停讓使用者看結果
          Print(L"\nPress any key to return menu...");
          gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, NULL);
          gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
          gST->ConOut->EnableCursor(gST->ConOut, FALSE);
        }
        break;
    }
  }

  gST->ConOut->ClearScreen(gST->ConOut);
  gST->ConOut->EnableCursor(gST->ConOut, TRUE);
  return EFI_SUCCESS;
}

//
// 1. Create file with user input context
//
VOID DoCreateFile() {
  EFI_STATUS Status;
  EFI_FILE_PROTOCOL *Root = NULL;
  EFI_FILE_PROTOCOL *FileHandle = NULL;
  CHAR16 FileName[MAX_INPUT_LEN];
  CHAR16 Content[MAX_INPUT_LEN];
  UINTN  WriteSize;

  Print(L"Function: Create File\n");
  
  if (EFI_ERROR(GetUserInput(L"Enter Filename: ", FileName, MAX_INPUT_LEN))) return;
  if (EFI_ERROR(GetUserInput(L"Enter Content: ", Content, MAX_INPUT_LEN))) return;

  if (EFI_ERROR(OpenRootVolume(&Root))) return;

  // 使用 EFI_FILE_MODE_CREATE [cite: 312]
  Status = Root->Open(Root, &FileHandle, FileName, 
                      EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 
                      0); // Attributes default 0

  if (!EFI_ERROR(Status)) {
    WriteSize = StrLen(Content) * 2; // Unicode size
    Status = FileHandle->Write(FileHandle, &WriteSize, Content); // [cite: 361]
    FileHandle->Close(FileHandle); // [cite: 332]
    Print(L"File Created Successfully.\n");
  } else {
    Print(L"Error Creating File: %r\n", Status);
  }
  Root->Close(Root);
}

//
// 2. Delete an existing file
//
VOID DoDeleteFile() {
  EFI_STATUS Status;
  EFI_FILE_PROTOCOL *Root = NULL;
  EFI_FILE_PROTOCOL *FileHandle = NULL;
  CHAR16 FileName[MAX_INPUT_LEN];

  Print(L"Function: Delete File\n");
  if (EFI_ERROR(GetUserInput(L"Enter Filename to Delete: ", FileName, MAX_INPUT_LEN))) return;

  if (EFI_ERROR(OpenRootVolume(&Root))) return;

  // 開啟檔案以進行刪除 (需 Write 權限)
  Status = Root->Open(Root, &FileHandle, FileName, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);

  if (!EFI_ERROR(Status)) {
    // 呼叫 Delete Protocol [cite: 337]
    Status = FileHandle->Delete(FileHandle); 
    if (!EFI_ERROR(Status)) {
      Print(L"File Deleted Successfully.\n");
    } else {
      Print(L"Delete Failed (Handle Closed): %r\n", Status); // [cite: 340]
    }
  } else {
    Print(L"File Not Found or Error: %r\n", Status);
  }
  Root->Close(Root);
}

//
// 3. Read a file and display data
//
VOID DoReadFile() {
  EFI_STATUS Status;
  EFI_FILE_PROTOCOL *Root = NULL;
  EFI_FILE_PROTOCOL *FileHandle = NULL;
  CHAR16 FileName[MAX_INPUT_LEN];
  UINT64 FileSize;
  VOID   *Buffer;
  UINTN  ReadSize;
  EFI_FILE_INFO *FileInfo = NULL;
  UINTN InfoSize = 0;

  Print(L"Function: Read File\n");
  if (EFI_ERROR(GetUserInput(L"Enter Filename to Read: ", FileName, MAX_INPUT_LEN))) return;

  if (EFI_ERROR(OpenRootVolume(&Root))) return;

  Status = Root->Open(Root, &FileHandle, FileName, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR(Status)) {
    Print(L"Error Opening File: %r\n", Status);
    Root->Close(Root);
    return;
  }

  // 取得檔案大小以分配記憶體 (GetInfo) [cite: 272]
  // 簡化起見，先嘗試取得 InfoSize
  FileHandle->GetInfo(FileHandle, &gEfiFileInfoGuid, &InfoSize, NULL);
  FileInfo = AllocateZeroPool(InfoSize);
  Status = FileHandle->GetInfo(FileHandle, &gEfiFileInfoGuid, &InfoSize, FileInfo);

  if (!EFI_ERROR(Status)) {
    FileSize = FileInfo->FileSize;
    Buffer = AllocateZeroPool((UINTN)FileSize + 2); // +2 for null terminator
    ReadSize = (UINTN)FileSize;

    Status = FileHandle->Read(FileHandle, &ReadSize, Buffer); // [cite: 343]
    if (!EFI_ERROR(Status)) {
      Print(L"--- Content Start ---\n");
      // 假設是 Unicode 文字檔，如果亂碼則可能為 ASCII
      Print(L"%s\n", Buffer); 
      Print(L"--- Content End ---\n");
    }
    FreePool(Buffer);
  }
  
  if (FileInfo) FreePool(FileInfo);
  FileHandle->Close(FileHandle);
  Root->Close(Root);
}

//
// 4. Copy File
//
VOID DoCopyFile() {
  EFI_STATUS Status;
  EFI_FILE_PROTOCOL *Root = NULL;
  EFI_FILE_PROTOCOL *SrcHandle = NULL;
  EFI_FILE_PROTOCOL *DstHandle = NULL;
  CHAR16 SrcName[MAX_INPUT_LEN], DstName[MAX_INPUT_LEN];
  UINTN BufferSize = 1024; // 1KB Buffer
  VOID *Buffer;
  UINTN ReadSize;

  Print(L"Function: Copy File\n");
  GetUserInput(L"Source File: ", SrcName, MAX_INPUT_LEN);
  GetUserInput(L"Dest File: ", DstName, MAX_INPUT_LEN);

  if (EFI_ERROR(OpenRootVolume(&Root))) return;

  // Open Source
  Status = Root->Open(Root, &SrcHandle, SrcName, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR(Status)) {
    Print(L"Error Source: %r\n", Status);
    Root->Close(Root);
    return;
  }

  // Create Dest
  Status = Root->Open(Root, &DstHandle, DstName, 
             EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
  
  if (!EFI_ERROR(Status)) {
    Buffer = AllocatePool(BufferSize);
    do {
      ReadSize = BufferSize;
      SrcHandle->Read(SrcHandle, &ReadSize, Buffer);
      if (ReadSize > 0) {
        DstHandle->Write(DstHandle, &ReadSize, Buffer);
      }
    } while (ReadSize > 0);
    
    FreePool(Buffer);
    DstHandle->Close(DstHandle);
    Print(L"Copy Done.\n");
  }

  SrcHandle->Close(SrcHandle);
  Root->Close(Root);
}

//
// 5. Merge two file data
//
VOID DoMergeFile() {
  EFI_STATUS Status;
  EFI_FILE_PROTOCOL *Root = NULL;
  EFI_FILE_PROTOCOL *H1 = NULL, *H2 = NULL, *HDst = NULL;
  CHAR16 F1[MAX_INPUT_LEN], F2[MAX_INPUT_LEN], F3[MAX_INPUT_LEN];
  VOID *Buffer;
  UINTN ReadSize, BufSize = 1024;

  Print(L"Function: Merge Two Files\n");
  GetUserInput(L"File 1: ", F1, MAX_INPUT_LEN);
  GetUserInput(L"File 2: ", F2, MAX_INPUT_LEN);
  GetUserInput(L"Output File: ", F3, MAX_INPUT_LEN);

  if (EFI_ERROR(OpenRootVolume(&Root))) return;

  // Open F1, F2
  if (EFI_ERROR(Root->Open(Root, &H1, F1, EFI_FILE_MODE_READ, 0))) {
    Print(L"Error opening File 1\n"); Root->Close(Root); return;
  }
  if (EFI_ERROR(Root->Open(Root, &H2, F2, EFI_FILE_MODE_READ, 0))) {
    Print(L"Error opening File 2\n"); H1->Close(H1); Root->Close(Root); return;
  }

  // Create Output
  Status = Root->Open(Root, &HDst, F3, 
             EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);

  if (!EFI_ERROR(Status)) {
    Buffer = AllocatePool(BufSize);
    
    // Copy F1 -> Dst
    do {
      ReadSize = BufSize;
      H1->Read(H1, &ReadSize, Buffer);
      if (ReadSize > 0) HDst->Write(HDst, &ReadSize, Buffer);
    } while (ReadSize > 0);

    // Copy F2 -> Dst (Append)
    do {
      ReadSize = BufSize;
      H2->Read(H2, &ReadSize, Buffer);
      if (ReadSize > 0) HDst->Write(HDst, &ReadSize, Buffer);
    } while (ReadSize > 0);

    FreePool(Buffer);
    HDst->Close(HDst);
    Print(L"Merge Done.\n");
  }

  H1->Close(H1);
  H2->Close(H2);
  Root->Close(Root);
}

//
// Helper: 取得使用者輸入
//
EFI_STATUS GetUserInput(IN CHAR16 *Prompt, OUT CHAR16 *InputBuffer, IN UINTN MaxLen) {
  EFI_INPUT_KEY Key;
  UINTN Count = 0;
  
  Print(Prompt);
  
  while (Count < MaxLen - 1) {
    gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, NULL);
    gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);

    if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
      break;
    } else if (Key.UnicodeChar == CHAR_BACKSPACE) {
      if (Count > 0) {
        Print(L"\b \b");
        Count--;
      }
    } else if (Key.UnicodeChar >= 0x20 && Key.UnicodeChar <= 0x7E) {
      InputBuffer[Count] = Key.UnicodeChar;
      Print(L"%c", Key.UnicodeChar);
      Count++;
    }
  }
  InputBuffer[Count] = L'\0';
  Print(L"\n");
  return EFI_SUCCESS;
}

//
// Helper: 取得檔案系統根目錄
//
EFI_STATUS OpenRootVolume(OUT EFI_FILE_PROTOCOL **RootHandle) {
  EFI_STATUS Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFileSystem;
  
  // 使用 ShellLib 方便取得當前檔案系統，或使用 LocateHandle
  // 這裡使用 gEfiSimpleFileSystemProtocolGuid
  Status = gBS->LocateProtocol(
             &gEfiSimpleFileSystemProtocolGuid,
             NULL,
             (VOID **)&SimpleFileSystem
             );
             
  if (EFI_ERROR(Status)) {
    Print(L"Cannot find FileSystem: %r\n", Status);
    return Status;
  }

  return SimpleFileSystem->OpenVolume(SimpleFileSystem, RootHandle); // [cite: 246]
}