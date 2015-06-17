#***************************************************************************
#*                                                                         *
#*   Copyright (c) 2015 - FreeCAD Developers                               *
#*   Przemo Firszt <przemo@firszt.eu>                                      *
#*                                                                         *
#*   This program is free software; you can redistribute it and/or modify  *
#*   it under the terms of the GNU Lesser General Public License (LGPL)    *
#*   as published by the Free Software Foundation; either version 2 of     *
#*   the License, or (at your option) any later version.                   *
#*   for detail see the LICENCE text file.                                 *
#*                                                                         *
#*   This program is distributed in the hope that it will be useful,       *
#*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
#*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
#*   GNU Library General Public License for more details.                  *
#*                                                                         *
#*   You should have received a copy of the GNU Library General Public     *
#*   License along with this program; if not, write to the Free Software   *
#*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
#*   USA                                                                   *
#*                                                                         *
#***************************************************************************


import FreeCAD
import FemGui
from PySide import QtCore

## fem_analysis - Finite Element Method Analysis
# fem_analysis class handles actions related to FEM analysis like preparing .inp
# file, running CalculiX, loading results, mesh colour/deformation operations
# It's a toolbox that can be used for easy scripting of FEM calculations
# All non GUI related features of MechanicalAnalysis.py will be migrated
# to use fem_analysis - currently there is significant functionality overlap


class fem_analysis:

    def __init__(self, analysis=None):

        self.set_analysis(analysis)
        self.update_objects()
        self.base_name = ""
        self.results_present = False
        self.setup_working_dir()
        self.setup_ccx()

    def set_analysis(self, analysis=None):
        if analysis:
            self.analysis = analysis
        else:
            self.analysis = FemGui.getActiveAnalysis()

    def purge_results(self):
        for m in self.analysis.Member:
            if (m.isDerivedFrom("Fem::FemResultVector") or
               (m.isDerivedFrom("Fem::FemResultValue") and m.DataType == "VonMisesStress") or
               (m.isDerivedFrom("Fem::FemResultValue") and m.DataType == "AnalysisStats")):
                FreeCAD.ActiveDocument.removeObject(m.Name)
        self.results_present = False

    def reset_mesh_deformation(self):
        if self.mesh:
            self.mesh.ViewObject.applyDisplacement(0.0)

    def reset_mesh_color(self):
        if self.mesh:
            self.mesh.ViewObject.NodeColor = {}
            self.mesh.ViewObject.ElementColor = {}
            self.mesh.ViewObject.setNodeColorByResult()

    def set_result_type(self, result_type):
        self.update_objects()
        if result_type == "None":
            self.reset_mesh_color()
            return
        if self.results_present:
            match = {"Uabs": 0, "U1": 1, "U2": 2, "U3": 3, "Sabs": 0}
            if result_type == "Sabs":
                obj = self.vm_stress
            else:
                obj = self.displacement
            self.mesh.ViewObject.setNodeColorByResult(obj, match[result_type])

    def update_objects(self):
        if not self.analysis:
            print "Analysis not set, cannot update objects"
            return
        # [{"Object":material}, {}, ...]
        # [{"Object":fixed_constraints, "NodeSupports":bool}, {}, ...]
        # [{"Object":force_constraints, "NodeLoad":value}, {}, ...
        # [{"Object":pressure_constraints, "xxxxxxxx":value}, {}, ...]
        self.mesh = None
        self.material = []
        self.fixed_constraints = []
        self.force_constraints = []
        self.pressure_constraints = []
        self.displacement = None
        self.vm_stress = None

        for m in self.analysis.Member:
            if m.isDerivedFrom("Fem::FemMeshObject"):
                self.mesh = m
            elif m.isDerivedFrom("App::MaterialObjectPython"):
                material_dict = {}
                material_dict["Object"] = m
                self.material.append(material_dict)
            elif m.isDerivedFrom("Fem::ConstraintFixed"):
                fixed_constraint_dict = {}
                fixed_constraint_dict["Object"] = m
                self.fixed_constraints.append(fixed_constraint_dict)
            elif m.isDerivedFrom("Fem::ConstraintForce"):
                force_constraint_dict = {}
                force_constraint_dict["Object"] = m
                self.force_constraints.append(force_constraint_dict)
            elif m.isDerivedFrom("Fem::ConstraintPressure"):
                PressureObjectDict = {}
                PressureObjectDict["Object"] = m
                self.pressure_constraints.append(PressureObjectDict)
            elif m.isDerivedFrom("Fem::FemResultVector") and m.DataType == "Displacement":
                    self.displacement = m
            elif m.isDerivedFrom("Fem::FemResultValue") and m.DataType == "VonMisesStress":
                    self.vm_stress = m
            elif m.isDerivedFrom("Fem::FemResultValue") and m.DataType == "AnalysisStats":
                    self.stats = m

    def check_prerequisites(self):
        message = ""
        if not self.analysis:
            message += "No active Analysis\n"
        if not self.mesh:
            message += "No mesh object in the Analysis\n"
        if not self.material:
            message += "No material object in the Analysis\n"
        if not self.fixed_constraints:
            message += "No fixed-constraint nodes defined in the Analysis\n"
        if not (self.force_constraints or self.pressure_constraints):
            message += "No force-constraint or pressure-constraint defined in the Analysis\n"
        if not self.ccx_binary:
            message += "CalculiX binary ccx is not set\n"
        if not self.working_dir:
            message += "CalculiX working dir not set\n"

        return message

    def write_inp_file(self):
        import ccxInpWriter
        import sys
        try:
            iw = ccxInpWriter.inp_writer(self.analysis, self.mesh, self.material,
                                         self.fixed_constraints, self.force_constraints,
                                         self.pressure_constraints)
            self.base_name = iw.write_calculix_input_file()
        except:
            print "Unexpected error when writing CalculiX input file:", sys.exc_info()[0]
            raise

    def start_ccx(self):
        # change cwd because ccx may crash if directory has no write permission
        # there is also a limit of the length of file names so jump to the document directory
        import subprocess
        if self.base_name != "":
            cwd = QtCore.QDir.currentPath()
            f = QtCore.QFileInfo(self.base_name)
            QtCore.QDir.setCurrent(f.path())
            print f.baseName()
            print f.path()
            #self.ccx_process.start(self.ccx_binary, ["-i", f.baseName()])
            p = subprocess.Popen([self.ccx_binary, "-i ", f.baseName()],
                                 shell=False,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE)

            self.ccx_stdout, self.ccx_stderr = p.communicate()

            #self.ccx_process.start(self.ccx_binary, ["-i", f.baseName()])
            # Restore previous cwd
            QtCore.QDir.setCurrent(cwd)

    def setup_working_dir(self, working_dir=None):
        if working_dir is None:
            self.fem_prefs = FreeCAD.ParamGet("User parameter:BaseApp/Preferences/Mod/Fem")
            self.working_dir = self.fem_prefs.GetString("WorkingDir", "/tmp")
        else:
            self.working_dir = working_dir

    def setup_ccx(self, ccx_binary=None):
        import threading
        if not ccx_binary:
            self.fem_prefs = FreeCAD.ParamGet("User parameter:BaseApp/Preferences/Mod/Fem")
            ccx_binary = self.fem_prefs.GetString("ccxBinaryPath", "")
        if not ccx_binary:
            from platform import system
            if system() == "Linux":
                ccx_binary = "ccx"
            elif system() == "Windows":
                ccx_binary = FreeCAD.getHomePath() + "bin/ccx.exe"
            else:
                ccx_binary = "ccx"
        self.ccx_binary = ccx_binary
