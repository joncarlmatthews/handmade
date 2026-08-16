# Day 000 - Visual Studio Set up - Build directories

*I opted to tidy up and consolidate the location of the build files.*

For each Project within the Solution:

Right click on the Project's name in the Solution Explorer and click on `Properties`. Then select `General` -> `General Properties`. Select `All Configurations` within the `Configuration` selector. Select `All Platforms` within the `Platform` selector. For the `Output Directory` value input `$(SolutionDir)build\$(ProjectName)\$(Platform)\$(Configuration)\`. For the `Intermediate Directory` value input `$(SolutionDir)build\$(ProjectName)\intermediates\$(Platform)\$(Configuration)\`. Click `Apply`

The Windows Platform project requires `winmm.lib` for the `timeBeginPeriod` and `timeEndPeriod` external symbols. Right click on the Project's name in the Solution Explorer and click on `Properties`. Then select `Configuration Properties` -> `Linker` -> `Input`. Select `Additional Dependencies.  Select `All Configurations` within the `Configuration` selector. Select `All Platforms` within the `Platform` selector. Add `winmm.lib` to the list. Click `Apply`

*Set the warning to Level 4*

For each Project within the Solution:

Right click on the Project's name in the Solution Explorer and click on `Properties`. Then select `Configuration Properties` -> `C/C++`. Select `All Configurations` within the `Configuration` selector. Select `All Platforms` within the `Platform` selector. For the `Warning Level` value input `Level4 (/W4)`. Click `Apply`
