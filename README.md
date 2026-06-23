# Example raylib project with custom build steps

This repository shows one way to build a small raylib project with nobs.
The main point is that the project can be built from a normal terminal without starting the w64devkit shell directly.

The default build is intentionally simple: it links prebuilt asset objects from `prebuild/`.
The `full` target shows the same demo with custom asset-processing steps, so it can be used as a reference for projects that need generated build inputs.

## Minimal build

The default target is `minimal`. It uses already processed assets checked into `prebuild/`, so it does not require FFmpeg or the asset processing scripts.

```sh
nobs windows_release
```

Use another profile if needed:

```sh
nobs windows_debug
nobs unix_release
nobs unix_debug
```

## Build with asset processing

The `full` target rebuilds the processed image/audio assets from the source files and then links the demo.

```sh
nobs full windows_release
```

This path is intentionally more demanding: it needs FFmpeg and the platform helper script support (`tools/Build-AssetResource.ps1` on Windows, `tools/Build-AssetResource.sh` on Linux).

The asset-processing part is split into small targets:

- `bckg_res` takes `bckg.png`, scales/optimizes it with FFmpeg, and embeds the processed PNG as `out/generated-assets/bckg.o`.
- `music_res` takes `braccada_jam.ogg`, re-encodes it with FFmpeg, and embeds the processed OGG as `out/generated-assets/braccada_jam.o`.
- `logo_res` embeds `logo.png` directly as `out/generated-assets/logo.o` with `objcopy`. It does not run FFmpeg.

The `full` target depends on these targets and links their generated object files into the final executable.

## Windows requirements

This build uses mingw packaged with raylib installation, it can be run without w64devkit though.

Install raylib so that the bundled toolchain is available, then run:

```sh
nobs --find-compilers
```

This creates the nobs user registry and prints its path. On Windows it is normally:

```txt
%APPDATA%\nobs\.nobs_registry.txt
```

Open that file and add this entry under `tools`. Adjust `toolchain_path` to match your raylib installation:

```txt
tools {
raylib_gcc {
  tool_type = "gnu-gcc"
  toolchain_version = "15.2.0"
  arch = "x86_64"
  toolchain_path = "C:\raylib" // path to raylib installation dir
}
}
```

The minimal build does not require FFmpeg and does not require PowerShell script execution to be enabled.

For the asset-processing build, install FFmpeg:

```sh
winget install Gyan.FFmpeg
```

Also enable running local `.ps1` scripts.

## Linux requirements

Install nobs, raylib development files, a C compiler, binutils, and the libraries normally needed for raylib desktop builds. Alternatively, build raylib yourself as a nobs dependency; see the "Build with your own raylib build as a dependency" section below.

For the minimal build:

```sh
nobs unix_release
```

For the asset-processing build, also install FFmpeg:

```sh
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
