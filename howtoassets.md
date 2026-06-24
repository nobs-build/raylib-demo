# Asset processing and advanced builds

This path is intentionally more demanding: it needs FFmpeg and the platform helper script support (`tools/Build-AssetResource.ps1` on Windows, `tools/Build-AssetResource.sh` on Linux).

The asset-processing part is split into small targets:

- `bckg_res` takes `bckg.png`, scales/optimizes it with FFmpeg, and embeds the processed PNG as `out/generated-assets/bckg.o`.
- `music_res` takes `braccada_jam.ogg`, re-encodes it with FFmpeg, and embeds the processed OGG as `out/generated-assets/braccada_jam.o`.
- `logo_res` embeds `logo.png` directly as `out/generated-assets/logo.o` with `objcopy`. It does not run FFmpeg.

The `full` target depends on these targets and links their generated object files into the final executable.

## Windows asset build requirements

Install FFmpeg:

```sh
winget install Gyan.FFmpeg
```

Also enable running local `.ps1` scripts.



## Linux asset build requirements

Install FFmpeg:

```sh
sudo apt install ffmpeg
```

or

```sh-session
sudo dnf install \
    https://mirrors.rpmfusion.org/free/el/rpmfusion-free-release-$(rpm -E %rhel).noarch.rpm

sudo dnf install ffmpeg
```

The `full` target rebuilds the processed image/audio assets from the source files and then links the demo.

## The build line

```sh-session
nobs full windows_release
```

or

```sh-session
nobs full unix_release
```

## Build with your own raylib build as a dependency

Clone [nobs-build/raylib](https://github.com/nobs-build/raylib), register it with nobs, and then import it from this project's `.nobs_project.txt`.
That project has its own build dependencies, so check its README/build instructions before using it as a dependency here.

```sh
git clone https://github.com/nobs-build/raylib
cd raylib
nobs -r
```

Add this snippet near the top of `.nobs_project.txt`, before `tools`, `units`, and `profiles`:

```txt
import
{
  windows_release: raylib (windows_release);
  windows_debug: raylib (windows_debug);
  unix_release: raylib (unix_release);
  unix_debug: raylib (unix_debug);
}
```

The profile names happen to match the four profiles used by this demo, so each profile imports the corresponding raylib profile. Then update the demo target dependency to include raylib and remove the direct `-lraylib` line from that target.

For the minimal build:

```diff
-minimal:
+minimal: +raylib/raylib::build
 {
   OUTPUT_NAME="demo${OUTPUT_SUFFIX}"
   C_FILES = P"(demo.c)"
   C_FLAGS = [ "-Os", "-ffunction-sections", "-fdata-sections" ]
   LINK_FLAGS = [ "--gc-sections", "--strip-all" ]
   OBJS = P"(prebuild/${TARGET_PLATFORM}/*.o)"
-  LIB = [ "-lraylib" ]
   if os_family = "windows"
   {
     LIB += [ "-lgdi32", "-lwinmm" ]
   }
 }
```

For the asset-processing build:

```diff
-full: +bckg_res +music_res logo_res
+full: +raylib/raylib::build +bckg_res +music_res logo_res
 {
   OUTPUT_NAME="demo${OUTPUT_SUFFIX}"
   C_FILES = P"(demo.c)"
   C_FLAGS = [ "-Os", "-ffunction-sections", "-fdata-sections" ]
   LINK_FLAGS = [ "--gc-sections", "--strip-all" ]
   OBJS += logo_res.OBJS
-  LIB = [ "-lraylib" ]
   if os_family = "windows"
   {
     LIB += [ "-lgdi32", "-lwinmm" ]
   }
 }
```
