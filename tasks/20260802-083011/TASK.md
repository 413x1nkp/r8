# Build raylib from the source instead of shipping pre-built binaries

- STATUS: OPEN
- PRIORITY: 100

On one of my machines I have an oldoldstable Debian where the official Raylib 6.0 [binaries](https://github.com/raysan5/raylib/releases/tag/6.0) do not work because of an old glibc.

We need to build raylib from the sources. We already do that in [Musializer](https://github.com/tsoding/musializer) just steal some code from there.

Should be probably done before [20260801-113453](../20260801-113453/TASK.md) and [20260801-113457](../20260801-113457/TASK.md).
