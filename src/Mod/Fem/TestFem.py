# Unit test for the FEM module

#***************************************************************************
#*   Copyright (c) 2015 - FreeCAD Developers                               *
#*   Author: Przemo Firszt <przemo@firszt.eu>                              *
#*                                                                         *
#*   This file is part of the FreeCAD CAx development system.              *
#*                                                                         *
#*   This program is free software; you can redistribute it and/or modify  *
#*   it under the terms of the GNU Lesser General Public License (LGPL)    *
#*   as published by the Free Software Foundation; either version 2 of     *
#*   the License, or (at your option) any later version.                   *
#*   for detail see the LICENCE text file.                                 *
#*                                                                         *
#*   FreeCAD is distributed in the hope that it will be useful,            *
#*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
#*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
#*   GNU Library General Public License for more details.                  *
#*                                                                         *
#*   You should have received a copy of the GNU Library General Public     *
#*   License along with FreeCAD; if not, write to the Free Software        *
#*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
#*   USA                                                                   *
#*                                                                         *
#***************************************************************************/

import Fem
import FemTools
import FreeCAD
import MechanicalAnalysis
import csv
import tempfile
import unittest

mesh_name = 'Mesh'
static_analysis_dir = tempfile.gettempdir() + '/FEM_static'
frequency_analysis_dir = tempfile.gettempdir() + '/FEM_frequency'
static_analysis_inp_file = FreeCAD.getHomePath() + 'Mod/Fem/test_files/cube_static.inp'
static_expected_values = "Mod/Fem/test_files/cube_static_expected_values"
frequency_analysis_inp_file = FreeCAD.getHomePath() + 'Mod/Fem/test_files/cube_frequency.inp'
frequency_expected_values = "Mod/Fem/test_files/cube_frequency_expected_values"
mesh_points_file = FreeCAD.getHomePath() + 'Mod/Fem/test_files/mesh_points.csv'
mesh_volumes_file = FreeCAD.getHomePath() + 'Mod/Fem/test_files/mesh_volumes.csv'


