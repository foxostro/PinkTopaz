PinkTopaz (working title)

Cross-platform voxel engine. Some day, this might eventually become a video game.


Building on macOS
=================

1. Install Xcode and command line tools.

2. Create a Mac Developer signing certificate and add a line to $HOME/.profile which exports an environment variable named MAC_DEVELOPER_IDENTITY. This variable should contain the name of this certificate. For example:

    export MAC_DEVELOPER_IDENTITY="Mac Developer: Andrew Fox (6854DN3267)"

This variable is used by the script "sign_the_app_bundle" to sign the app bundle when building on macOS.

2. Install Homebrew. See <https://brew.sh> for details.

3. Use Homebrew to install some build tools:
	
	% brew install cmake automake libtool nasm conan

4. Use conan to install dependencies:

    % conan remote add PinkTopaz https://api.bintray.com/conan/foxostro/PinkTopaz
    % conan install . --build missing

5. Configure the CMake project:

    % cmake -GXcode .

6. Build it:

    % cmake --build . --target PinkTopaz


Building on Windows
===================

1. Install Microsoft Visual Studio 2017.

2. Install Conan. See <https://www.conan.io/downloads> for details.

3. Use conan to install dependencies. Use the Developer Console:

    % conan remote add PinkTopaz https://api.bintray.com/conan/foxostro/PinkTopaz
    % conan install . --build=missing --settings compiler="Visual Studio" --settings compiler.version=15

4. Configure the CMake project:

    % cmake -G "Visual Studio 15 2017 Win64" .

5. Build it:

    % cmake --build . --target PinkTopaz
