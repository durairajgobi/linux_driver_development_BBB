#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <poll.h>

int main(int argc, char *argv[])
{
    int fd = open(argv[1], O_RDONLY );
    if(fd < 0){
        perror("Failed to open\n");
        return fd;
    }

    struct pollfd pfd = {
        .fd = fd,
        .events = POLLIN,
    };

    char buffer[64];

    printf("Waiting for button press.. (Ctrl + C to exit)\n");

    while(1){
        int ret = poll(&pfd, 1, -1);
        if(ret < 0){
            perror("poll failed\n");
            break;
        }

        if(pfd.revents & POLLIN){

            int n = read(fd, buffer, sizeof(buffer) - 1);
            if(n < 0){
                perror("read failed\n");
                break;
            }

            buffer[n] = '\0';
            printf("Received: %s \n", buffer);

        }
    }

    close(fd);
    return 0;
}