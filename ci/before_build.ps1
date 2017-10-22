Set-PSDebug -Trace 1

function CheckLastExitCode {
    param ([int[]]$SuccessCodes = @(0), [scriptblock]$CleanupScript=$null)

    Push-AppveyorArtifact "$LogFile"
    Push-AppveyorArtifact "C:/projects/libossia/build/CMakeFiles/CMakeOutput.log"
    Push-AppveyorArtifact "C:/projects/libossia/build/CMakeFiles/CMakeError.log"

    if ($SuccessCodes -notcontains $LastExitCode) {
        if ($CleanupScript) {
            "Executing cleanup script: $CleanupScript"
            &$CleanupScript
        }
        $msg = @"
EXE RETURNED EXIT CODE $LastExitCode
CALLSTACK:$(Get-PSCallStack | Out-String)
"@
        throw $msg
    }
}

cd  C:\projects\libossia
git submodule update --init --recursive

if ( $env:APPVEYOR_BUILD_TYPE -eq "max" ){
  appveyor DownloadFile https://cycling74.s3.amazonaws.com/download/max-sdk-7.3.3.zip
  7z x max-sdk-7.3.3.zip -y
}

mkdir build
cd build

if ( $env:APPVEYOR_BUILD_TYPE -eq "testing" ){
  if ( Test-Path ${env:QTDIR}\bin\ ) {
    set $env:PATH=${env:QTDIR}\bin;${env:PATH};
  }

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\config-${env:APPVEYOR_BUILD_TYPE}-${env:configuration}.log"
  if ( $env:configuration -eq "Release" ){
    cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_BUILD_TYPE=Release -DOSSIA_PD=0 -DOSSIA_CI=1 -DOSSIA_TESTING=1 -DBOOST_ROOT="${env:BOOST_ROOT}" -DCMAKE_PREFIX_PATH="${env:QTDIR}\lib\cmake\Qt5" c:\projects\libossia > $LogFile
  } else {
    cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_BUILD_TYPE=Debug   -DOSSIA_PD=0 -DOSSIA_CI=1 -DOSSIA_TESTING=1 -DBOOST_ROOT="${env:BOOST_ROOT}" -DCMAKE_PREFIX_PATH="${env:QTDIR}\lib\cmake\Qt5" c:\projects\libossia > $LogFile
  }
  CheckLastExitCode

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "Release" ){
  if ( Test-Path ${env:QTDIR}\bin\ ) {
    set $env:PATH=${env:QTDIR}\bin;${env:PATH};
  }

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\config-${env:APPVEYOR_BUILD_TYPE}-win64.log"
  cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_INSTALL_PREFIX="${env:APPVEYOR_BUILD_FOLDER}/install" -DCMAKE_BUILD_TYPE=Release -DOSSIA_PD=0 -DOSSIA_CI=1 -DOSSIA_C=1 -DOSSIA_CPP=1 -DOSSIA_UNITY3D=0 -DOSSIA_TESTING=0 -DBOOST_ROOT="${env:BOOST_ROOT}" -DCMAKE_PREFIX_PATH="${env:QTDIR}\lib\cmake\Qt5" c:\projects\libossia > $LogFile
  CheckLastExitCode

  # now configure 32 bit version
  cd ..
  mkdir build-32bit
  cd build-32bit

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\config-${env:APPVEYOR_BUILD_TYPE}-win32.log"
  cmake -G "Visual Studio 15 2017"       -DCMAKE_INSTALL_PREFIX="${env:APPVEYOR_BUILD_FOLDER}/install-32bit" -DCMAKE_BUILD_TYPE=Release -DOSSIA_PD=0 -DOSSIA_CI=1 -DOSSIA_C=1 -DOSSIA_CPP=1 -DOSSIA_UNITY3D=0 -DOSSIA_TESTING=0 -DBOOST_ROOT="${env:BOOST_ROOT}" -DCMAKE_PREFIX_PATH="${env:QTDIR-32bit}\lib\cmake\Qt5" c:\projects\libossia > $LogFile
  CheckLastExitCode

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "Unity3d" ){
  if ( Test-Path ${env:QTDIR}\bin\ ) {
    set $env:PATH=${env:QTDIR}\bin;${env:PATH};
  }

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\config-${env:APPVEYOR_BUILD_TYPE}-win64.log"
  cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_INSTALL_PREFIX="${env:APPVEYOR_BUILD_FOLDER}" -DCMAKE_BUILD_TYPE=Release -DOSSIA_PD=0 -DOSSIA_CI=1 -DOSSIA_C=1 -DOSSIA_CPP=1 -DOSSIA_UNITY3D=1 -DOSSIA_TESTING=0 -DBOOST_ROOT="${env:BOOST_ROOT}" -DCMAKE_PREFIX_PATH="${env:QTDIR}\lib\cmake\Qt5" c:\projects\libossia > $LogFile
  CheckLastExitCode

  # now configure 32 bit version
  cd ..
  mkdir build-32bit
  cd build-32bit

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\config-${env:APPVEYOR_BUILD_TYPE}-win32.log"
  cmake -G "Visual Studio 15 2017"       -DCMAKE_INSTALL_PREFIX="${env:APPVEYOR_BUILD_FOLDER}" -DCMAKE_BUILD_TYPE=Release -DOSSIA_PD=0 -DOSSIA_CI=1 -DOSSIA_C=1 -DOSSIA_CPP=1 -DOSSIA_UNITY3D=1 -DOSSIA_TESTING=0 -DBOOST_ROOT="${env:BOOST_ROOT}" -DCMAKE_PREFIX_PATH="${env:QTDIR-32bit}\lib\cmake\Qt5" c:\projects\libossia > $LogFile
  CheckLastExitCode

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "max" ){
  $LogFile = c:\projects\libossia\configure-max.log
  cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_BUILD_TYPE=Release -DOSSIA_MAX=1 -DMAXSDK_MAINPATH="${env:APPVEYOR_BUILD_FOLDER}\max-sdk-7.3.3\source" -DOSSIA_PD=0 -DCMAKE_INSTALL_PREFIX="${env:APPVEYOR_BUILD_FOLDER}/install" -DOSSIA_STATIC=1 -DOSSIA_QT=0 -DOSSIA_NO_QT=1 -DOSSIA_EXAMPLES=0 -DOSSIA_CI=1 -DOSSIA_TESTING=0 -DBOOST_ROOT="${env:BOOST_ROOT}" c:\projects\libossia > $LogFile
  CheckLastExitCode

  # now configure 32 bit version
  cd ..
  mkdir build-32bit
  cd build-32bit

  $LogFile = "c:\projects\libossia\configure-max-32bit.log"
  cmake -G "Visual Studio 15 2017" -DCMAKE_BUILD_TYPE=Release -DOSSIA_MAX=1 -DMAXSDK_MAINPATH="${env:APPVEYOR_BUILD_FOLDER}\max-sdk-7.3.3\source" -DOSSIA_PD=0 -DCMAKE_INSTALL_PREFIX="${env:APPVEYOR_BUILD_FOLDER}/install" -DOSSIA_STATIC=1 -DOSSIA_QT=0 -DOSSIA_NO_QT=1 -DOSSIA_EXAMPLES=0 -DOSSIA_CI=1 -DOSSIA_TESTING=0 -DBOOST_ROOT="${env:BOOST_ROOT}" c:\projects\libossia > $LogFile
  CheckLastExitCode

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "pd" ){
  $LogFile = "c:\projects\libossia\configure-pd.log"
  cmake -G "Visual Studio 15 2017" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS_RELEASE="/MT" -DCMAKE_C_FLAGS_DEBUG="/MTd" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="${env:APPVEYOR_BUILD_FOLDER}/install" -DOSSIA_STATIC=1 -DOSSIA_PD=1 -DOSSIA_QT=0 -DOSSIA_EXAMPLES=0 -DOSSIA_CI=1 -DOSSIA_TESTING=0 -DOSSIA_PYTHON=0 -DOSSIA_QML=0 -DBOOST_ROOT="${env:BOOST_ROOT}" c:\projects\libossia > $LogFile
  CheckLastExitCode

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "python" ){
  $LogFile = "c:\projects\libossia\configure-${env:APPVEYOR_BUILD_TYPE}-${env:platform}.log"

  if ( "${env:platform}" -eq "x64" ){

    C:\${env:python}-x64\Scripts\pip.exe install wheel
    C:\${env:python}-x64\Scripts\pip.exe install twine

    ${env:PYTHON_EXE}="C:\${env:python}-x64\python.exe"

    cmake -G "Visual Studio 15 2017 Win64" -DPYTHON_EXECUTABLE:FILEPATH=C:\${env:python}-x64\python.exe -DPYTHON_LIBRARY=C:\${env:python}-x64\lib${env:python}.a -DCMAKE_BUILD_TYPE=Release -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="${env:APPVEYOR_BUILD_FOLDER}/install" -DOSSIA_STATIC=1 -DOSSIA_PD=0 -DOSSIA_QT=0 -DOSSIA_EXAMPLES=0 -DOSSIA_CI=1 -DOSSIA_TESTING=0 -DOSSIA_PYTHON=1 -DOSSIA_QML=0 -DBOOST_ROOT="${env:BOOST_ROOT}" c:\projects\libossia > $LogFile
    CheckLastExitCode
  } else {

    C:\${env:python}\Scripts\pip.exe install wheel
    C:\${env:python}\Scripts\pip.exe install twine

    ${env:PYTHON_EXE}="C:\${env:python}\python.exe"

    cmake -G "Visual Studio 15 2017" -DPYTHON_EXECUTABLE:FILEPATH=C:\${env:python}\python.exe -DPYTHON_LIBRARY=C:\${env:python}\lib${env:python}.a -DCMAKE_BUILD_TYPE=Release -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="${env:APPVEYOR_BUILD_FOLDER}/install" -DOSSIA_STATIC=1 -DOSSIA_PD=0 -DOSSIA_QT=0 -DOSSIA_EXAMPLES=0 -DOSSIA_CI=1 -DOSSIA_TESTING=0 -DOSSIA_PYTHON=1 -DOSSIA_QML=0 -DBOOST_ROOT="${env:BOOST_ROOT}" c:\projects\libossia > $LogFile
    CheckLastExitCode
  }

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "qml" ){
  if ( Test-Path ${env:QTDIR}\bin\ ) {
    set $env:PATH=${env:QTDIR}\bin;${env:PATH};
  }

  $LogFile = "c:\projects\libossia\configure-${env:APPVEYOR_BUILD_TYPE}.log"
  cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_BUILD_TYPE=Release -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="${env:APPVEYOR_BUILD_FOLDER}/install" -DCMAKE_PREFIX_PATH="${env:QTDIR}\lib\cmake\Qt5"  -DOSSIA_STATIC=0 -DOSSIA_PD=0 -DOSSIA_QT=1 -DOSSIA_EXAMPLES=0 -DOSSIA_CI=1 -DOSSIA_TESTING=0 -DOSSIA_PYTHON=0 -DOSSIA_QML=1 -DBOOST_ROOT="${env:BOOST_ROOT}" c:\projects\libossia > $LogFile
  CheckLastExitCode
}
