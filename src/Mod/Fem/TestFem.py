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
import FreeCAD
import unittest


class FemTest(unittest.TestCase):

    def setUp(self):
        try:
            FreeCAD.setActiveDocument("FemTest")
        except:
            FreeCAD.newDocument("FemTest")
        finally:
            FreeCAD.setActiveDocument("FemTest")
        self.active_doc = FreeCAD.ActiveDocument
        self.create_box()

    def test_new_analysis(self):
        FreeCAD.Console.PrintMessage('\nChecking FEM new analysis...\n')
        import MechanicalAnalysis
        analysis = MechanicalAnalysis.makeMechanicalAnalysis('MechanicalAnalysis')
        self.failUnless(analysis, "FemTest of new analysis failed")

    def create_new_mesh(self):
        self.mesh = Fem.FemMesh()
        import csv
        with open('Mod/Fem/mesh_points.csv', 'r') as points_file:
            reader = csv.reader(points_file)
            for p in reader:
                self.mesh.addNode(float(p[0]), float(p[1]), float(p[2]), int(p[3]))

        with open('Mod/Fem/mesh_volumes.csv', 'r') as volumes_file:
            reader = csv.reader(volumes_file)
            for _v in reader:
                v = [int(x) for x in _v]
                self.mesh.addVolume([v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9]], v[10])

    def test_new_mesh(self):
        FreeCAD.Console.PrintMessage('\nChecking FEM new mesh...\n')
        self.create_new_mesh()
        self.failUnless(self.mesh, "FemTest of new mesh failed")

    def create_box(self):
        self.box = self.active_doc.addObject("Part::Box", "Box")

    def create_fixed_constraint(self):
        self.fixed_constraint = self.active_doc.addObject("Fem::ConstraintFixed", "FemConstraintFixed")
        self.fixed_constraint.References = [(self.box, "Face1")]

    def test_new_fixed_constraint(self):
        FreeCAD.Console.PrintMessage('\nChecking FEM new fixed constraint...\n')
        self.create_fixed_constraint()
        self.failUnless(self.fixed_constraint, "FemTest of new fixed constraint failed")

    def create_force_constraint(self):
        self.force_constraint = self.active_doc.addObject("Fem::ConstraintForce", "FemConstraintForce")
        self.force_constraint.References = [(self.box, "Face1")]
        self.force_constraint.Force = 10.000000
        self.force_constraint.Direction = (self.box, ["Edge12"])
        self.force_constraint.Reversed = True

    def test_new_force_constraint(self):
        FreeCAD.Console.PrintMessage('\nChecking FEM new force constraint...\n')
        self.create_force_constraint()
        self.failUnless(self.force_constraint, "FemTest of new force constraint failed")

    def create_pressure_constraint(self):
        self.pressure_constraint = self.active_doc.addObject("Fem::ConstraintPressure", "FemConstraintPressure")
        self.pressure_constraint.References = [(self.box, "Face1")]
        self.pressure_constraint.Pressure = 10.000000
        self.pressure_constraint.Reversed = True

    def test_new_pressure_constraint(self):
        FreeCAD.Console.PrintMessage('\nChecking FEM new pressure constraint...\n')
        self.create_pressure_constraint()
        self.failUnless(self.pressure_constraint, "FemTest of new pressure constraint failed")

    def tearDown(self):
        FreeCAD.closeDocument("FemTest")
        pass
