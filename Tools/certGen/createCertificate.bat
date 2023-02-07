@echo off
setlocal enableextensions
setlocal enabledelayedexpansion
set e=echo
set ver=1.0
Set _string=%~1

@REM ## Read args
:loopargs
if "%1"=="" (goto :exitloopargs)
if "%1"=="-e" set ENV=%2
if "%1"=="--environment" set ENV=%2
if "%1"=="-o" set OUT=%2
if "%1"=="-outdir" set OUT=%2
shift
shift
goto :loopargs
:exitloopargs
if "%ENV%"=="" echo "Requires --environment <ENV> or -e <ENV> argument " & exit 2

@REM ## Set vars
set TEMPLATE_OPENSSL_CONF=openssl.cnf.template
if not exist %TEMPLATE_OPENSSL_CONF% (
    echo Could not find openssl.cnf.template
    exit 2
)
set TARGET_DIR=%ENV%
if not "%OUT%"=="" (
    set TARGET_DIR=%OUT%
)
set TARGET_OPENSSL_CONF=%TARGET_DIR%\openssl.cnf
echo Environment: %ENV%
echo TEMPLATE_OPENSSL_CONF: %TEMPLATE_OPENSSL_CONF%
echo TARGET_DIR: %TARGET_DIR%

@REM ## Create target folder
if not exist "%TARGET_DIR%" mkdir "%TARGET_DIR%"
@REM COPY "%TEMPLATE_OPENSSL_CONF%" "%TARGET_OPENSSL_CONF%"

@REM ## Load Settings
@REM #default settings

set Hostnames=localhost
set IPs=127.0.0.1
set URI=http://dashboard.umati.app/OPCUA_DataClient
set Keysize=2048
set Days=365
set Subject=/C=DE/O=SampleOrganization/CN=UmatiDashboardClient@localhost
set CONF_FILE=%ENV%.conf
if exist "%CONF_FILE%" (
    echo Load config file %CONF_FILE%
    for /f "delims== tokens=1* eol=# " %%x in (%CONF_FILE%) do set %%x=%%y
) else (
    echo Use default config
)
set SETTINGS_BACKUP=%TARGET_DIR%/%ENV%.conf.bak
@REM touch $SETTINGS_BACKUP
break > %SETTINGS_BACKUP%
echo Hostnames=%Hostnames%>>%SETTINGS_BACKUP%
echo IPs=%IPs%>>%SETTINGS_BACKUP%
echo URI=%URI%>>%SETTINGS_BACKUP%
echo Keysize=%Keysize%>>%SETTINGS_BACKUP%
echo Days=%Days%>>%SETTINGS_BACKUP%
echo Subject=%Subject%>>%SETTINGS_BACKUP%

@REM #Generate openssl.cnf

(for /f "tokens=* delims=" %%a in (%TEMPLATE_OPENSSL_CONF%) do (
    set str=%%a
    if not !str!==!str:{{IP}}=! (
        set /a n=0
        for %%b in (%IPs%) do (
            echo IP.!n! = %%b
            set /a n=n+1
        )
    ) else (
        if not !str!==!str:{{HOSTNAME}}=! (
            set /a n=0
            for %%b in (%Hostnames%) do (
                echo DNS.!n! = %%b
                set /a n=n+1
            )
        ) else (
            if not !str!==!str:{{URI}}=! (
                echo URI.1 = !URI!
            ) else (
                echo !str!
            )
        )
    )
))>%TARGET_OPENSSL_CONF%

set outCrt=%~dp0%TARGET_DIR%\%ENV%.crt
set outKey=%~dp0%TARGET_DIR%\%ENV%.key
set outCert=%~dp0%TARGET_DIR%\client_cert.der
set outKeyDer=%~dp0%TARGET_DIR%\client_key.der
echo %outKey%

openssl req ^
     -config %~dp0%TARGET_OPENSSL_CONF% ^
     -noenc ^
     -new ^
     -x509 -sha256 ^
     -out %outCrt% ^
     -newkey rsa:%Keysize% ^
     -keyout %outKey% ^
     -days %Days% ^
     -subj %Subject%

openssl x509 -in %outCrt% -outform der -out %outCert%
openssl rsa -inform PEM -in %outKey% -outform DER -out %outKeyDer%

echo Use the generated %outCert% and %outKeyDer% for the client.
    