#http://stackoverflow.com/questions/984941/python-subprocess-popen-from-a-thread
        self.ccx_thread = threading.Thread(target=self.execute_calcs)
#        self.ccx_thread.daemon = True
#        self.ccx_process = QtCore.QProcess()
#        QtCore.QObject.connect(self.ccx_process, QtCore.SIGNAL("started()"), self.ccx_started)
#        QtCore.QObject.connect(self.ccx_process, QtCore.SIGNAL("stateChanged(QProcess::ProcessState)"), self.ccx_state_changed)
#        QtCore.QObject.connect(self.ccx_process, QtCore.SIGNAL("error(QProcess::ProcessError)"), self.ccx_error)
#        QtCore.QObject.connect(self.ccx_process, QtCore.SIGNAL("finished(int)"), self.ccx_finished)

#    def ccx_started(self):
#        self.ccx_status = "started"

#    def ccx_state_changed(self, new_state):
#        if (new_state == QtCore.QProcess.ProcessState.Starting):
#                self.ccx_status = "starting"
#        elif (new_state == QtCore.QProcess.ProcessState.Running):
#                self.ccx_status = "running"
#        elif (new_state == QtCore.QProcess.ProcessState.NotRunning):
#                self.ccx_status = "not running"

#    def ccx_error(self, error):
#        #FIXME error hadling
#        self.ccx_status = "error"
#        print "ccx_error {}".format(error)

#    def ccx_finished(self, exit_code):
#        self.ccx_status = "finished"
#        print "ccx_finished {}".format(exit_code)
#        self.load_results()

    def ccx_read_stdout(self):
        return self.ccx_process.readAllStandardOutput()

    def load_results(self):
        import ccxFrdReader
        import os
        if os.path.isfile(self.base_name + ".frd"):
            ccxFrdReader.importFrd(self.base_name + ".frd", self.analysis)
            self.results_present = True
        else:
            self.results_present = False

    def execute_calcs(self):
        print "execute_calcs 1"
        if self.check_prerequisites():
            print "execute_calcs 2"
            return False
        print "execute_calcs 3"
        self.write_inp_file()
        print "execute_calcs 4"
        self.start_ccx()
        print "execute_calcs 5"
        self.load_results()
        print "execute_calcs 6"
        self.ccx_thread = None
        return True

    def run(self):
        print "starting thread"
        self.ccx_thread.start()

    ## returns minimum, average and maximum value for provided result type
    #  @param self The python object self
    #  @result_type Type of FEM result, allowed U1, U2, U3, Uabs, Sabs and None
    def get_stats(self, result_type):
        stats = (0.0, 0.0, 0.0)
        for m in self.analysis.Member:
            if m.isDerivedFrom("Fem::FemResultValue") and m.DataType == 'AnalysisStats':
                match = {"U1": (m.Values[0], m.Values[1], m.Values[2]),
                         "U2": (m.Values[3], m.Values[4], m.Values[5]),
                         "U3": (m.Values[6], m.Values[7], m.Values[8]),
                         "Uabs": (m.Values[9], m.Values[10], m.Values[11]),
                         "Sabs": (m.Values[12], m.Values[13], m.Values[14]),
                         "None": (0.0, 0.0, 0.0)}
                stats = match[result_type]
        return stats
