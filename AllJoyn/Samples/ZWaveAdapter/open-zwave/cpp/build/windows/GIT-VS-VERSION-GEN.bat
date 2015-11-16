@ECHO OFF
SETLOCAL

REM  Script for generation of rc VERSIONINFO & StringFileInfo

REM ====================
REM Installation Variables
REM ====================
:: VERSION_FILE - Untracked file to be included in packaged source releases.
::                it should contain a single line in the format:
::                $Project_Name VERSION $tag (ie: Foobar VERSION v1.0.0-alpha0)
SET VERSION_FILE=GIT-VS-VERSION-FILE

:: DEFAULT_VERSION - Version string to be processed when neither Git nor a
::                packed version file is available.
SET DEFAULT_VERSION=v1.3.0

:: COUNT_PATCHES_FROM - Determines which tag to count the number of patches from
::                for the final portion of the digital version number.
::                Valid values are:
::                   major - count from earliest Major.0.0* tag.
::                   minor - count from earliest Major.Minor.0* tag.
::                   fix   - count from earliest Major.Minor.Fix tag.
SET COUNT_PATCHES_FROM=fix

:: USES_PRERELEASE_TAGS - numeric bool value to determine if GET_GIT_PATCHES
::                function should read the number of patches in the format of
::                  (Default) 1 - Major.Minor.Fix-Stage#-'CommitCount'
::                            0 - Major.Minor.Fix-'CommitCount'
SET USE_PRERELEASE_TAGS=1

:: --------------------
:CHECK_ARGS
:: --------------------

:: Console output only.
IF [%1] == [] GOTO START

IF "%~1" == "--help" GOTO USAGE
IF "%~1" == "--quiet" SET fQUIET=1& SHIFT
IF "%~1" == "--force" SET fFORCE=1& SHIFT

:: Un-documented switch
IF "%~1" == "--test" GOTO TEST

IF EXIST %~1\NUL (
  :: %1 is a path
  SET CACHE_FILE=%~s1\%VERSION_FILE%
  SHIFT
)

IF [%~nx1] NEQ [] (
  :: %1 is a file
  SET HEADER_OUT_FILE=%~fs1
  SHIFT
)
:: This should always be the last argument.
IF [%1] NEQ [] GOTO USAGE

:: Some basic sanity checks.
IF DEFINED fQUIET (
  IF NOT DEFINED HEADER_OUT_FILE GOTO USAGE
)

IF DEFINED CACHE_FILE (
  SET CACHE_FILE=%CACHE_FILE:\\=\%
  IF NOT DEFINED HEADER_OUT_FILE GOTO USAGE
)
GOTO START