class FemTest(unittest.TestCase):

    def setUp(self):
        try:
            FreeCAD.setActiveDocument("FemTest")
        except:
            FreeCAD.newDocument("FemTest")
        finally:
            FreeCAD.setActiveDocument("FemTest")
        self.active_doc = FreeCAD.ActiveDocument
        self.box = self.active_doc.addObject("Part::Box", "Box")
        self.active_doc.recompute()

    def create_new_analysis(self):
        self.analysis = MechanicalAnalysis.makeMechanicalAnalysis('MechanicalAnalysis')
        self.active_doc.recompute()

    def create_new_mesh(self):
        self.mesh_object = self.active_doc.addObject('Fem::FemMeshObject', mesh_name)
        self.mesh = Fem.FemMesh()
        with open(mesh_points_file, 'r') as points_file:
            reader = csv.reader(points_file)
            for p in reader:
                self.mesh.addNode(float(p[1]), float(p[2]), float(p[3]), int(p[0]))

        with open(mesh_volumes_file, 'r') as volumes_file:
            reader = csv.reader(volumes_file)
            #FIXME _v, __v
            for _v in reader:
                __v = [x if x is not " " else '0' for x in _v]
                v = [int(x) for x in __v]
                self.mesh.addVolume([v[2], v[1], v[3], v[4], v[5], v[7], v[6], v[9], v[8], v[10]], v[0])
        self.mesh_object.FemMesh = self.mesh
        self.active_doc.recompute()

    def create_new_material(self):
        self.new_material_object = self.active_doc.addObject("App::MaterialObjectPython", 'MechanicalMaterial')
        mat = self.new_material_object.Material
        mat['Name'] = "Steel"
        mat['YoungsModulus'] = "200000 MPa"
        mat['PoissonRatio'] = "0.30"
        mat['Density'] = "7900 kg/m^3"
        self.new_material_object.Material = mat

    def create_fixed_constraint(self):
        self.fixed_constraint = self.active_doc.addObject("Fem::ConstraintFixed", "FemConstraintFixed")
        self.fixed_constraint.References = [(self.box, "Face1")]

    def create_force_constraint(self):
        self.force_constraint = self.active_doc.addObject("Fem::ConstraintForce", "FemConstraintForce")
        self.force_constraint.References = [(self.box, "Face2")]
        self.force_constraint.Force = 10.000000
        self.force_constraint.Direction = (self.box, ["Edge5"])
        self.force_constraint.Reversed = True

    def create_pressure_constraint(self):
        self.pressure_constraint = self.active_doc.addObject("Fem::ConstraintPressure", "FemConstraintPressure")
        self.pressure_constraint.References = [(self.box, "Face2")]
        self.pressure_constraint.Pressure = 10.000000
        self.pressure_constraint.Reversed = True

    def compare_inp_files(self, file_name1, file_name2):
        file1 = open(file_name1, 'r')
        file2 = open(file_name2, 'r')
        f1 = file1.readlines()
        f2 = file2.readlines()
        lf1 = [l for l in f1 if not l.startswith('**')]
        lf2 = [l for l in f2 if not l.startswith('**')]
        import difflib
        diff = difflib.unified_diff(lf1, lf2, n=0)
        result = ''
        for l in diff:
            result += l
        file1.close()
        file2.close()
        return result

    def compare_stats(self, fea, stat_file=None):
        if stat_file:
            sf = open(stat_file, 'r')
            sf_content = sf.readlines()
            sf.close()
        stat_types = ["U1", "U2", "U3", "Uabs", "Sabs"]
        stats = []
        for s in stat_types:
            stats.append("{}: {}\n".format(s, fea.get_stats(s)))
        if sf_content != stats:
            print ("Expected stats from {}".format(stat_file))
            print sf_content
            print ("Stats read from {}.frd file".format(fea.base_name))
            print stats
            return True
        return False

    def test_new_analysis(self):
        FreeCAD.Console.PrintMessage('\n ---------- Start of FEM tests ----------\n')
        FreeCAD.Console.PrintMessage('Checking FEM new analysis...\n')
        self.create_new_analysis()
        self.assertTrue(self.analysis, "FemTest of new analysis failed")

        FreeCAD.Console.PrintMessage('Checking FEM new mesh...\n')
        self.create_new_mesh()
        self.assertTrue(self.mesh, "FemTest of new mesh failed")
        self.analysis.Member = self.analysis.Member + [self.mesh_object]

        FreeCAD.Console.PrintMessage('Checking FEM new material...\n')
        self.create_new_material()
        self.assertTrue(self.new_material_object, "FemTest of new material failed")
        self.analysis.Member = self.analysis.Member + [self.new_material_object]

        FreeCAD.Console.PrintMessage('Checking FEM new fixed constraint...\n')
        self.create_fixed_constraint()
        self.assertTrue(self.fixed_constraint, "FemTest of new fixed constraint failed")
        self.analysis.Member = self.analysis.Member + [self.fixed_constraint]

        FreeCAD.Console.PrintMessage('Checking FEM new force constraint...\n')
        self.create_force_constraint()
        self.assertTrue(self.force_constraint, "FemTest of new force constraint failed")
        self.analysis.Member = self.analysis.Member + [self.force_constraint]

        FreeCAD.Console.PrintMessage('Checking FEM new pressure constraint...\n')
        self.create_pressure_constraint()
        self.assertTrue(self.pressure_constraint, "FemTest of new pressure constraint failed")
        self.analysis.Member = self.analysis.Member + [self.pressure_constraint]

        fea = FemTools.FemTools(self.analysis)
        FreeCAD.Console.PrintMessage('Checking FEM inp file prerequisites...\n')
        error = fea.check_prerequisites()
        self.assertFalse(error, "FemTools check_prerequisites returned error message: {}".format(error))

        FreeCAD.Console.PrintMessage('Checking FEM inp file write...\n')
        fea.setup_working_dir(static_analysis_dir)
        FreeCAD.Console.PrintMessage('Writing {}/{}.inp for static analysis\n'.format(static_analysis_dir, mesh_name))
        error = fea.write_inp_file()
        FreeCAD.Console.PrintMessage('Comparing {} to {}/{}.inp\n'.format(static_analysis_inp_file, static_analysis_dir, mesh_name))
        ret = self.compare_inp_files(static_analysis_inp_file, static_analysis_dir + "/" + mesh_name + '.inp')
        self.assertFalse(ret, "FemTools write_inp_file test failed.\n{}".format(ret))

        FreeCAD.Console.PrintMessage('Checking FEM frd file read from static analysis...\n')
        fea.load_results()
        FreeCAD.Console.PrintMessage('Result object created as \"{}\"\n'.format(fea.result_object.Name))
        self.assertTrue(fea.results_present, "Cannot read results from {}.frd frd file".format(fea.base_name))

        FreeCAD.Console.PrintMessage('Reading stats from result object for static analysis...\n')
        ret = self.compare_stats(fea, static_expected_values)
        self.assertFalse(ret, "Invalid results read from .frd file")

        fea.set_analysis_type("frequency")
        fea.setup_working_dir(frequency_analysis_dir)
        FreeCAD.Console.PrintMessage('Writing {}/{}.inp for frequency analysis\n'.format(frequency_analysis_dir, mesh_name))
        error = fea.write_inp_file()
        FreeCAD.Console.PrintMessage('Comparing {} to {}/{}.inp\n'.format(frequency_analysis_inp_file, frequency_analysis_dir, mesh_name))
        ret = self.compare_inp_files(frequency_analysis_inp_file, frequency_analysis_dir + "/" + mesh_name + '.inp')
        self.assertFalse(ret, "FemTools write_inp_file test failed.\n{}".format(ret))

        FreeCAD.Console.PrintMessage('Checking FEM frd file read from frequency analysis...\n')
        fea.load_results()
        FreeCAD.Console.PrintMessage('Last result object created as \"{}\"\n'.format(fea.result_object.Name))
        self.assertTrue(fea.results_present, "Cannot read results from {}.frd frd file".format(fea.base_name))

        FreeCAD.Console.PrintMessage('Reading stats from result object for frequency analysis...\n')
        ret = self.compare_stats(fea, frequency_expected_values)
        self.assertFalse(ret, "Invalid results read from .frd file")


        FreeCAD.Console.PrintMessage('-------------- End of FEM tests ----------- \n')

    def tearDown(self):
        FreeCAD.closeDocument("FemTest")
        pass
