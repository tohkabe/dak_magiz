:::info
Lập trình inject đoạn code hiển thị messagebox vào tiến trình chuẩn notepad.exe bằng tất cả các cách dưới đây:
- Inject New Thread
- Process Hollowing
- APC Injection
:::

# Inject New Thread
## CreateRemoteThread
**CreateRemoteThread** là API Windows giúp tạo luồng thực thi (thread) trong một process khác — thường dùng để inject DLL hoặc shellcode.

`CreateRemoteThread` được nhắm đến vì không làm gián đoạn process khi nó đang thực hiện một việc gì đó quan trọng

## Inject New Thread 
**Inject New Thread** hay còn gọi là **Remote Thread Injection** là một kỹ thuật trong đó attacker:
- Ghi mã độc vào vùng nhớ của một process hợp lệ (sử dụng `WriteProcessMemory`).
- Tạo một luồng mới (new thread) trong process đó để thực thi code từ vùng nhớ đã ghi bằng `CreateRemoteThread`  hoặc inject DLL thông qua `CreateRemoteThread` và thực thi `LoadLibrary` sau đó truyền tham số trong trình `CreateRemoteThread`

Kỹ thuật này thường bị lợi dụng để ẩn mã độc trong các process hợp pháp như explorer.exe, notepad.exe, svchost.exe, v.v.

> DLL Injection là một dạng cụ thể của Inject New Thread. Cụ thể, DLL Injection sử dụng Inject New Thread để gọi `LoadLibrary` trong process đích.

### Workflow of technique
```
                                                                                           
          +-------------------+                                          +-------------------+     
          |                   |                                          |                   |     
          |                   |                                          |                   |     
          |  Notepad Process  |                                          |  Malware Process  |     
          |                   |                                          |                   |     
          |                   |           1 allocating space             |                   |     
          |-------------------| <--------------------------------------  |                   |     
          |  shellcode, dll   |                                          |                   |     
          |                   |      2 writing shellcode or dll          |                   |     
          +-------------------+ <--------------------------------------  +-------------------+     
                 ^                                                         |                       
                 |                                                         |                       
                 |                                                         |                       
                 |                                                         |                       
                 v 3 creating a remote thread to run shellcode or load dll |                       
             +---------+                                                   |                       
             |         |  <------------------------------------------------+                       
             | thread  |                                                                           
             |         |                                                                           
             +---------+                                          
```
### Detection
Kỹ thuật chiếm luồng (thread hijacking) thường được sử dụng để bypass AV. Nhưng chính vì nó phổ biến và dễ nhận diện, nên AV thường theo dõi sát kỹ thuật này. AV sẽ phân tích các API có thể bị lợi dụng để thực hiện inject code và đánh dấu chúng là hành vi đáng ngờ (chẳng hạn như `CreateRemoteThread`). Ngoài ra nó sẽ cố gắng ngăn chặn việc mở handle đến các luồng mà chúng sở hữu từ một process khác.

Như đã phân tích ở trên với kỹ thuật này, ta sẽ có dấu hiệu nhận biết như sử dụng `CreateRemoteThread` hay `LoadLibrary `. Viết config để Sysmon bắt và alert nó
```xml
<Sysmon schemaversion="4.82">
  <EventFiltering>
    <CreateRemoteThread onmatch="include">
      <TargetImage condition="contains">.exe</TargetImage>
    </CreateRemoteThread>
  </EventFiltering>
</Sysmon>
```

