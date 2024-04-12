# JetBrains.Timeline.Profiler.Api [![official JetBrains project](https://jb.gg/badges/official.svg)](https://confluence.jetbrains.com/display/ALL/JetBrains+on+GitHub)

**1. Example of using the API in native code.**
   
The library is header-only. Just include all needed headers.

```
#include "../include/etw_provider_api.hpp" // the API itself
#include "../include/debug_output.hpp"     // Debug Output provider
#include <iostream>
#include <string>

// callback which will be called when ETW provider is enabled
void on_provider_enable(bool is_enabled, uint64_t match_any_keyword, uint64_t match_all_keyword, void *) {
    std::cout << "Provider enable callback: enabled: " << std::boolalpha << is_enabled << ", match_any_keyword: " << match_any_keyword << ", match_all_keyword: " << match_all_keyword << "\n";
};

int main()
{
    using namespace events;
    using namespace events::etw;

    uint64_t handle = 0;

    try {
        std::cout << "Register provider...\n";
        // register Debug Output provider
        handle = register_provider<on_provider_enable>(JetBrains_Common_DebugOutput::guid);
        std::cout << "Provider is registered\n";
        std::cout << "Write string and then Enter to send it to DebugOutput:\n";

        using dbg_output = JetBrains_Common_DebugOutput::DebugOutput;

        for(std::string str; std::getline(std::cin, str); )
            // write Debug Output event with params (uint32_t ProcessId, const char * DebugString) - see the format in debug_output.hpp
            write_event(handle, { dbg_output::id, dbg_output::version }, dbg_output::data::to_tuple({ 1, str.c_str() }));
    }
    catch(const std::exception & ex) {
        std::cout << "Error occurred: " << ex.what() << "\n\n";
    }

    if(handle != 0)
        unregister_provider(handle);

    return 0;
}
```

**2. Example of using the API in managed code.**

1) First, build the native API library from `profiler-timeline-api\native\CMakeLists.txt` project. As the result you'll get `profiler_timeline_api.dll`.
2) Build the managed library from `profiler-timeline-api\managed\JetBrains.Timeline.Profiler.Api\Src\JetBrains.Timeline.Profiler.Api.csproj` project. You'll get `JetBrains.Timeline.Profiler.Api.dll`.
3) Make a test C# project and reference the assembly `JetBrains.Timeline.Profiler.Api.dll` from the previous step.
```
using System.ComponentModel;

try
{
    // IMPORTANT!: provide the path to profiler_timeline_api.dll which was built on the step 1.
    var prof = new JetBrains.Timeline.Profiler.Api.TimelineProfiler(
        @"G:\repos\profiler-timeline-api\native\cmake-build-debug",
        (enabled, keyword, allKeyword) => Console.WriteLine($"Provider enabled: {enabled}, {keyword}, {allKeyword}"));

    while (true)
    {
        string str = Console.ReadLine() ?? "Empty string";
        // write Debug Output event with the only string parameter
        prof.WriteDebugOutput(str);
    }
}
catch (Win32Exception error)
{
    Console.WriteLine($"Error occurred: {error.Message}, code: {error.NativeErrorCode}");
}
catch (Exception error)
{
    Console.WriteLine("Error occurred: " + error.Message);
}
```
