@echo off

rem	=============================
rem	If ROM exists, generate patch
rem	=============================
if exist yi.sfc (
	copy yi.sfc yi_patched.sfc
	asar main_8mb.asm yi_patched.sfc
	beat -create -delta -p patch_8mb.bps -o yi_patched.sfc yi.sfc
	del yi_patched.sfc
)

rem	===============================================
rem	Otherwise, let user know that ROM doesn't exist
rem	===============================================
else (
	echo Could not find Yoshi's Island US 1.0 ROM `yi.sfc`.
)

rem	==============================
rem	Pause so we can see any errors
rem	==============================
pause
