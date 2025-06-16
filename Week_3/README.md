# IAT Hooking
## Import Address Table (IAT)
**Import Address Table (IAT)** được sử dụng để lưu trữ địa chỉ của các API mà chương trình gọi trong lúc thực thi. API trong Windows đóng vai trò rất quan trọng, nhờ có nó mà chương trình mới có thể truy cập vào các tài nguyên của hệ thống một cách dễ dàng. Vì lý do đó, địa chỉ của API được sắp xếp trong một bảng có cấu trúc là **IAT**, điều này giúp quản lý thuận tiện hơn so với việc lưu trữ các địa chỉ này một cách rời rạc

Ngoài ra, như ta đã biết, các API mà chương trình sử dụng có thể được định nghĩa trong nhiều DLL khác nhau. Cho nên, để quản lý các địa chỉ một cách hiệu quả, chương trình sẽ tạo một IAT riêng cho mỗi DLL mà nó import vào. Ví dụ, chương trình của ta sẽ có một IAT dành riêng cho `kernel32.dll` và một IAT riêng cho `ntdll.dll`

![image](https://hackmd.io/_uploads/H1sOj7lmxg.png)

Mỗi lần máy tính khởi động lại, địa chỉ của các DLL trong bộ nhớ sẽ thay đổi và dẫn đến việc địa chỉ của các API cũng thay đổi theo. Do đó, **các địa chỉ API trong IAT sẽ không cố định**

## IAT Hooking 
**IAT Hooking** là một kỹ thuật dùng để can thiệp vào các API mà một process Windows gọi tới khiến nó không còn gọi đến địa chỉ gốc, mà gọi đến hàm do attacker chỉ định

IAT Hooking chỉ ảnh hưởng đến một module, tức là nếu hook `MessageBoxA` trong IAT của `main.exe`, chỉ `main.exe` bị ảnh hưởng, các DLL khác không bị ảnh hưởng

Cả IAT và EAT Hooking chỉ ảnh hưởng đến process hiện tại, không ảnh hưởng đến toàn bộ hệ thống hoặc các process khác
```
       [Import Address Table (IAT)]
       +----------------------------+
       |  CreateFileW -> 0x7C801D7B |  <--- entry gốc: trỏ về hàm CreateFileW trong kernel32.dll
       +----------------------------+
                  |
       (Attacker ghi đè entry trong IAT)
                  |
                  v
       +----------------------------+
       |  CreateFileW -> 0x10001000 |  <--- entry đã bị ghi đè: trỏ về MyHookFunction do attacker dựng
       +----------------------------+
```
## Demo
### Source code
Source code và file [ở đây](https://github.com/tohkabe/dak_magiz/tree/main/Week_3/Task_1)

### PoC
[Click here](https://limewire.com/d/Don28#rTGg99sZgz)

Kết quả khi quét bằng VT sẽ không bị đánh dấu là độc bởi vì hook API lành tính (`CreateFileW`) không có tác động gì gây hại cho hệ thống, nó được xem như hành vi thực hiện debug hoặc monitor

![image](https://hackmd.io/_uploads/B1QGlf_Qxg.png)

# EAT Hooking
## Export Address Table (EAT)
Nếu như IAT được dùng để lưu trữ địa chỉ của các API mà chương trình import trong quá trình thực thi, thì EAT (Export Address Table) lại được dùng để lưu trữ địa chỉ của các API mà DLL export ra bên ngoài. Các DLL trong Windows được tạo ra với mục đích để cung cấp các API cho các chương trình khác sử dụng, do đó chúng phải cung cấp thông tin về RVA (Relative Virtual Address) cho các API mà chúng export ra ngoài. Nếu DLL không cung cấp thông tin này, các chương trình sẽ không biết được địa chỉ cụ thể của những API và sẽ không thể sử dụng chúng

![image](https://hackmd.io/_uploads/ByqaeQ_mgx.png)
## EAT Hooking
EAT Hooking nhằm can thiệp vào quá trình gọi hàm xuất (exported functions) của một DLL (thường là các API của Windows) bằng cách thay đổi địa chỉ trong EAT để chuyển hướng sang một hàm khác do attacker chỉ định

EAT Hooking ảnh hưởng đến tất cả các module được load sau khi việc hooking diễn ra, tức là sửa EAT của `user32.dll` để `MessageBoxA` trỏ đến hàm giả thì mọi module sau đó gọi `MessageBoxA` qua `user32.dll` đều bị ảnh hưởng (kể cả plugin DLL được load sau đó)
```
     [Export Address Table (EAT) – mydll.dll]
     +-----------------------------+
     |  DoSomething -> 0x20001000  |  <--- entry gốc: trỏ về hàm thật trong DLL
     +-----------------------------+
                  |
       (Attacker ghi đè entry trong EAT)
                  |
                  v
     [Export Address Table (EAT) – mydll.dll]
     +-----------------------------+
     |  DoSomething -> 0x30002000  |  <--- entry bị ghi đè: trỏ về MyHookFunction 
     +-----------------------------+
```

## Demo 
Notepad không gọi `CreateFileW` qua EAT mà thường gọi từ IAT hoặc qua `ntdll` nên làm demo như IAT Hooking hoặc Inline Hooking là không được

### Source code
Source code và file [ở đây](https://github.com/tohkabe/dak_magiz/tree/main/Week_3/Task_2)

### PoC
[Click here]()

# Inline Hooking
## Inline Hooking 
**Inline hooking** gần như tương tự với IAT hooking. Điểm khác biệt là thay vì đánh lừa process gọi đến các hàm độc hại thông qua việc sửa đổi IAT table thì inline hooking ghi trực tiếp mã độc vào bên trong dll hợp lệ. DLL hợp lệ này vốn dĩ đã được process sử dụng, vì hàm cần thiết đã được ánh xạ sẵn trong IAT table

Attacker chỉ cần xác định được dll nào sẽ được process gọi đến, sau đó ghi đè khoảng 5 byte đầu tiên của hàm đó bằng một lệnh nhảy (jump). Nó sẽ tiếp tục phân giải và gọi hàm như bình thường, nhưng khi hàm được thực thi, phần mã độc đã được chèn vào sẽ được thực thi cùng với mã gốc

Lý do ghi đè khoảng 5 byte là để giữ cho process không bị lỗi và tránh bị phát hiện, phần mã độc thường sẽ thực thi lại các lệnh hợp lệ đã bị ghi đè trong 5 byte đầu của hàm gốc, rồi sau đó nhảy trở về phần còn lại của hàm ban đầu (sau đoạn đã bị ghi đè).

```
[Chương trình gọi DoSomething()]

          |
          v
[0x20001000] -> JMP 0x30002000  (-> nhảy tới mã độc)
                       |
                       v
[0x30002000] Mã độc được thực thi
[0x30002010] Chạy lại 5 byte gốc đã bị ghi đè
[0x30002015] JMP 0x20001005     (-> quay lại hàm thật)
```
## Demo
### Source code
Source code và file [ở đây](https://github.com/tohkabe/dak_magiz/tree/main/Week_3/Task_3)

### PoC
[Click here](https://limewire.com/d/ynD9n#jv1ERxIj1d)

Kết quả khi quét bằng VT sẽ không bị đánh dấu là độc bởi vì hook API lành tính (`CreateFileW`) không có tác động gì gây hại cho hệ thống, nó được xem như hành vi thực hiện debug hoặc monitor

![image](https://hackmd.io/_uploads/SyM9MFtmge.png)
