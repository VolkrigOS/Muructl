/*
*       main muructl structure
*       see LICENSE for more licensing details.
*/

#include <sys/socket.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include "muru.h"

int send_command(const char *command)
{
    int fd;
    struct sockaddr_un addr;
    char response[256];
    ssize_t n;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("muructl: socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, MURU_SOCKET,
        sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("muructl: connect");
        fprintf(stderr, "Try running murud\n");
        close(fd);
        return 1;
    }

    if (write(fd, command, strlen(command)) < 0) {
        perror("muructl: write");
        close(fd);
        return 1;
    }

    n = read(fd, response, sizeof(response) - 1);
    if (n > 0) {
        response[n] = '\0';
        fputs(response, stdout);
    } else if (n < 0) {
        perror("muructl: read");
    } else {
        fprintf(stderr, "muructl: closed connection\n");
    }

    close(fd);
    return n > 0 ? 0 : 1;
}

void usage(void)
{
    fprintf(stdout,
        "usage: muructl <command> [service]\n"
        "\n"
        "commands:\n"
        "  start        NAME      Start service\n"
        "  stop [-p]    NAME      Stop service\n"
        "  restart [-p] NAME      Restart service\n"
        "  status [-p]  NAME      Show service status\n"
        "  list                   List services\n");
}

int s_start(const char *service)
{
    char command[4096];

    char path[256];
    snprintf(path, sizeof(path), "%s/%s.service", MURU_SERVICES_DIR, service);

    if (access(path, F_OK) != 0) {
        fprintf(stderr, "muructl: create a file named %s.service based on example.service\n", service);
        return 1;
    }

    if (snprintf(command, sizeof(command), "start %s\n",
        service) >= (int)sizeof(command)) {
        fprintf(stderr, "muructl: command too long\n");
        return 1;
    }

    return send_command(command);
}

int s_list(void)
{
    if (access(MURU_SOCKET, F_OK) != 0) {
        fprintf(stderr, "muructl: %s does not exist\n", MURU_SOCKET);
        return 1;
    }

    return send_command("list");
}

int s_stop(const char *service, int p)
{
    char command[4096];
    int n;

    n = snprintf(command, sizeof(command), p ? "stopid %s\n" : "stop %s\n", service);
    if (n < 0 || (size_t)n >= sizeof(command)) {
        fprintf(stderr, "muructl: command too long\n");
        return 1;
    }

    return send_command(command);
}


int main(int argc, char **argv) 
{
    if (geteuid() != 0) {
        fprintf(stderr, "are you root?\n");
        return 1;
    }
    if (argc < 2)
    {
        usage();
        return 1;
    }

    if (access(MURU_SOCKET, F_OK) != 0) {
        fprintf(stderr, "muructl: %s does not exist\n", MURU_SOCKET);
        return 1;
    }
    const char *command = argv[1];
    if (strcmp(command, "start") == 0 && argc == 3)
        return s_start(argv[2]);

    if (strcmp(command, "stop") == 0 && argc == 3)
        return s_stop(argv[2], 0);

    if (strcmp(command, "stop") == 0 && argc == 4 && strcmp(argv[2], "-p") == 0) {
        char *end;
        long pid;

        errno = 0;
        pid = strtol(argv[3], &end, 10);
        if (errno != 0 || *argv[3] == '\0' || *end != '\0' || pid <= 0) {
            fprintf(stderr, "muructl: invalid PID: %s\n", argv[3]);
            return 1;
        }
        return s_stop(argv[3], 1);
    }

    if (strcmp(command, "list") == 0)
        return s_list();

    printf("%s", command);

    usage();
    return 1;
}