:: --------------------
:USAGE
:: --------------------
ECHO usage: [--help] ^| ^| [--quiet] [--force] [CACHE PATH] [OUT FILE]
ECHO.
ECHO  When called without arguments version information writes to console.
ECHO.
ECHO  --help      - displays this output.
ECHO.
ECHO  --quiet     - Suppress console output.
ECHO  --force     - Ignore cached version information.
ECHO  CACHE PATH  - Path for non-tracked file to store git-describe version.
ECHO  OUT FILE    - Path to writable file that is included in the project's rc file.
ECHO.
ECHO  Version information is expected to be in the format: vMajor.Minor.Fix[-stage#]
ECHO  Where -stage# is alpha, beta, or rc. ( example: v1.0.0-alpha0 )
ECHO.
ECHO  Example pre-build event:
ECHO  CALL $(SolutionDir)..\scripts\GIT-VS-VERSION-GEN.bat "$(IntDir)\" "$(SolutionDir)..\src\gen-versioninfo.h"
ECHO.
GOTO END


REM ===================
REM Entry Point
REM ===================
:START
ECHO.
CALL :INIT_VARS
CALL :GET_VERSION_STRING
IF DEFINED fGIT_AVAILABLE (
  IF DEFINED fLEAVE_NOW GOTO END
  IF DEFINED CACHE_FILE (
    CALL :CHECK_CACHE
  )
)
IF DEFINED fLEAVE_NOW GOTO END
CALL :SET_BUILD_PARTS
CALL :SET_FLAGS
CALL :PREP_OUT
CALL :WRITE_OUT
GOTO END

REM ====================
REM FUNCTIONS
REM ====================
:: --------------------
:INIT_VARS
:: --------------------
:: The following variables are used for the final version output.
::    String Version:  Major.Minor.Fix.Stage#[.Patches.SHA1[.dirty]]
SET strFILE_VERSION=

::    Digital Version: Major, Minor, Fix, Patches
SET nbMAJOR_PART=0
SET nbMINOR_PART=0
SET nbFIX_PART=0
SET nbPATCHES_PART=0

:: VERSIONINFO VS_FF_ flags
SET fPRIVATE=0
SET fPATCHED=0
SET fPRE_RELEASE=0

:: Supporting StringFileInfo - not used for clean release builds.
SET strPRIVATE_BUILD=
SET strCOMMENT=

GOTO :EOF

:: --------------------
:GET_VERSION_STRING
:: --------------------
:: Precedence is Git, VERSION_FILE, then DEFAULT_VERSION.
:: Check if git is available by testing git describe.
CALL git describe>NUL 2>&1
IF NOT ERRORLEVEL 1 (
  SET fGIT_AVAILABLE=1
  :: Parse git version string
  CALL :PARSE_GIT_STRING
) ELSE (
  :: Use the VERSION_FILE if it exists.
  IF EXIST "%VERSION_FILE%" (
    FOR /F "tokens=3" %%A IN (%VERSION_FILE%) DO (
      SET strFILE_VERSION=%%A
    )
  ) ELSE (
    :: Default to the DEFAULT_VERSION
    SET strFILE_VERSION=%DEFAULT_VERSION%
  )
)
SET strFILE_VERSION=%strFILE_VERSION:~1%
SET strFILE_VERSION=%strFILE_VERSION:-=.%
GOTO :EOF

:: --------------------
:PARSE_GIT_STRING
:: --------------------
FOR /F "tokens=*" %%A IN ('"git describe --abbrev=5 HEAD"') DO (
  SET strFILE_VERSION=%%A
)
:: If HEAD is dirty then this is not part of an official build and even if a
:: commit hasn't been made it should still be marked as dirty and patched.
SET tmp=
CALL git update-index -q --refresh >NUL 2>&1
IF ERRORLEVEL 1 (
  IF [%fFORCE%] EQU [1] (
    verify > nul
  ) ELSE (
    ECHO >> The working tree index is not prepared for build testing!
    ECHO >> Please check git status or use --force to ignore the index state.
    SET fLEAVE_NOW=1
  )
)
FOR /F %%A IN ('git diff-index --name-only HEAD --') DO SET tmp=%%A
IF NOT "%tmp%" == "" (
  SET strFILE_VERSION=%strFILE_VERSION%-dirty
)
SET tmp=
GOTO :EOF

:: --------------------
:CHECK_CACHE
:: --------------------
:: Exit early if a cached git built version matches the current version.
IF DEFINED HEADER_OUT_FILE (
  IF EXIST "%HEADER_OUT_FILE%" (
    IF [%fFORCE%] EQU [1] DEL "%CACHE_FILE%"
    IF EXIST "%CACHE_FILE%" (
      FOR /F "tokens=*" %%A IN (%CACHE_FILE%) DO (
        IF "%%A" == "%strFILE_VERSION%" (
          IF NOT DEFINED fQUIET (
            ECHO Build version is assumed unchanged from: %strFILE_VERSION%.
          )
          SET fLEAVE_NOW=1
        )
      )
    )
  )

  ECHO %strFILE_VERSION%> "%CACHE_FILE%"
)
GOTO :EOF

:: --------------------
:SET_BUILD_PARTS
:: --------------------
:: The min version is X.Y.Z and the max is X.Y.Z.Stage#.Commits.SHA.dirty
:: strTMP_STAGE_PART is a holder for anything past 'X.Y.Z.'.
FOR /F "tokens=1,2,3,* delims=." %%A IN ("%strFile_Version%") DO (
  SET nbMAJOR_PART=%%A
  SET nbMINOR_PART=%%B
  SET nbFIX_PART=%%C
  SET strTMP_STAGE_PART=%%D
)
CALL :SET_STAGE_PARTS
IF DEFINED fGIT_AVAILABLE CALL :GET_GIT_PATCHES
GOTO :EOF

:: --------------------
:SET_STAGE_PARTS
:: --------------------
SET nbSTAGE_VERSION=
SET tmp=%strTMP_STAGE_PART:~,1%
IF "%tmp%" == "a" (
  SET strCOMMENT=Alpha Release
  SET strSTAGE_PART=-alpha
  CALL :APPEND_STAGE_VERSION %strTMP_STAGE_PART:~5%
) ELSE (
  IF "%tmp%" == "b" (
    SET strCOMMENT=Beta Release
    SET strSTAGE_PART=-beta
    CALL :APPEND_STAGE_VERSION %strTMP_STAGE_PART:~4%
  ) ELSE (
    IF "%tmp%" == "r" (
      SET strCOMMENT=Release Candidate
      SET strSTAGE_PART=-rc
      CALL :APPEND_STAGE_VERSION %strTMP_STAGE_PART:~2%
    ) ELSE (
      SET strComment=Major Version Release
      SET strSTAGE_PART=
    )
  )
)
GOTO :EOF

:: --------------------
:APPEND_STAGE_VERSION
:: --------------------
:: [PARAM in] %1 - the numeric portion of strTMP_STAGE_PART and the remainder.
::                 ie: rc1.13.g28456 is passed in as 1.13.g28456
FOR /F "tokens=1 delims=." %%A IN ("%1") DO SET nbSTAGE_VERSION=%%A
SET strCOMMENT=%strCOMMENT% %nbSTAGE_VERSION%
IF NOT [%1] == [] SET fPRE_RELEASE=1
GOTO :EOF

:: --------------------
:GET_GIT_PATCHES
:: --------------------
:: Read in the description of the current commit in terms of the earliest
:: release stream tag.
IF "%COUNT_PATCHES_FROM%" == "major" (
  SET tmp=v%nbMAJOR_PART%.0.0*
) ELSE (
    IF "%COUNT_PATCHES_FROM%" == "minor" (
      SET tmp=v%nbMAJOR_PART%.%nbMINOR_PART%.0*
    ) ELSE (
      SET tmp=v%nbMAJOR_PART%.%nbMINOR_PART%.%nbFIX_PART%*
    )
)
SET git_cmd=git for-each-ref --count=1 --format=%%(refname:short)
SET git_cmd=%git_cmd% --sort=taggerdate refs/tags/%tmp%
FOR /F "tokens=* usebackq" %%A IN (`"%git_cmd%"`) DO SET tmp=%%A
SET git_cmd=

:: Full version releases have the Git patch count at the first '-' while
:: pre-release versions have it at the second.
IF [%USE_PRERELEASE_TAGS%] == [0] (
  FOR /F "tokens=2 delims=-" %%A IN ('"git describe --match %tmp%"') DO (
    SET nbPATCHES_PART=%%A
  )
) ELSE (
  FOR /F "tokens=3 delims=-" %%A IN ('"git describe --match %tmp%"') DO (
    SET nbPATCHES_PART=%%A
  )
)
IF NOT DEFINED nbPATCHES_PART SET nbPATCHES_PART=0
SET tmp=
GOTO :EOF

:: --------------------
:SET_FLAGS
:: --------------------
:: PATCHED indicates that the build is taking place at a non-tagged commit.
SET tmp=v%nbMAJOR_PART%.%nbMINOR_PART%.%nbFIX_PART%%strSTAGE_PART%%nbSTAGE_VERSION%
IF DEFINED fGIT_AVAILABLE (
  FOR /F "tokens=*" %%A IN ('"git describe HEAD"') DO SET tmp2=%%A
)
IF NOT "%tmp%" == "%tmp2%" SET fPATCHED=1

:: When HEAD is dirty the build is considered both PRIVATE and PATCHED
:: PRIVATE indicates that the HEAD is dirty, and the string Custom Build is used.
SET tmp3=%strFILE_VERSION:*dirty=dirty%
SET tmp3=%tmp3:~0,5%
IF "%tmp3%" == "dirty" (
  SET fPATCHED=1
  SET fPRIVATE=1
  SET strPRIVATE=Custom Build
)

:: Capture full version maintenance releases. (ie: v1.0.0.1)
SET tmp=v%nbMAJOR_PART%.%nbMINOR_PART%.%nbFIX_PART%.%strTMP_STAGE_PART%
IF [%fPATCHED%] == [1] (
  IF [%fPRIVATE%] == [0] (
    IF "%strCOMMENT:~0,5%" == "Major" (
      IF "%tmp%" == "%tmp2%" (
        SET strCOMMENT=%strCOMMENT:Major=Maintanence%
        SET fPATCHED=0
      ) ELSE (
        SET fPRIVATE=1
      )
    )
  )
)

:: The only flag not set here is the PRE_RELEASE flag.  It made more sense to
:: set it in the APPEND_STAGE_VERSION since only PRE_RELEASE stages pass that
:: function an argument.

SET tmp=
SET tmp2=
SET tmp3=
GOTO :EOF

:: --------------------
:PREP_OUT
:: --------------------
SET csvFILE_VERSION=%nbMAJOR_PART%,%nbMINOR_PART%,%nbFIX_PART%,%nbPATCHES_PART%
SET hexFILE_VERSION=
CALL :SET_HEX

IF NOT %fPRIVATE% EQU 0 SET fPRIVATE=VS_FF_PRIVATEBUILD
IF NOT %fPATCHED% EQU 0 SET fPATCHED=VS_FF_PATCHED
IF NOT %fPRE_RELEASE% EQU 0 SET fPRE_RELEASE=VS_FF_PRERELEASE
GOTO :EOF

:: --------------------
:SET_HEX
:: --------------------
:: Iterate Major, Minor, Fix, Patches as set in csvFILEVERSION and convert to
:: hex while appending to the hexFILE_VERION string to give a padded 32bit
:: end result. ie: v1.0.1.34 = 0x0001000000010022
SET hex_values=0123456789ABCDEF

FOR /F "tokens=1-4 delims=," %%A IN ("%csvFILE_VERSION%") DO (
  CALL :int2hex %%A
  CALL :int2hex %%B
  CALL :int2hex %%C
  CALL :int2hex %%D
)

SET hexFILE_VERSION=0x%hexFILE_VERSION%
SET hex_values=

GOTO :EOF

:int2hex
SETLOCAL ENABLEDELAYEDEXPANSION
SET /A pad=4
SET /A iVal=%1

:hex_loop
SET /A pad=%pad% - 1
SET /A hVal=%iVal% %% 16
SET hVal=!hex_values:~%hVal%,1!
SET hex_word=%hVal%%hex_word%
SET /A iVal=%iVal% / 16
IF %iVal% GTR 0 GOTO hex_loop

:hex_pad_loop
FOR /L %%A in (1,1,%pad%) DO SET hex_word=0!hex_word!
ENDLOCAL& SET hexFILE_VERSION=%hexFILE_VERSION%%hex_word%
GOTO :EOF

:: --------------------
:WRITE_OUT
:: --------------------
:: HEADER_OUT falls through to CON_OUT which checks for the QUIET flag.
IF DEFINED HEADER_OUT_FILE (
  CALL :OUT_HEADER
) ELSE (
  IF NOT DEFINED TESTING (
  CALL :CON_OUT
  ) ELSE (
    CALL :TEST_OUT
  )
)
GOTO :EOF

:: --------------------
:OUT_HEADER
:: --------------------
ECHO unsigned short ozw_vers_minor = %nbMAJOR_PART%; >> "%HEADER_OUT_FILE%"
ECHO unsigned short ozw_vers_major = %nbMINOR_PART%; >> "%HEADER_OUT_FILE%"
ECHO unsigned short ozw_vers_revision = %nbFIX_PART%; >> "%HEADER_OUT_FILE%"
ECHO char ozw_version_string[] = "%strFILE_VERSION%\0"; >> "%HEADER_OUT_FILE%"


SET nbMAJOR_PART=0
SET nbMINOR_PART=0
SET nbFIX_PART=0
SET nbPATCHES_PART=0

:: --------------------
:CON_OUT
:: --------------------
IF DEFINED fQUIET GOTO :EOF
ECHO Version String::      %strFILE_VERSION%
ECHO Digital Version ID:   %csvFILE_VERSION%
ECHO Hex Version ID:       %hexFILE_VERSION%
ECHO Comment:              %strCOMMENT%
ECHO Private Build String: %strPRIVATE%
ECHO Is Private Build:     %fPRIVATE%
ECHO Is Patched:           %fPATCHED%
ECHO Is PreRelease:        %fPRE_RELEASE%
GOTO :EOF

:: --------------------
:TEST
:: --------------------
:: Create the test directory & repo
SET TERM=
SET TESTING=1
SET git-vs-version-test-dir=git-vs-version-test
MKDIR %git-vs-version-test-dir%
PUSHD %git-vs-version-test-dir%
CALL git init >NUL 2>&1
IF ERRORLEVEL 1 (
  ECHO Test requires git.
  GOTO END
)

:: Generate the test patches and tags
SET test_stage=-alpha
:TEST_LOOP
FOR /L %%A IN (0,1,1) DO (
      SET test_ver=%test_stage%%%A
      IF "%test_stage%" == "" SET test_ver=
      CALL git commit --allow-empty -m "Commit v1.0.0%%test_ver%%" >NUL
      IF ERRORLEVEL 1 GOTO END
      CALL git tag -a v1.0.0%%test_ver%% -m "Test v1.0.0%%test_ver%%"
      IF ERRORLEVEL 1 GOTO END
  FOR /L %%B IN (0,1,2) DO (
      CALL git commit --allow-empty -m "Work on v1.0.0%%test_ver%%" >NUL
      IF ERRORLEVEL 1 GOTO END
  )
  IF "%test_stage%" == "" GOTO TEST_MAINT
)
IF "%test_stage%" == "-alpha" SET test_stage=-beta& GOTO TEST_LOOP
IF "%test_stage%" == "-beta" SET test_stage=-rc& GOTO TEST_LOOP
IF "%test_stage%" == "-rc" SET test_stage=& GOTO TEST_LOOP
SET test_stage=

:TEST_MAINT
:: Simulate a maint patch
CALL git commit --allow-empty -m "Maint Work on v1.0.0.1" >NUL
CALL git tag -a v1.0.0.1 -m "Test v1.0.0.1"

:: Simulate the first patch for v1.0.1
CALL git commit --allow-empty -m "Starting v1.0.1" >NUL
CALL git tag -a v1.0.1-alpha0 -m "Test v1.0.1-alpha0"

:: Simulate another maint patch
CALL git commit --allow-empty -m "Maint Work on v1.0.0.2" >NUL
CALL git tag -a v1.0.0.2 -m "Test v1.0.0.2"

:: Generate the output
ECHO TAG, Version, Hex, Maj, Min, Fix, Patches (from %COUNT_PATCHES_FROM%), PreRelease, Private, Patched, Comment

SET git_cmd=git for-each-ref --format=%%(refname:short) refs/tags/
FOR /F "tokens=* usebackq" %%A IN (`"%git_cmd%"`) DO (
  CALL git reset --hard %%A >NUL
  CALL :TEST_VERSION %%A
)

:: Builder checked out the parent of v1.0.0
CALL git reset --hard v1.0.0~1 >NUL
FOR /F "tokens=*" %%A IN ('"git describe HEAD"') DO SET tmp=%%A
CALL :TEST_VERSION %tmp%

:: Builder staged a file.
CALL touch README >NUL
CALL git add README
FOR /F "tokens=*" %%A IN ('"git describe HEAD"') DO SET tmp=%%A
CALL :TEST_VERSION %tmp%

:: Builder checks out a tagged release and stages a file
CALL git reset --hard v1.0.0 >NUL
FOR /F "tokens=*" %%A IN ('"git describe HEAD"') DO SET tmp=%%A
CALL touch README
CALL git add README
CALL :TEST_VERSION %tmp%

:: Builder commits that file.
CALL git commit -m "Modified Release" >NUL
FOR /F "tokens=*" %%A IN ('"git describe HEAD"') DO SET tmp=%%A
CALL :TEST_VERSION %tmp%

:: Builder creates own tag
CALL git tag v1.0.0-custom -m "Modified Release Tag"
FOR /F "tokens=*" %%A IN ('"git describe HEAD"') DO SET tmp=%%A
CALL :TEST_VERSION %tmp%

:: Builder checked out a maint release and staged a file
CALL git reset --hard v1.0.0.1 >NUL
FOR /F "tokens=*" %%A IN ('"git describe HEAD"') DO SET tmp=%%A
CALL touch README
CALL git add README
CALL :TEST_VERSION %tmp%

:: Builder commits that file.
CALL git commit -m "Modified Maint Release" >NUL
FOR /F "tokens=*" %%A IN ('"git describe HEAD"') DO SET tmp=%%A
CALL :TEST_VERSION %tmp%

:: Builder creates own tag
CALL git tag v1.0.0.1-custom -m "Modified Maint Release Tag"
FOR /F "tokens=*" %%A IN ('"git describe HEAD"') DO SET tmp=%%A
CALL :TEST_VERSION %tmp%

ECHO.

:: Cleanup the directory
POPD
RMDIR /S /Q %git-vs-version-test-dir%
GOTO :EOF

:: --------------------
:TEST_VERSION
:: --------------------
SET TEST_OUT_STRING=%1
CALL :START
ECHO %TEST_OUT_STRING%
SET TEST_OUT_STRING=
GOTO :EOF

:: --------------------
:TEST_OUT
:: --------------------
SET TEST_OUT_STRING=%TEST_OUT_STRING%, %strFILE_VERSION%
SET csvFILE_VERSION=%csvFILE_VERSION:,=, %
SET TEST_OUT_STRING=%TEST_OUT_STRING%, %csvFILE_VERSION%, %hexFILE_VERSION%
SET TEST_OUT_STRING=%TEST_OUT_STRING%, %fPRE_RELEASE%, %fPRIVATE%, %fPATCHED%, %strCOMMENT%
GOTO :EOF

:: --------------------
:END
:: --------------------
