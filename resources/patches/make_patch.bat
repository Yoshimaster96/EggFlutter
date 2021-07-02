@echo off
if exist yi.sfc (
	copy yi.sfc yi_patched.sfc
	asar main.asm yi_patched.sfc
	beat -create -delta -p patch.bps -o yi_patched.sfc yi.sfc
	del yi_patched.sfc
) else (
	echo Could not find Yoshi's Island US 1.0 ROM `yi.sfc`.
)
pause
