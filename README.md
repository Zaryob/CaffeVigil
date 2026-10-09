# CaffeVigil

A small Windows desktop/tray utility that requests the system and display remain awake for **5, 10 or 30 minutes, 1 hour, or indefinitely**.

The current implementation is timer based. It does **not** watch another process or automatically stop when that process exits.

## Build from source

The checked-in project uses Visual Studio 2022's **v143 C++ toolset** and the **Windows 10 SDK** (`WindowsTargetPlatformVersion=10.0`). Install Visual Studio's Desktop development with C++ workload and open:

```sh
git clone https://github.com/Zaryob/CaffeVigil.git
```

Open `CaffeVigil.sln`, select Release and x64, then build. In a Visual Studio Developer Command Prompt:

```bat
msbuild CaffeVigil.sln /p:Configuration=Release /p:Platform=x64
```

The Windows SDK setting is a build input, not a tested minimum Windows version. A Windows build and supported-runtime matrix have not yet been recorded. No downloadable binary release was present in the public repository audit on 9 October 2026.

## Use

Run the executable and select a duration. Closing the window hides it to the tray; use the tray menu's **Exit** action to quit.

`TrayIconManager` calls `SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED)` to request wakefulness, and `RemoveAwakeInterval` resets the thread's request with `ES_CONTINUOUS`. The normal timer-expiry and Exit paths call that reset. These source paths still need Windows runtime verification.

Known release blockers from source review:

- The **1 Hour** handler passes 30 minutes to `SetAwakeInterval`, while its countdown uses 60 minutes.
- The indefinite countdown assigns `endTime=2`, while its display branch tests `endTime==1`.
- Timer replacement, sleep restoration and tray Exit need a Windows smoke test.

Before publishing a binary, verify all five durations, replacement of an existing timer, tray restore/Exit, normal sleep after expiration, and behavior under the machine's power policy. A screenshot or runtime result should come from the actual Windows app.

## License

[MIT](License.md). Original source and attribution are preserved.