Log bắt được có dạng 
```xml
...
- <EventData>
  <Data Name="RuleName">-</Data> 
  <Data Name="UtcTime">2025-05-29 07:25:33.455</Data> 
  <Data Name="SourceProcessGuid">{a122b8ed-0bed-6838-1c02-000000002800}</Data> 
  <Data Name="SourceProcessId">4592</Data> 
  <Data Name="SourceImage">C:\Users\vanld5\Downloads\VCS\task1.exe</Data> 
  <Data Name="TargetProcessGuid">{a122b8ed-0b8b-6838-1202-000000002800}</Data> 
  <Data Name="TargetProcessId">5180</Data> 
  <Data Name="TargetImage">C:\Windows\System32\notepad.exe</Data> 
  <Data Name="NewThreadId">7208</Data> 
  <Data Name="StartAddress">0x00007FFA842F04F0</Data> 
  <Data Name="StartModule">C:\Windows\System32\KERNEL32.DLL</Data> 
  <Data Name="StartFunction">LoadLibraryA</Data> 
  <Data Name="SourceUser">DESKTOP-O0DBRSC\vanld5</Data> 
  <Data Name="TargetUser">DESKTOP-O0DBRSC\vanld5</Data> 
  </EventData>
  </Event>
```
![image](https://hackmd.io/_uploads/Hy8OJqHfex.png)

## Demo
### Source code
Source code và file [ở đây](https://github.com/tohkabe/dak_magiz/tree/main/Week_2)

### PoC
[Click here](https://limewire.com/d/zQLi9#ew9JzEn9Bw)

Kết quả khi quét bằng VT
![image](https://hackmd.io/_uploads/r1QKv5HGlx.png)

# Process Hollowing
## Process Hollowing
**Process Hollowing** là code injection. Attacker sử dụng loại này để ẩn mã độc trong process có vẻ vô hại và ẩn trong process gốc đã thực hiện việc inject code. **Process Hollowing** chạy một chương trình mới và inject mã độc vào đó. Vì chương trình mới do attacker tạo ra nên có thể thao túng bộ nhớ của chương trình đó. Không giống như các kỹ thuật inject code khác nơi attacker có thể phân bổ bộ nhớ mới để lưu trữ mã độc thì **Process Hollowing** sẽ cố gắng ghi đè lên code hiện có. Tùy thuộc vào cách code bị ghi đè, rất có thể nó sẽ làm hỏng quá trình thực thi ban đầu, khiến việc sử dụng bình thường của chương trình chạy không thực thi được.

![image](https://hackmd.io/_uploads/H1YFm9HGxg.png)

### Workflow of technique
1. Attacker tạo process mới ở trạng thái “tạm dừng” (suspended state) bằng `CREATE_SUSPENDED`
2. Sau khi process được tạo, bộ nhớ hợp lệ của tiến trình đó sẽ được giải phóng (làm rỗng) bằng API `NtUnmapViewOfSection()`
3. Sau đó, bằng cách sử dụng `VirtualAllocEx()`, attacker sẽ phân bổ bộ nhớ trong process đã bị rỗng và tìm entry point
4. Attacker sẽ ghi đè mã nhị phân độc hại vào bộ nhớ của process này.
5. Đổi entry point của process sang đoạn mã độc mới.
6. Process được khôi phục bằng `ResumeThread` và bắt đầu thực thi mã độc thay vì mã hợp pháp ban đầu.
 
<img src="https://www.trustedsec.com/wp-content/uploads/2023/06/proc_hollowing_flow.gif" alt="" class="wp-image-29902">

### Detection
Detect kỹ thuật **Process Hollowing** rất khó trong hầu hết các môi trường. Tuy nhiên có một số cách: 
- Các process được khởi tạo nhưng có flag `CREATE_SUSPENDED`, `WriteProcessMemory `, `ResumeThread`, `VirtualAllocEx`
- Sử dụng một công cụ như [HollowFind](https://github.com/monnappa22/HollowFind), Volatility để phân tách địa chỉ của entry point bằng plugin `malfind` 
- Nhờ vào log của Sysmon cụ thể là **eventID 1** ta có thể biết được nó có độc hại hay không
```
Process Create:
RuleName: -
UtcTime: 2025-05-30 08:11:45.427
ProcessGuid: {a122b8ed-6841-6839-8b01-000000002d00}
ProcessId: 7804
Image: C:\Windows\System32\notepad.exe
FileVersion: 10.0.19041.5794 (WinBuild.160101.0800)
Description: Notepad
Product: Microsoft® Windows® Operating System
Company: Microsoft Corporation
OriginalFileName: NOTEPAD.EXE
CommandLine: "C:\Windows\System32\notepad.exe"
CurrentDirectory: C:\Users\vanld5\Downloads\VCS\
User: DESKTOP-O0DBRSC\vanld5
LogonGuid: {a122b8ed-65eb-6839-016b-040000000000}
LogonId: 0x46B01
TerminalSessionId: 1
IntegrityLevel: Medium
Hashes: SHA1=F3A517B07528CEE3A7399386C58A9C7A59AA96B3,MD5=6F51BCABF1B2B34AD7E670AEE6DA451F,SHA256=DA5807BB0997CC6B5132950EC87EDA2B33B1AC4533CF1F7A22A6F3B576ED7C5B,IMPHASH=09ED737A03DB7295BF734A9953F6EB5E
ParentProcessGuid: {a122b8ed-6841-6839-8a01-000000002d00}
ParentProcessId: 8040
ParentImage: C:\Users\vanld5\Downloads\VCS\task2.exe
ParentCommandLine: "C:\Users\vanld5\Downloads\VCS\task2.exe" 
ParentUser: DESKTOP-O0DBRSC\vanld5
```
![image](https://hackmd.io/_uploads/HJ5TOkPGgl.png)

`ParentImage` là `task2.exe` đang spawning `notepad.exe`, đây là dấu hiệu nghi ngờ độc hại 

## Demo
### Source code
Source code và file [ở đây](https://github.com/tohkabe/dak_magiz/tree/main/Week_2/Task_2)

### PoC

[Click here](https://limewire.com/d/xv7E5#s02UFxd0tM)

Kết quả khi quét bằng VT, có lẽ vì shellcode tạo từ msfvenom dễ nhận biết nên nhiều vendor gắn tag `shellcode/mfsvenom` hơn là gắn tag về `injector`

![image](https://hackmd.io/_uploads/SylLt6Bfle.png)

# APC Injection
## APC Injection
**Asynchronous Procedure Call (APC)** là cách để thực hiện hàm một cách không đồng bộ trong context của một thread cụ thể. Mỗi thread đều có một hàng đợi APC riêng, và các thread sẽ thực hiện các APC có trong hàng đợi khi chúng tiến vào trạng thái alertable. Một thread sẽ tiến vào trạng thái alertable khi nó gọi API `SleepEx()`, `SignalObjectAndWait()`, `MsgWaitForMultipleObjectsEx()`, `WaitForMultipleObjectsEx()`, hoặc `WaitForSingleObjectEx()`. Một process có thể chèn APC vào hàng đợi của một thread khác bằng cách sử dụng API `QueueUserAPC()`

Khi một thread đang chờ đợi tác vụ của nó được hoàn thành, như việc nhận dữ liệu từ mạng thì tại điểm này thread sẽ chuyển sang trạng thái **alertable**. Trong trạng thái alertable, thread đang rảnh rỗi và sẵn sàng thực hiện các tác vụ khác thay vì chờ đợi một cách vô ích. Thay vì lãng phí thời gian chờ đợi một cách không cần thiết, thread có thể tận dụng trạng thái alertable để thực thi các tác vụ trong APC queue của nó. Rõ ràng, điều này giúp tận dụng thời gian chờ đợi một cách thông minh và giảm thời gian xử lý các tác vụ trên hệ thống.

Tính năng APC trong Windows, mặc dù có chức năng tối ưu hóa thời gian chờ đợi và quản lý thread, nhưng nó cũng có khả năng bị lợi dụng để thực hiện các tác vụ độc hại. Cụ thể, attacker thường chèn các tác vụ độc hại vào trong hàng đợi APC của target process, một khi target process tiến vào trạng thái alertable thì các tác vụ này sẽ được thực thi ngay lập tức. 

### Workflow of technique
![image](https://hackmd.io/_uploads/S1Y5XePzgl.png)

1. Injector Process sẽ inject các tác vụ độc hại vào process mục tiêu thông qua APC
2. Khi process này vào trạng thái **alertable** (alertable state), nó sẽ kiểm tra hàng đợi APC của nó
3. Sau khi bị inject, queue chứa các `Task 1`, `Task 2`, `Task 3` – trong đó có chứa mã độc
4. Khi một luồng trong process mục tiêu vào trạng thái **alertable**, Windows cho phép xử lý các hàm được chèn qua APC -> các tác vụ độc hại trong APC queue sẽ được thực thi

### Detection


## Demo
### Source code
Source code và file [ở đây](https://github.com/tohkabe/dak_magiz/tree/main/Week_2/Task_3)

### PoC
[Click here](https://limewire.com/d/JE9gR#pVEzdQMKta)

Kết quả khi quét bằng VT
![image](https://hackmd.io/_uploads/B11mdGDfge.png)
