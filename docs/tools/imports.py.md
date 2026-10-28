# tools/imports.py

`imports.py` checks the imports of a library of Windows. Each one must be a library of the system, or a need that `readme.md` states for that target.
The cases of the project run it on the `wgpu_native.dll` of each target of Windows, on the machine of each target. Thus each target fetches the two zips of Windows for the cases.

```sh
python tools/imports.py <library> [<needed> ...]
```

Each `<needed>` is the name of a library that `readme.md` states as a need of `<library>` on its target, for example `vcruntime140.dll` on `aarch64-windows-gnu`.
A name `<needed>` counts in lower case. It is not a library of the system.
With no argument, the tool writes `usage: python tools/imports.py <library> [<needed> ...]`, and gives 2.

## The steps

1. It reads the PE header of the file, and finds its table of imports and its table of delayed imports.
2. It reads the name of each library that the two tables name, in lower case. A name that comes again does not count again.
3. It compares each name with the list of the libraries of the system, and with each name `<needed>`.
4. It writes a line for each name that it does not find, in the order of the tables. It gives 1.
5. Else it writes a line for each name `<needed>` that the tables name. Then it writes the count of the names of the system, and gives 0.

The lines:

- `imports: <library>: <name> is not a library of the system`
- `imports: <library>: <name> is a stated need`
- `imports: <library>: <count> libraries of the system`
- `imports: <library>: no PE file`

A path that names no file, or a file that is no PE file, gives the line `no PE file`, and 1.

## The libraries of the system

The list holds the libraries that each Windows of the targets gives, for example `kernel32.dll`, `user32.dll`, `ntdll.dll`, `d3d12.dll` and `dxgi.dll`.
It also holds each name that starts with `api-ms-win-`, which Windows maps to its own libraries.
A library of the runtime of MSVC, for example `vcruntime140.dll`, is not on the list, because a machine without it cannot load the library.
Thus the case of `aarch64-windows-gnu` names `vcruntime140.dll` as `<needed>`.

## Functions

| Function | Result |
| --- | --- |
| `read_imports(path)` | Gives the names that the two tables of the PE file hold, in lower case and each one time. Gives `None` for a path that names no PE file. |
| `is_system(name)` | Tells if a name is on the list of the libraries of the system. |
| `main()` | Checks the library of the command line with its names `<needed>`, writes the lines, and gives 0 or 1. Gives 2 with no argument. |
