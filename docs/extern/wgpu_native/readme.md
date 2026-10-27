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
It also holds `leaks.toml`, which names the objects of WebGPU for `[leaks] with` of a module, and a list of the calls that wgpu-native lacks.

| Target | Library | The zip of gfx-rs |
| --- | --- | --- |
| `x86_64-windows-gnu` | `wgpu_native.dll`, with its import library | `wgpu-windows-x86_64-gnu-release.zip` |
| `aarch64-windows-gnu` | `wgpu_native.dll`, with its import library | `wgpu-windows-aarch64-msvc-release.zip` |
| `x86_64-linux-gnu` | `libwgpu_native.so` | `wgpu-linux-x86_64-release.zip` |
| `aarch64-linux-gnu` | `libwgpu_native.so` | `wgpu-linux-aarch64-release.zip` |
| `x86_64-macos` | `libwgpu_native.dylib` | `wgpu-macos-x86_64-release.zip` |
| `aarch64-macos` | `libwgpu_native.dylib` | `wgpu-macos-aarch64-release.zip` |

BL-WGPU-Native compiles nothing: it takes the libraries that gfx-rs builds, each with its SHA-256, and checks them on the machine of each target:

- The library needs only the libraries of the system. On Windows, the check reads the imports of `wgpu_native.dll`.
- A program loads the library and gets an adapter, from the GPU or from the software adapter of the machine.

gfx-rs gives no build of `aarch64-windows-gnu`, so that target takes the build of `aarch64-windows-msvc`. The checks tell if it runs.

BL-WGPU-Native holds no module. Each WebGPU module of Bridge fetches its release, and links the dynamic library.
Thus a process loads one copy of wgpu-native, and the WebGPU modules share its objects. A deployment holds one copy of the library, beside the modules.
A module calls no function of the list of the calls that wgpu-native does not implement, for example `wgpuInstanceWaitAny`.

## Contributing

| Document | Content |
| --- | --- |
| `docs/contributing/needs.md` | The needs that each decision meets, and how to weigh a decision. |
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
