# Day 018 - Enforcing a Video Frame Rate

Hertz is another term for "cycles per second".

If our target is 60 frames per second, how many milliseconds do we have to compute a single frame?

(1000 milliseconds in 1 second)

60 frames per second  = (1000ms / 60fps) = 16.6ms per frame

30 frames per second  = (1000ms / 60fps) = 33.3ms per frame

`__rdtsc` is not used to determine time elapsed as it varies from machine to machine, rather it's used for profiling the performance of code.

`QueryPerformanceCounter` is used to determine how long it takes for a certain amount of code to execute.

To include an external .Lib (Library file) to your Project and include it within the linking phase of the compilation process, right click on the Project's name in the Solution Explorer and click on `Properties`. Then select `Configuration Properties` -> Linker` -> `Input`. Assuming you want the .lib file to be included for all Configurations and all Plarforms, make sure you select `All Configurations` and `All Platforms` at the top of the dialog. Once done, in the top input box (`Additional Dependencies`) add the `.lib` file preceded by a semicolon to the end of the string e.g. `;Winmm.lib` and click `Apply`
