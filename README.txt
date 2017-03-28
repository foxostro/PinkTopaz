PinkTopaz (working title)

Cross-platform voxel engine. Some day, this might eventually become a video game.

Building Pink Topaz
===================

1. Install conan:

    % pip install conan

2. Use conan to install dependencies:

    % conan install . --build missing

4. Configure the CMake project:

    * macOS

        % cmake -GXcode .

    * Windows

        % cmake -G "Visual Studio 14 2015 Win64" .

    * Linux

        % cmake -G "Unix Makefiles" .

5. Build it:

    % cmake --build . --target PinkTopaz
