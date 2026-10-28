# BL-WebGPU-Contract

The contract of the WebGPU modules of Bridge: the C types and the descriptors of webgpu:Window and webgpu:Frame.

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

Each release holds, for each target, the header `include/webgpu_contract.h` and the notice `license`.
After the zip, `pack` builds a program from the zip and the headers of Bridge alone. Thus a module can build against the release.
`docs/extern/webgpu_contract.h.md` tells the four types of the header, the roles of the modules and their grants.

BL-WebGPU-Contract holds no module. Each module or app in C that uses one of the four types fetches the release with a `[[source]]`:

```toml
[[source]]
name = "webgpu_contract"
url = "https://github.com/Encrid/BL-WebGPU-Contract/releases/download/v<version>/BL-WebGPU-Contract-{target}.zip"
sha256 = "{webgpu_contract_sha256}"
into = "build/third_party/webgpu_contract/{target}"
```

`<version>` is the number of the release, for example `0.1.0`.
Each `[targets.<target>]` of the module gives `webgpu_contract_sha256`: the digest of the zip of that target, in `sha256.txt` of the release.
The artifact of the module adds `{webgpu_contract}/include` to its `include`.
The module also fetches the release of Bridge, because the header includes `bridge_schema.h`.

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
