# Rust_NoRecoil
No recoil macro program for Rust game

## How to compile?
Open the [project solution](no_recoil.sln) with visual studio 2017, 
then click ```Build -> Build Solution``` or alternatively ```CTRL+Shift+B```.
The executable file should appear in ```x64\Debug``` or ```x64\Release```.

## How to use?
Run the executable at any point (either before or after opening Rust).
Initially the macro will be off and pressing ```B``` will toggle the
macro on and off. There are currently macro support for ```Assault Rifle```
and ```Semi Automatic Rifle```. 

To switch between weapons, press ```N``` when the macro is on, then input
the index of the weapon then press ```N``` again. To make sure when you can
start entering the index number, listen for the beep sound which will happen
at the first click of ```N``` button.
