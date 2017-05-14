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

Known Issues With The Windows Build
===================================

1. PinkTopaz builds as a library?

2>     Creating library C:/Users/Andrew/Documents/PinkTopaz/lib/PinkTopaz.lib and object C:/Users/Andrew/Documents/PinkTopaz/lib/PinkTopaz.exp
2>LINK : warning LNK4098: defaultlib 'MSVCRT' conflicts with use of other libs; use /NODEFAULTLIB:library

