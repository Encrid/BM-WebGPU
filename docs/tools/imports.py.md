# tools/imports.py

`imports.py` checks that a library of Windows needs only the libraries of the system.
A case of the project runs it on `wgpu_native.dll` of each target of Windows.

```sh
python tools/imports.py <library>
```

## The steps

1. It reads the PE header of the file, and finds its table of imports.
2. It reads the name of each library that the table names.
3. It compares each name, in lower case, with the list of the libraries of the system.
4. It writes a line for each other name, and gives 1.
5. Else it writes the count of the names, and gives 0.

The lines:

- `imports: <library>: <name> is not a library of the system`
- `imports: <library>: <count> libraries of the system`
- `imports: <library>: no PE file`

## The libraries of the system

The list holds the libraries that each Windows of the targets gives, for example `kernel32.dll`, `user32.dll`, `ntdll.dll`, `d3d12.dll` and `dxgi.dll`.
It also holds each name that starts with `api-ms-win-`, which Windows maps to its own libraries.
A library of the runtime of MSVC, for example `vcruntime140.dll`, is not on the list, because a machine without it cannot load the library.

## Functions

| Function | Result |
| --- | --- |
| `read_imports(path)` | Gives the names of the libraries that the PE file imports, or `None` if the file is no PE file. |
| `is_system(name)` | Tells if a name is on the list of the libraries of the system. |
| `main()` | Checks the library of the command line, writes the lines, and gives 0 or 1. |
