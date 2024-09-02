# NewVerDLLProxy

---

Just a quick and easy way to make more than one proxy dll.. 
 - version.dll
 - winmm.dll
 - cryptbase.dll
   
# this contains some test code for...
 - notepad.exe (pre win11, new is untested)
 - calc.exe (pre win11)

---
# quickstart
- use #define in dllmain.c to select which dll to build
- update target exe name in init
- put code in init function of version.c for things to run before starting a thread and releasing loader lock
- *NOTE:* although init is being used in this demo.. it is not recommended to be used unless you know what you are doing. Most usage should have all code in pRun.
- put code in pRun that will be launched in a thread allowing for loader lock to be released.
- NOTE: due to the demo code using notepad.exe and calc.exe (do not overwrite your actual dll in system32, copy needed files to a folder for testing (exe and locale .mui file)).
- That being said... compile and place in the same folder as the target executable (not where the original resides)...
