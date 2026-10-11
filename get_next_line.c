#include "get_next_line.h"

/*Reads chunks from file descriptor and appends to accumulated buffer*/
static char	*read_and_save(int fd, char *saved)
{
	char	*buffer;
	ssize_t	bytes_read;

	buffer = malloc((size_t)BUFFER_SIZE + 1);
	if (!buffer)
	{
		free(saved);
		return (NULL);
	}
	while (!ft_strchr(saved, '\n'))
	{
		bytes_read = read(fd, buffer, BUFFER_SIZE);
		if (bytes_read == -1)
		{
			free(saved);
			free(buffer);
			return (NULL);
		}
		if (bytes_read == 0)
			break ;
		buffer[bytes_read] = '\0';
		saved = ft_strjoin(saved, buffer);
		if (!saved)
		{
			free(buffer);
			return (NULL);
		}
	}
	free(buffer);
	return (saved);
}

/*Extracts single newline-terminated line from accumulated buffer*/
static char	*extract_line(char *saved)
{
	char	*line;
	size_t	i;

	i = 0;
	if (!saved || !saved[i])
		return (NULL);
	while (saved[i] && saved[i] != '\n')
		i++;
	line = malloc(i + (saved[i] == '\n') + 1);
	if (!line)
		return (NULL);
	i = 0;
	while (saved[i] && saved[i] != '\n')
	{
		line[i] = saved[i];
		i++;
	}
	if (saved[i] == '\n')
		line[i++] = '\n';
	line[i] = '\0';
	return (line);
}

/*Trims extracted line and preserves unread leftover bytes*/
static char	*keep_leftovers(char *saved)
{
	char	*new_saved;
	size_t	i;
	size_t	j;

	i = 0;
	while (saved[i] && saved[i] != '\n')
		i++;
	if (!saved[i] || !saved[i + 1])
	{
		free(saved);
		return (NULL);
	}
	new_saved = malloc(ft_strlen(saved) - i);
	if (!new_saved)
	{
		free(saved);
		return (NULL);
	}
	i++;
	j = 0;
	while (saved[i])
		new_saved[j++] = saved[i++];
	new_saved[j] = '\0';
	free(saved);
	return (new_saved);
}

/*Reads and returns next line from given file descriptor*/
char	*get_next_line(int fd)
{
	static char	*saved;
	char		*line;

	if (fd < 0 || BUFFER_SIZE <= 0 || BUFFER_SIZE > 0x7FFFFFFF)
		return (NULL);
	saved = read_and_save(fd, saved);
	if (!saved)
		return (NULL);
	line = extract_line(saved);
	if (!line)
	{
		free(saved);
		saved = NULL;
		return (NULL);
	}
	saved = keep_leftovers(saved);
	return (line);
}
