using System;
using System.IO;
using System.Runtime.InteropServices;

namespace JetBrains.Timeline.Profiler.Api.Impl;

internal sealed class TimelineProfilerInterop : IDisposable
{
  public sealed class NativeDelegate : IDisposable
  {
    private GCHandle myHandle;
    private readonly ulong myPointer;
    private bool myDisposed;

    public NativeDelegate(Delegate del)
    {
      myHandle = GCHandle.Alloc(del);
      myPointer = (ulong)Marshal.GetFunctionPointerForDelegate(del).ToInt64();
    }

    public ulong GetPointer() { return myPointer; }

    public void Dispose()
    {
      if (myDisposed)
        return;
      myHandle.Free();
      myDisposed = true;
      GC.SuppressFinalize(this);
    }

    ~NativeDelegate() => Dispose();
  }

  [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
  public delegate ulong RegisterProviderDelegate(ulong onEnableCallback, ref IntPtr errorMessage);

  [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
  public delegate void UnregisterProviderDelegate(ulong providerHandle, ref IntPtr errorMessage);

  [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
  public delegate void WriteDebugOutputDelegate(ulong providerHandle, [MarshalAs(UnmanagedType.LPStr)] string str, ref IntPtr errorMessage);

  [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
  public delegate void OnProviderEnabled(int isEnabled, ulong matchAnyKeyword, ulong matchAllKeyword);

  [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
  private delegate void ReleaseStringDelegate(IntPtr str);

  private readonly NativeLibrary myLibrary;
  private readonly ReleaseStringDelegate myReleaseString;

  public readonly RegisterProviderDelegate RegisterProvider;
  public readonly UnregisterProviderDelegate UnregisterProvider;
  public readonly WriteDebugOutputDelegate WriteDebugOutput;

  public TimelineProfilerInterop(string binDir)
  {
    myLibrary = new NativeLibrary(Path.Combine(binDir,"profiler_timeline_api.dll"));

    RegisterProvider = myLibrary.GetNativeFunction<RegisterProviderDelegate>("RegisterProvider");
    UnregisterProvider = myLibrary.GetNativeFunction<UnregisterProviderDelegate>("UnregisterProvider");
    WriteDebugOutput = myLibrary.GetNativeFunction<WriteDebugOutputDelegate>("WriteDebugOutput");
    myReleaseString = myLibrary.GetNativeFunction<ReleaseStringDelegate>("ReleaseString");
  }

  public void CheckError(IntPtr errorMessage)
  {
    if (errorMessage == IntPtr.Zero) return;
    var errorStr = Marshal.PtrToStringAnsi(errorMessage);
    myReleaseString(errorMessage);
    throw new ApplicationException(errorStr);
  }

  public void Dispose()
  {
    myLibrary.Dispose();
    GC.SuppressFinalize(this);
  }
}
