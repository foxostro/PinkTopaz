PinkTopaz (working title)

This project is intended to be a simple third-person shooter video game.

Building Pink Topaz
===================

1. Install conan:

    % pip install conan

2. Use conan to install dependencies:

    % cd $PINK_TOPAZ_ROOT_DIR
    % conan install . --build=missing

3. Install googletest too. (Not presently configured in the conanfile.)

4. Actually build the project:

    % cmake -GXcode .
    % cmake --build . --target PinkTopaz

TODO: The conan.io package for entityx always builds as shared and that makes it slightly harder to package. The app will not run on a non-dev machine.
