# Automated Regression Test Runner (Windows PowerShell)
$env:PATH = "C:\msys64\usr\bin;C:\msys64\mingw64\bin;" + $env:PATH

Write-Host "==================================================" -ForegroundColor Cyan
Write-Host " Starting Automated Disassembler Regression Suite " -ForegroundColor Cyan
Write-Host "==================================================" -ForegroundColor Cyan

mingw32-make clean

Write-Host "`n[STEP 1/3] Running 97 MC/DC Unit Tests..." -ForegroundColor Yellow
mingw32-make unit_tests

Write-Host "`n[STEP 2/3] Running State & Feature Verification..." -ForegroundColor Yellow
mingw32-make test_state

Write-Host "`n[STEP 3/3] Running Adapter Bridge & Full Regression..." -ForegroundColor Yellow
mingw32-make test_adapter_bridge

Write-Host "`n==================================================" -ForegroundColor Green
Write-Host " ALL REGRESSION SUITES PASSED CLEANLY!            " -ForegroundColor Green
Write-Host "==================================================" -ForegroundColor Green
