PinkTopaz (working title)

Cross-platform voxel engine. Some day, this might eventually become a video game.

Building Pink Topaz
===================

1. Install Xcode and command line tools.

2. Create a Mac Developer signing certificate and add a line to $HOME/.profile which exports an environment variable named MAC_DEVELOPER_IDENTITY. This variable should contain the name of this certificate. For example:

    export MAC_DEVELOPER_IDENTITY="Mac Developer: Andrew Fox (6854DN3267)"

This variable is used by the script "sign_the_app_bundle" to sign the app bundle when building on macOS.

2. Install Homebrew. See <https://brew.sh> for details.

3. Use Homebrew to install some build tools:
	
	% brew install cmake automake libtool nasm pip

4. Install conan:

    % pip install --user conan

5. Use conan to install dependencies:

	% conan remote add bintray https://api.bintray.com/conan/conan-community/conan
    % conan install . --build missing

6. Configure the CMake project:

    % cmake -GXcode .

7. Build it:

    % cmake --build . --target PinkTopaz
