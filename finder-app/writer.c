#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    // Настройка логирования через syslog
    openlog("writer", LOG_PID, LOG_USER);

    // Проверка количества аргументов (должно быть 2 аргумента: путь к файлу и строка)
    if (argc != 3) {
        syslog(LOG_ERR, "Invalid number of arguments: expected 2, got %d", argc - 1);
        fprintf(stderr, "Error: Two arguments required: <file_path> <string_to_write>\n");
        closelog();
        return 1;
    }

    const char *writefile = argv[1];
    const char *writestr = argv[2];

    // Открытие файла для записи (создание, перезапись, права 0644)
    int fd = open(writefile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        syslog(LOG_ERR, "Failed to open file %s: %s", writefile, strerror(errno));
        perror("Error opening file");
        closelog();
        return 1;
    }

    // Запись строки в лог syslog со статусом LOG_DEBUG
    syslog(LOG_DEBUG, "Writing '%s' to '%s'", writestr, writefile);

    // Запись строки в файл
    ssize_t bytes_written = write(fd, writestr, strlen(writestr));
    if (bytes_written == -1 || bytes_written < (ssize_t)strlen(writestr)) {
        syslog(LOG_ERR, "Failed to write to file %s: %s", writefile, strerror(errno));
        perror("Error writing to file");
        close(fd);
        closelog();
        return 1;
    }

    close(fd);
    closelog();
    return 0;
}