#include <fcntl.h>
#include <stdio.h>
#include "get_next_line.h"

int	main(void)
{
	int		fd;
	char	*line;
	int		i;

	printf("--- 1. Invalid FD Test ---\n");
	printf("fd -1: %s\n\n", get_next_line(-1));

	printf("--- 2. Empty File Test ---\n");
	fd = open("empty.txt", O_RDONLY);
	printf("empty: %s\n\n", get_next_line(fd));
	close(fd);

	printf("--- 3. test.txt (BUFFER_SIZE = %d) ---\n", BUFFER_SIZE);
	fd = open("test.txt", O_RDONLY);
	i = 1;
	while (1)
	{
		line = get_next_line(fd);
		printf("Call %2d: [%s]\n", i, line);
		if (!line)
			break ;
		free(line);
		i++;
	}
	close(fd);
	return (0);
}