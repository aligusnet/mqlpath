# MQL Path language

Experimenting with MQL path semantics.

## Using conan for dependencies.

### Install conan

```sh
> python -m pip install conan
```

### Configure project

```sh
> conan profile detect  # in case if no conan profiles created
> conan install . --output-folder=build --build=missing --settings=compiler.cppstd=20
> cd build
> source conanbuild.sh
> cd ..
> code .
```

Use Conan's CMake presets in VS Code.