
add_subdirectory(App)
if(BUILD_GUI)
    add_subdirectory(Gui)
endif(BUILD_GUI)


INSTALL(
    FILES
        Init.py
        InitGui.py
        convert2TetGen.py
        ccxFrdReader.py
        ccxInpWriter.py
        FemTools.py
        TestFem.py
        FemExample.py
        MechanicalAnalysis.py
        MechanicalMaterial.py
        MechanicalMaterial.ui
        MechanicalAnalysis.ui
        ShowDisplacement.ui
    DESTINATION
        Mod/Fem
)

INSTALL(
    FILES
	test_files/mesh_points.csv
	test_files/mesh_volumes.csv
	test_files/static_analysis.inp
	test_files/frequency_analysis.inp
        test_files/cube_10x10x10_no_1_frequency.inp
	test_files/cube_10x10x10_no_1_static.inp
    DESTINATION
        Mod/Fem/test_files
)
