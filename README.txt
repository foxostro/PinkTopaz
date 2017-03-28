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

1. The Conan package for EntityX builds with iterator debug settings that conflict with the PinkTopaz project:

1>entityx.lib(Event.obj) : error LNK2038: mismatch detected for '_ITERATOR_DEBUG_LEVEL': value '0' doesn't match value '2' in VoxelData.obj 1>entityx.lib(Event.obj) : error LNK2038: mismatch detected for 'RuntimeLibrary': value 'MD_DynamicRelease' doesn't match value 'MDd_DynamicDebug' in VoxelData.obj

2. lasote's libboost builds with a RuntimeLibrary setting that conflicts with PinkTopaz when building a Debug build:

2>libboost_test_exec_monitor-vc140-mt-1_60.lib(test_tools.obj) : error LNK2038: mismatch detected for 'RuntimeLibrary': value 'MD_DynamicRelease' doesn't match value 'MDd_DynamicDebug' in VoxelData.obj


3. PinkTopaz builds as a library?

2>     Creating library C:/Users/Andrew/Documents/PinkTopaz/lib/PinkTopaz.lib and object C:/Users/Andrew/Documents/PinkTopaz/lib/PinkTopaz.exp
2>LINK : warning LNK4098: defaultlib 'MSVCRT' conflicts with use of other libs; use /NODEFAULTLIB:library

4. PinkTopaz fails to link due to unresolved test_main() symbol. For whatever reason...

2>libboost_test_exec_monitor-vc140-mt-1_60.lib(test_main.obj) : error LNK2019: unresolved external symbol "int __cdecl test_main(int,char * * const)" (?test_main@@YAHHQEAPEAD@Z) referenced in function "public: void __cdecl test_main_caller::operator()(void)" (??Rtest_main_caller@@QEAAXXZ)
2>C:\Users\Andrew\Documents\PinkTopaz\bin\PinkTopaz.exe : fatal error LNK1120: 1 unresolved externals
