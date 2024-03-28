using System;
using System.Runtime.InteropServices;

namespace JetBrains.Timeline.Profiler.Api.Impl;

internal sealed class NativeLibrary : IDisposable
{
  private IntPtr myHandle;

  public NativeLibrary(string libraryPath)
  {
    myHandle = Kernel32Dll.LoadLibraryW(libraryPath);
    if (myHandle == IntPtr.Zero)
      throw new DllNotFoundException("Failed to load shared library " + libraryPath);
  }

  public TDelegate GetNativeFunction<TDelegate>(string functionName) where TDelegate : Delegate
  {
    var ptr = Kernel32Dll.GetProcAddress(myHandle, functionName);
    if (ptr == IntPtr.Zero)
      throw new EntryPointNotFoundException("Failed to get a function entry point " + functionName);
#pragma warning disable CS0618
    return (TDelegate)Marshal.GetDelegateForFunctionPointer(ptr, typeof(TDelegate));
#pragma warning restore CS0618
  }

  public void Dispose()
  {
    if (myHandle == IntPtr.Zero)
      return;
    Kernel32Dll.FreeLibrary(myHandle);
    myHandle = IntPtr.Zero;
    GC.SuppressFinalize(this);
  }

  ~NativeLibrary() => Dispose();
}
