﻿using System.Runtime.InteropServices;
using JetBrains.Timeline.Profiler.Api.Impl;

namespace JetBrains.Timeline.Profiler.Api;

public sealed class TimelineProfiler : IDisposable
{
  private readonly string myBinDirectory;
  private readonly TimelineProfilerInterop myProfiler;
  private readonly TimelineProfilerInterop.NativeDelegate myProviderEnableDelegate;
  private readonly ulong myProviderHandle;
  private bool myDisposed = false;

  public delegate int OnProviderEnabled(bool isEnabled, ulong matchAnyKeyword, ulong matchAllKeyword);

  public TimelineProfiler(string binDirectory, OnProviderEnabled onProviderEnabled)
  {
    myBinDirectory = binDirectory;
    myProfiler = new TimelineProfilerInterop(myBinDirectory);

    myProviderEnableDelegate = new TimelineProfilerInterop.NativeDelegate(onProviderEnabled);
    var errorMessage = IntPtr.Zero;
    myProviderHandle = myProfiler.RegisterProvider(myProviderEnableDelegate.GetPointer(), ref errorMessage);
    myProfiler.CheckError(errorMessage);
  }

  public void WriteDebugOutput(string str)
  {
    var errorMessage = IntPtr.Zero;
    myProfiler.WriteDebugOutput(myProviderHandle, str, ref errorMessage);
    myProfiler.CheckError(errorMessage);
  }

  public void Dispose()
  {
    Dispose(true);
    GC.SuppressFinalize(this);
  }

  private void Dispose(bool disposing)
  {
    if (myDisposed)
      return;
    var errorMessage = IntPtr.Zero;
    myProfiler.UnregisterProvider(myProviderHandle, ref errorMessage);
    myProfiler.CheckError(errorMessage);
    if (disposing)
    {
      myProfiler.Dispose();
      myProviderEnableDelegate.Dispose();
    }
    myDisposed = true;
  }

  ~TimelineProfiler() => Dispose(false);
}
