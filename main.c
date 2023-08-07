#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define OLIVEC_IMPLEMENTATION
#include "deps/olive.c"

#define READ_END 0
#define WRITE_END 1

#define WIDTH 800
#define HEIGHT 600
#define FPS 30
u_int32_t pixels[WIDTH * HEIGHT];

#define STR2(x) #x
#define STR(x) STR2(x)

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
                         "-s", STR(WIDTH) "x" STR(HEIGHT),
                         "-r", STR(FPS),
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

    Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT, WIDTH);

    size_t duration = 10;
    float x = WIDTH / 2;
    float y = HEIGHT / 2;
    float r = HEIGHT / 8;
    float dx = 100;
    float dy = 100;
    float dt = 1.f / FPS;

    for (size_t i = 0; i < FPS * duration; i++)
    {
        float nx = x + dx * dt;
        if (0 + r < nx && nx < WIDTH - r)
        {
            x = nx;
        }
        else
        {
            dx = -dx;
        }

        float ny = y + dy * dt;
        if (0 + r < ny && ny < HEIGHT - r)
        {
            y = ny;
        }
        else
        {
            dy = -dy;
        }

        olivec_fill(oc, 0xFF181818);
        olivec_circle(oc, x, y, r, 0xFF0000FF);
        write(pipefd[WRITE_END], pixels, sizeof(*pixels) * WIDTH * HEIGHT);
    }
    close(pipefd[WRITE_END]);

    wait(NULL);

    printf("Done rendering the video. The child's pid is %d\n", child);

    return 0;
}