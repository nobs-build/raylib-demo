# Example raylib project

This repository shows one way to build a small raylib project with [nobs](https://nobs.build).


The default build is intentionally simple: it links prebuilt asset objects from `prebuild/`. For more complex build with asset processing see bottom of the page.

## Windows requirements

This build uses mingw packaged with raylib installation, it can be run without w64devkit though.

Install nobs and raylib so that the bundled toolchain is available, then run:

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

## Linux requirements

Install nobs and raylib development files.

## Minimal build

The default target is `minimal`. It uses already processed assets checked into `prebuild/`, so it does not require FFmpeg or the asset processing scripts.

For Windows run:

```sh
nobs windows_release
```

For Linux-based:

```sh
nobs unix_release
```

## Asset processing

For the full asset-processing build and advanced build notes, see [howtoassets.md](howtoassets.md).




