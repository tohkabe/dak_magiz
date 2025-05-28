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
Kỹ thuật chiếm quyền điều khiển luồng (thread hijacking) thường được sử dụng để bypass AV. Nhưng chính vì nó phổ biến và dễ nhận diện, nên AV thường theo dõi sát kỹ thuật này. AV sẽ phân tích các API có thể bị lợi dụng để thực hiện inject code và đánh dấu chúng là hành vi đáng ngờ (chẳng hạn như `CreateRemoteThread`). Ngoài ra nó sẽ cố gắng ngăn chặn việc mở handle đến các luồng mà chúng sở hữu từ một process khác.

## Demo
### Source code
Source code và file [ở đây]()

### PoC
[Click here](https://limewire.com/d/zQLi9#ew9JzEn9Bw)
