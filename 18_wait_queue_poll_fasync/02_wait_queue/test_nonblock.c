#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int fd = open(argv[1], O_RDONLY | O_NONBLOCK);
    if(fd < 0){
        perror("Failed to open\n");
        return fd;
    }

    char buffer[64];
    int ret = read(fd, buffer, sizeof(buffer));

    if( ret < 0){
        printf("Read failed : %s\n", strerror(errno));
    }else{
        printf("Read success: %s\n", buffer);
    }

    close(fd);
    
    return 0;
}