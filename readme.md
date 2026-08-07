# Have Fun Screw Off

HFSO is rollback based netcode inspired by <a href="https://www.ggpo.net/">GGPO</a>.
The goal of this project is to make code + feature improvements to the original concepts, as well as making libraries and bindings available to multiple platforms and languages.  Long term we intend for this project to be used in all kinds of online game projects, not just fighting games!


## Build
We are using CMake to make the multiplatform development a bit easier to do.


### Windows
HFPO uses CMake, and was originally developed on Windows 10, and Visual Studio 2022.
Other versions of Windows + Visual Studio may work, but aren't explicitly supported at this time.

Use the following command on windows to generate the project/solution files for VS 2022.
```
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
```

### Linux
TBD