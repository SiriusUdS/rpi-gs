#include <fcntl.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    // 1. Mount essential filesystems
    mount("devtmpfs", "/dev", "devtmpfs", 0, "");
    mount("proc", "/proc", "proc", 0, "");
    
    //Mount sysfs (Required for networking tools)
    mount("sysfs", "/sys", "sysfs", 0, "");

    // Silence Kernel Console Messages
    int printk_fd = open("/proc/sys/kernel/printk", O_WRONLY);
    if (printk_fd >= 0) {
        write(printk_fd, "1\n", 2); 
        close(printk_fd);
    }

    // Set up TTY2 for supervisor logging
    int tty2_fd = open("/dev/tty2", O_RDWR);
    if (tty2_fd >= 0) {
        dup2(tty2_fd, 1);
        dup2(tty2_fd, 2);
        if (tty2_fd > 2) close(tty2_fd);
    }

    // Give the shell a basic PATH so system() can find standard binaries
    setenv("PATH", "/bin:/sbin:/usr/bin:/usr/sbin", 1);

    write(1, "WAITING FOR ETH0...\n", 20);

    // Wait for the kernel to finish creating eth0
    while (access("/sys/class/net/eth0", F_OK) != 0) {
        sleep(1);
    }

    write(1, "ETH0 DETECTED. CONFIGURING NETWORK...\n", 38);

    // (Check your specific system. It might be /bin/ip instead of /sbin/ip)
    int ret;
    ret = system("/sbin/ip link set eth0 up");
    if (ret != 0) write(2, "WARNING: Failed to bring eth0 up\n", 33);

    ret = system("/sbin/ip addr add 192.168.0.100/24 dev eth0");
    if (ret != 0) write(2, "WARNING: Failed to assign IP\n", 29);

    //ret = system("/sbin/ip route add default via 192.168.1.1");
    //if (ret != 0) write(2, "WARNING: Failed to set gateway\n", 31);

    write(1, "SIRIUS GS STARTED\n", 18); // Prints on TTY2

    while (1) {
        pid_t pid = fork();

        if (pid == 0) {
            // --- CHILD PROCESS ---
            
            // 4. Give the ncurses app exclusive access to TTY1
            int tty1_fd = open("/dev/tty1", O_RDWR);
            if (tty1_fd >= 0) {
                dup2(tty1_fd, 0); // stdin
                dup2(tty1_fd, 1); // stdout
                dup2(tty1_fd, 2); // stderr
                if (tty1_fd > 2) {
                    close(tty1_fd);
                }
            }

            write(1, "\033[2J\033[H", 7); 

            char *args[] = { "sirius", NULL };
            execv("/sbin/sirius", args);

            write(2, "Error: Failed to start /sbin/sirius\n", 36);
            _exit(1); 
        } 
        else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);

            write(1, "App exited. Restarting in 2 seconds...\n", 39);
            sleep(2);
        } 
        else {
            write(2, "Error: Fork failed\n", 19);
            sleep(5);
        }
    }

    return 0;
}