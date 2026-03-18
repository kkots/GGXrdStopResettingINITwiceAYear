# GGXrdStopResettingINITwiceAYear

## Description

On Windows, in countries with daylight saving time, Guilty Gear Xrd Rev2 version 2211 will erase all changes done to the RED* INI configs in the `GUILTY GEAR Xrd -REVELATOR-\REDGame\Config` directory twice a year, when the clocks change. This patch is intended to fix that.

It needs to be applied only once in forever.

## How to use (Windows)

- Go to the [Releases](/releases/latest) section and download the GGXrdStopResettingINITwiceAYear.exe.
- Make sure the game is not running prior to attempting to patch it, or you will get an error.
- Launch GGXrdStopResettingINITwiceAYear.exe. Press Enter, a file browser dialog will appear. Use it to select the GuiltyGearXrd.exe file. You can find where it is using Steam - Library - right-click Xrd - Manage - Browse local files. The EXE is in Binaries\Win32 folder.
- The patcher will create a backup copy of the game's EXE, patch it, and update the IniVersion timestamps in the RED* INI files. If everything went well, your INI files should not get overwritten when you launch the game, and will never do so again.

## How to use (Linux)

- Go to the [Releases](/releases/latest) section and download the GGXrdStopResettingINITwiceAYear_linux.
- Give yourself permission to run it:

```bash
chmod u+x GGXrdStopResettingINITwiceAYear_linux
```

- Launch GGXrdStopResettingINITwiceAYear_linux:
```bash
./GGXrdStopResettingINITwiceAYear_linux
```

- Press Enter, the program will ask for a path to GuiltyGearXrd.exe. You can find where it is using Steam - Library - right-click Xrd - Manage - Browse local files. The EXE is in Binaries\Win32 folder. You can drag and drop the EXE onto the console window, and the path will get pasted. Press Enter after pasting the path.
- The patcher will create a backup copy of the game's EXE, patch it, and update the IniVersion timestamps in the RED* INI files. If everything went well, your INI files should not get overwritten when you launch the game, and will never do so again.

## What was causing the bug

Daylight saving. There is a bug in the MSVCR100.DLL that GuiltyGearXrd uses (you can find this DLL in its Binaries\Win32 folder), described here: <https://devblogs.microsoft.com/cppblog/c-runtime-crt-features-fixes-and-breaking-changes-in-visual-studio-14-ctp1/> (See `fstat and _utime`.) The exact function Unreal Engine 3 was using was MSVCR100.DLL::_wstat64i32, which is of the fstat family, so it had the bug. This bug is not present in the most up-to-date version of the MSVCR DLL (MSVCR140.DLL).

The way I solved it is by calling KERNEL32.DLL::GetFileTime instead. It does not mess with timezones at all prior to returning the time, at least on Windows 10 and on Wine. God knows how it behaved on older versions of Windows. I am too lazy to test.

Anyway, so Unreal Engine 3 reads the time, and then what? It compares the last-write time of the `GUILTY GEAR Xrd -REVELATOR-\Engine\Config\Base*.ini` and of the `GUILTY GEAR Xrd -REVELATOR-\REDGame\Config\Default*.ini` to the timestamps written in `GUILTY GEAR Xrd -REVELATOR-\REDGame\Config\RED*.ini` that are located in the `[IniVersion]` section. The `0` timestamp is for the Base, `1` is for Default. If there's only one timestamp in the RED ini, that means the Default ini had no `BasedOn` setting in its `[Configuration]` section, and that timestamps corresponds to the Default file. The timestamps are compared, and if they're different, it is assumed that is because either the Default, or Base, or both ini files have changed, and that triggers the update of the RED ini file from the Default and Base files (exact mechanics of how it combines the Default and Base aren't studied, but I think it reads Base first, then applies Default on top).

## Credits

Thanks to Worse Than You for finding and posting the initial report of the bug and highlighting its relation to daylight saving, and later helping come up with the only correct solution to solving it.

## How to build

There is an almost certain probability that this patcher will get flagged as a virus. In that case, you may want to build it yourself. Download Visual Studio 2022 Community Edition, optn the .sln file and go to Build - Build Solution. The GGXrdStopResettingINITwiceAYear.exe will appear in Debug, Release, x64/Debug or x64/Release, depending on the Build Configuration.

On Linux you need to use CMake, see CMakeLists.txt for details.
