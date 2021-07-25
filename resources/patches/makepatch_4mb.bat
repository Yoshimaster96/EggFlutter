@echo off

rem	=============================
rem	If ROM exists, generate patch
rem	=============================
if exist yi.sfc (
	rem Patch for clean ROM
	copy yi.sfc yi_patched.sfc
	asar main_4mb.asm yi_patched.sfc
	beat -create -delta -p patch_4mb.bps -o yi_patched.sfc yi.sfc
	del yi_patched.sfc
	rem Patch for converting from 1.0x to 1.1x
	beat -apply -p old\v100_4mb.bps -o yi_old.sfc yi.sfc
	copy yi_old.sfc yi_patched.sfc
	asar main_4mb.asm yi_patched.sfc
	beat -create -delta -p fromv100_4mb.bps -o yi_patched.sfc yi_old.sfc
	del yi_old.sfc
	del yi_patched.sfc

rem	===============================================
rem	Otherwise, let user know that ROM doesn't exist
rem	===============================================
) else (
	echo Could not find Yoshi's Island US 1.0 ROM `yi.sfc`.
)

rem	==============================
rem	Pause so we can see any errors
rem	==============================
pause
