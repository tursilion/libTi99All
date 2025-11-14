@ECHO OFF
@rem https://github.com/desvandev/faster-vcvars

SETLOCAL EnableDelayedExpansion

SET _target_build=
IF [%1] == [32] SET _target_build=x86
IF [%1] == [64] SET _target_build=x64
IF [!_target_build!] == [] (
		ECHO Error: Target build must be either 32 or 64
		EXIT
	)

SET "_path_vs=C:\Program Files (x86)\Microsoft Visual Studio\"
SET "_path_winkit=C:\Program Files (x86)\Windows Kits\10\"

SET _latest_vs=
SET _latest_msvc=
SET _latest_winkit=

SET _lib_msvc_lib=
SET _lib_ucrt=
SET _lib_um=

SET _libpath_metadata=
SET _libpath_references=

SET _include_path=
SET _include_msvc_path=

SET _new_paths=
SET _new_lib=
SET _new_libpath=
SET _new_include=



FOR /f "tokens=*" %%G IN ('DIR /b /a:d /o:n "!_path_vs!" 2^>nul ^| FINDSTR /R /C:"^20[1-2][0-9]$"') DO (
		SET "_latest_vs=!_path_vs!%%G\"
	)

FOR /f "tokens=*" %%I IN ('DIR /b /a:d /o:n "!_latest_vs!Community\VC\Tools\MSVC\"') DO (
		SET "_latest_msvc=!_latest_vs!Community\VC\Tools\MSVC\%%I\bin\HostX64\!_target_build!"
		SET "_lib_msvc_lib=!_latest_vs!Community\VC\Tools\MSVC\%%I\lib\!_target_build!"
		SET "_include_msvc_path=!_latest_vs!Community\VC\Tools\MSVC\%%I\include"
	)

FOR /f "tokens=*" %%H IN ('DIR /b /a:d /o:n "!_path_winkit!bin\" 2^>nul ^| FINDSTR /R /C:"^[0-9][0-9][.].*"') DO (
		SET "_latest_winkit=!_path_winkit!bin\%%H\x64"
		SET "_lib_ucrt=!_path_winkit!lib\%%H\ucrt\!_target_build!"
		SET "_lib_um=!_path_winkit!lib\%%H\um\!_target_build!"
		SET "_libpath_metadata=!_path_winkit!UnionMetadata\%%H"
		SET "_libpath_references=!_path_winkit!References\%%H"
		SET "_include_path=!_path_winkit!include\%%H"
	)


:: merging variable Path
SET "_new_paths=!_latest_msvc!;"
SET "_new_paths=!_new_paths!!_latest_winkit!;"
SET "_new_paths=!_new_paths!!_path_winkit!bin\x64;"
SET "_new_paths=!_new_paths!!_latest_vs!Community\\MSBuild\Current\Bin;"
SET "Path=!_new_paths!%Path%"

:: merging variable LIB
SET "_new_lib=!_lib_msvc_lib!;"
SET "_new_lib=!_new_lib!!_lib_ucrt!;"
SET "_new_lib=!_new_lib!!_lib_um!;"
SET "LIB=!_new_lib!%LIB%"

:: merging variable LIBPATH
SET "_new_libpath=!_lib_msvc_lib!;"
SET "_new_libpath=!_new_libpath!!_lib_msvc_lib!\store\references;"
SET "_new_libpath=!_new_libpath!!_libpath_metadata!;"
SET "_new_libpath=!_new_libpath!!_libpath_references!;"
SET "LIBPATH=!_new_libpath!%LIBPATH%"

:: merging variable INCLUDE
SET "_new_include=!_include_msvc_path!;"
SET "_new_include=!_new_include!!_include_path!\ucrt;"
SET "_new_include=!_new_include!!_include_path!\shared;"
SET "_new_include=!_new_include!!_include_path!\um;"
SET "_new_include=!_new_include!!_include_path!\winrt;"
SET "_new_include=!_new_include!!_include_path!\cppwinrt;"
SET "INCLUDE=!_new_include!%INCLUDE%"

ENDLOCAL & SET Path=%Path% & SET LIB=%LIB% & SET LIBPATH=%LIBPATH% & SET INCLUDE=%INCLUDE%

EXIT /B
