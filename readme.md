# BM-WebGPU

Draws with WebGPU in the native modules of a world: one wgpu-native library, a window, a frame, a device, meshes.

## Install

Python 3.11 or a later version.
The tests need a WebGPU adapter: a GPU, or a software adapter, for example lavapipe of Mesa on Linux or WARP on Windows.
The tests of `device` open a window, so they also need a display.

## Usage

| Command | Result |
| --- | --- |
| `python project.py fetch` | Downloads what the build needs. |
| `python project.py build` | Builds the project. |
| `python project.py pack` | Packs the build of each part for a release. |
| `python project.py test` | Runs the tests. |
| `python project.py test --break` | Runs the tests with their claims made false. Each test must fail. |
| `python project.py test --branches` | Runs the tests on a build with probes. Each branch of the code must run. |
| `python project.py test --leaks` | Runs the tests on a build with counts. Each test must give back what the code takes. |
| `python project.py check` | Checks the project. |
| `python project.py clean` | Removes what the build made. Keeps what `fetch` downloaded. |
| `python project.py distclean` | Removes the folder `build`, also what `fetch` downloaded. |

With `--part <part>`, the commands `build`, `pack` and `test` act on that part, and on the parts that it uses.

## Parts

| Part | What it does | Zip | Tag |
| --- | --- | --- | --- |
| `wgpu_native` | Gives the wgpu-native library `v29.0.1.1` of gfx-rs, its headers, `leaks.toml` and `gaps.txt`: *wgpu_native*. | `BL-WGPU-Native.zip` | `wgpu_native-v<version>` |
| `contract` | Gives the C types of the window and of the frame: `docs/extern/contract/webgpu_contract.h.md`. | `BL-WebGPU-Contract.zip` | `contract-v<version>` |
| `device` | Makes the WebGPU device and the surface of a window, and writes each frame: `docs/extern/device/webgpu_device.h.md`. | `BM-WebGPU-Device.zip` | `device-v<version>` |
| `mesh` | Draws the views, the meshes, the textures, the draws and the batches of an app: `docs/extern/mesh/webgpu_mesh.h.md`. | `BM-WebGPU-Mesh.zip` | `mesh-v<version>` |

A release holds the zip, with a folder for each target, and `sha256.txt`.
A project takes it with one `[[source]]` that states `top = "{target}"`: `.toolchain/docs/project.toml.md`, *[[source]]*.

### wgpu_native

The library is `wgpu_native.dll` on Windows, `libwgpu_native.so` on Linux and `libwgpu_native.dylib` on macOS. Its headers are in `include/webgpu`.
`docs/intern/wgpu_native/leaks.toml.md` and `docs/intern/wgpu_native/gaps.txt.md` tell the two lists.
On `aarch64-windows-gnu`, the build of gfx-rs for MSVC needs `vcruntime140.dll`: install the Microsoft Visual C++ Redistributable for arm64.
The zip holds no file of Microsoft, and no import library, since zig links `wgpu_native.dll` itself.
On each target, the cases check that the library needs only the libraries of the system and the need above, and gives an adapter.

A deployment holds one copy of the library beside the WebGPU modules, and `content.ignore` names it.
A module links the library by its bare name, and its rpath is its own folder. Thus the loader finds the copy beside the module.
In this project, the kind `wgpu_module` of `.toolchain/config/kinds.toml` makes the modules. It links the library that `wgpu_native` puts in the scratch folder.

### The modules

The modules of `device` and `mesh` run on the backend of BB-Native, which keeps no limit. Thus each limit and each timeout of a module is `"none"`.
A deployment gives their keys and their settings as https://github.com/Encrid/Bridge-Host/blob/v0.2.0/readme.md tells. A key that no file states takes the default of the manifest.
Each enum of the settings is of `uint32_t`, and a deployment states a constant by its name or by its value.
The header `bridge.h` of Bridge tells the room that `spans` needs in a world with many tables.

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
wgpu-native is under the MIT licence and the Apache licence 2.0, at the choice of the user: `third_party/wgpu_native/license` holds both.
