#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define SQUARE_DEV "/dev/square"

int main()
{
	long long ret;

	char buf[1];
	char write_buf[] = "testing writing";
	int offset = 10;
	int fd = 0;

	fd = open(SQUARE_DEV, O_RDWR);
	if(fd < 0)
	{
		perror("open() failed!");
		exit(1);
	}

	for(int i = 0; i <= offset; i++)
	{
		ret = write(fd, write_buf, strlen(write_buf));
		printf("Write to " SQUARE_DEV ", return %lld\n", ret);
	}

	for(int i = 0; i <= offset; i++)
	{
		lseek(fd, i, SEEK_SET);
		ret = read(fd, buf, 1);
		printf("Reading from " SQUARE_DEV "at offset %d, returned %lld\n", i, ret);
	}
	
	close(fd);
	return 0;
}
