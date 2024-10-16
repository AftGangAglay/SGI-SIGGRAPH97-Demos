 _________________________________________________________________________

 GLTRACE

 Win32 module to redirect calls from OPENGL32.DLL to specified provider DLL, 
 with API function trace to OutputDebugString() or text file

 Released into public domain 9-Jul-97

 Please forward enhancements and bug reports to John Miles at Digital
 Anvil, Inc.  (jmiles@digitalanvil.com)
 _________________________________________________________________________

 Instructions:

    - If original OPENGL32.DLL resides in application directory, rename or
      move it to avoid conflicts with this copy of OPENGL32.DLL

    - After renaming original OPENGL32.DLL (if necessary), copy 
      OPENGL32.DLL and GLTRACE.INI into target application directory 
      (e.g, C:\QUAKE)

    - Modify GLTRACE.INI's [Implementation] section to point to renamed or
      moved copy of original OPENGL32.DLL (e.g., the 3Dfx GLQuake library)

    - Modify other GLTRACE.INI sections, if desired, in accordance with
      instructions in GLTRACE.INI comment header

    - Run the application under test (e.g., GLQUAKE.EXE) and observe
      output trace
 _________________________________________________________________________

