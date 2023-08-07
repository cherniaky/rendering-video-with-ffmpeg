#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define READ_END 0
#define WRITE_END 1

#define WIDTH 800
#define HEIGHT 600
u_int32_t pixels[WIDTH * HEIGHT];

int main()
{
    int pipefd[2];

    if (pipe(pipefd) < 0)
    {
        fprintf(stderr, "ERROR: could not create a pipe: %s\n", strerror(errno));
        return 1;
    }

    pid_t child = fork();
    if (child < 0)
    {
        fprintf(stderr, "ERROR: could not fork a child: %s\n", strerror(errno));
        return 1;
    }

    if (child == 0)
    {
        if (dup2(pipefd[READ_END], STDIN_FILENO) < 0)
        {
            fprintf(stderr, "ERROR: could not reopen read end as stdin: %s\n", strerror(errno));
        }
        close(pipefd[WRITE_END]);

        int ret = execlp("ffmpeg", "ffmpeg",
                         "-loglevel", "verbose",
                         "-y",
                         "-f", "rawvideo",
                         "-pix_fmt", "rgba",
                         "-s", "800x600",
                         "-r", "60",
                         "-an",
                         "-i", "-",
                         "-c:v", "libx264",
                         "output.mp4",
                         NULL);
        if (ret < 0)
        {
            fprintf(stderr, "ERROR: could not run ffmpeg as child process: %s\n", strerror(errno));
        }

        assert(0 && "unreachable");
    }

    close(pipefd[READ_END]);

    for (size_t i = 0; i < 800 * 600; i++)
    {
        pixels[i] = 0xFF0000FF;
    }

    for (size_t i = 0; i < 60 * 5; i++)
    {
        write(pipefd[WRITE_END], pixels, sizeof(*pixels) * WIDTH * HEIGHT);
    }
    close(pipefd[WRITE_END]);

    wait(NULL);

    printf("Done rendering the video. The child's pid is %d\n", child);

    return 0;
}