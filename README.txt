PinkTopaz (working title)

Cross-platform voxel engine. Some day, this might eventually become a video game.

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

