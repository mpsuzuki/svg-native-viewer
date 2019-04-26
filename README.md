# Building Skia backend on GNU/Linux

For official document, see [README.official.md](/README.official.md).

* scope of this document
this document is a memo how to build Skia backend on GNU/Linux,
this is not a superset of official README.

* Background
in the original master branch of SVG Native Viewer, the backend
emitting graphical data is only Skia and CoreGraphics. Although
Skia itself is designed to be cross platform (macOS, Windows and
some Unix), CMakeLists.txt is tentatively specialized for macOS.
Although most GNU/Linux distributions do not provide prebuilt
binary package of Skia, it is not impossible to build Skia on
usual GNU/Linux. Thus, it is not impossible to build Skia backend
working on GNU/Linux. It would be helpful for the developers of
other backend to have an existing backend emitting graphical data.

* How to build Skia itself
CMakeLists.txt assumes the skia headers & libraries are located
in third_party. Once you executed "git clone https://github.com/adobe/svg-native-viewer"
there would be 2 subdirectories, svgnative and third_party.
Move to third_party, and execute the instruction on skia.org.

[how to download skia](https://skia.org/user/download)

The current Skia backend is written for m70 branch. After git
clone of Skia repository, and execute "python tools/git-sync-deps"
to download the third party libraries, you should checkout
origin/chrome/m70.

The build instruction is same with the later (at present,
the latest is a branch for m76),
[how to build skia](https://skia.org/user/build)

* Compiler

It is noted that SVG Native Viewer is ready to be compiled by
C++11 compiler, Skia is not (Skia maintainers strongly suggest
to use Clang). It seems that the header files of m70 is still
compatible C++11, so once you build Skia libraries, you are not
required to use same compiler to build SVG Native Viewer.
