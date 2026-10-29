# BL-WGPU-Native

The release of wgpu-native, a C library of WebGPU, for the modules of Bridge: headers, library and checks.

## Install

Python 3.11 or a later version.

## Usage

| Command | Result |
| --- | --- |
| `python project.py fetch` | Downloads what the build needs. |
| `python project.py build` | Builds the project. |
| `python project.py pack` | Packs the build for a release. |
| `python project.py test` | Runs the tests. |
| `python project.py test --break` | Runs the tests with their claims made false. Each test must fail. |
| `python project.py test --branches` | Runs the tests on a build with probes. Each branch of the code must run. |
| `python project.py test --leaks` | Runs the tests on a build with counts. Each test must give back what the code takes. |
| `python project.py check` | Checks the project. |
| `python project.py clean` | Removes what the build made. Keeps what `fetch` downloaded. |
| `python project.py distclean` | Removes the folder `build`, also what `fetch` downloaded. |

Each release holds, for each target, the wgpu-native library of gfx-rs `v29.0.1.1`, its headers `webgpu.h` and `wgpu.h` in `include/webgpu`, and its notices.
It also holds `leaks.toml`, which names the objects of WebGPU for `[leaks] with` of a module. It holds `gaps.txt` too: the list of the calls that wgpu-native does not implement.

| Target | Library | The zip of gfx-rs |
| --- | --- | --- |
| `x86_64-windows-gnu` | `wgpu_native.dll` | `wgpu-windows-x86_64-gnu-release.zip` |
| `aarch64-windows-gnu` | `wgpu_native.dll` | `wgpu-windows-aarch64-msvc-release.zip` |
| `x86_64-linux-gnu` | `libwgpu_native.so` | `wgpu-linux-x86_64-release.zip` |
| `aarch64-linux-gnu` | `libwgpu_native.so` | `wgpu-linux-aarch64-release.zip` |
| `x86_64-macos` | `libwgpu_native.dylib` | `wgpu-macos-x86_64-release.zip` |
| `aarch64-macos` | `libwgpu_native.dylib` | `wgpu-macos-aarch64-release.zip` |

BL-WGPU-Native compiles nothing: it takes the libraries that gfx-rs builds, each with its SHA-256, and checks them on the machine of each target:

- The library needs only the libraries of the system, and the needs that this document states for its target. On the machine of each target, a check reads the imports of the `wgpu_native.dll` of the two targets of Windows. On Linux and macOS, the program of the next check loads the library.
- A program loads the library and gets an adapter, from the GPU or from the software adapter of the machine.

gfx-rs gives no build of `aarch64-windows-gnu`, so that target takes the build of `aarch64-windows-msvc`.
On `aarch64-windows-gnu`, the library needs `vcruntime140.dll`, the runtime of MSVC, which Windows does not hold.
A deployment on that target needs the Microsoft Visual C++ Redistributable for arm64, installed on the machine. The release holds no file of Microsoft.

BL-WGPU-Native holds no module. Each WebGPU module of Bridge fetches its release, and links the dynamic library.
Thus a process loads one copy of wgpu-native, and the WebGPU modules share its objects. A deployment holds one copy of the library, beside the modules.
The library is no module, so `content.ignore` of the deployment names it.
A module calls no function of the list of the calls that wgpu-native does not implement, for example `wgpuInstanceWaitAny`.

A module links the library by its bare name, from the folder of the library. Thus the module records only that name.
The rpath of the module is its own folder: `$ORIGIN` on Linux and `@loader_path` on macOS. Thus the loader finds the copy beside the module.

On Linux, `libwgpu_native.so` has no soname, so a link by its path records that path.
On macOS, the install name of `libwgpu_native.dylib` is `@rpath/libwgpu_native.dylib`.
On Windows, zig links `wgpu_native.dll` itself, so the release holds no import library.

## Contributing

| Document | Content |
| --- | --- |
| `docs/contributing/needs.md` | The needs that each decision meets. |
| `docs/contributing/decisions.md` | The steps of a decision. |
| `docs/contributing/coding_style.md` | How to write the code. |
| `docs/contributing/documentation_style.md` | How to write the documents. |
| `docs/contributing/tests.md` | How to write the tests. |
| `docs/contributing/commits.md` | How to write a commit. |
| `docs/contributing/pull_request.md` | How to write a pull request. |
| `docs/contributing/agreement.md` | The agreement `cla.md`, which each contributor signs. |

## License

The GNU Affero General Public License, version 3 only. The file `license` holds the text.
Each source of third-party code has its notice in `third_party/<source>/license`.
wgpu-native is under the MIT licence and the Apache licence 2.0, at the choice of the user: `third_party/wgpu_native/license` holds both.
