@echo off
SET TMP_SDKDRIVE=N

@echo Visual C++ 7.1 setup.
@call "%TMP_SDKDRIVE%:\Microsoft Visual Studio .NET 2003\env-drv.cmd" %TMP_SDKDRIVE%

@echo Platform SDK setup.
@call "%TMP_SDKDRIVE%:\Microsoft Platform SDK for Windows XP SP2\Env-drv.Bat" %TMP_SDKDRIVE%

@echo WTL setup.
@call "%TMP_SDKDRIVE%:\wtl75_4291\Env.Bat" %TMP_SDKDRIVE%:


@echo Done.
@echo Done.
