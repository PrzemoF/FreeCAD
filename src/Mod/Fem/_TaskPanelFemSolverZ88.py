# ***************************************************************************
# *                                                                         *
# *   Copyright (c) 2016 - Bernd Hahnebach <bernd@bimstatik.org>            *
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU Lesser General Public License (LGPL)    *
# *   as published by the Free Software Foundation; either version 2 of     *
# *   the License, or (at your option) any later version.                   *
# *   for detail see the LICENCE text file.                                 *
# *                                                                         *
# *   This program is distributed in the hope that it will be useful,       *
# *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
# *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
# *   GNU Library General Public License for more details.                  *
# *                                                                         *
# *   You should have received a copy of the GNU Library General Public     *
# *   License along with this program; if not, write to the Free Software   *
# *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
# *   USA                                                                   *
# *                                                                         *
# ***************************************************************************

__title__ = "Z88 Job Control Task Panel"
__author__ = "Bernd Hahnebach"
__url__ = "http://www.freecadweb.org"

import FemToolsZ88
import FreeCAD
import os
import time

if FreeCAD.GuiUp:
    import FreeCADGui
    # import FemGui
    from PySide import QtCore, QtGui
    from PySide.QtCore import Qt
    from PySide.QtGui import QApplication


class _TaskPanelFemSolverZ88:
    def __init__(self, solver_object):
        self.form = FreeCADGui.PySideUic.loadUi(FreeCAD.getHomePath() + "Mod/Fem/TaskPanelFemSolverZ88.ui")
        self.fem_prefs = FreeCAD.ParamGet("User parameter:BaseApp/Preferences/Mod/Fem")
        # z88_binary = self.fem_prefs.GetString("z88BinaryPath", "")
        z88_binary = ''
        if z88_binary:
            self.Z88Binary = z88_binary
            print ("Using Z88 binary path from FEM preferences: {}".format(z88_binary))
        else:
            from platform import system
            if system() == 'Linux':
                self.Z88Binary = 'z88'
            elif system() == 'Windows':
                self.Z88Binary = FreeCAD.getHomePath() + 'bin/z88.exe'
            else:
                self.Z88Binary = 'z88'
        self.fem_prefs = FreeCAD.ParamGet("User parameter:BaseApp/Preferences/Mod/Fem")

        self.solver_object = solver_object

        self.Z88 = QtCore.QProcess()
        self.Timer = QtCore.QTimer()
        self.Timer.start(300)

        self.fem_console_message = ''

        #Connect Signals and Slots
        QtCore.QObject.connect(self.form.tb_choose_working_dir, QtCore.SIGNAL("clicked()"), self.choose_working_dir)
        QtCore.QObject.connect(self.form.pb_write_inp, QtCore.SIGNAL("clicked()"), self.write_input_file_handler)
        QtCore.QObject.connect(self.form.pb_edit_inp, QtCore.SIGNAL("clicked()"), self.editZ88InputFile)
        QtCore.QObject.connect(self.form.pb_run_z88, QtCore.SIGNAL("clicked()"), self.runZ88)

        QtCore.QObject.connect(self.Z88, QtCore.SIGNAL("started()"), self.z88Started)
        QtCore.QObject.connect(self.Z88, QtCore.SIGNAL("stateChanged(QProcess::ProcessState)"), self.z88StateChanged)
        QtCore.QObject.connect(self.Z88, QtCore.SIGNAL("error(QProcess::ProcessError)"), self.z88Error)
        QtCore.QObject.connect(self.Z88, QtCore.SIGNAL("finished(int)"), self.z88Finished)

        QtCore.QObject.connect(self.Timer, QtCore.SIGNAL("timeout()"), self.UpdateText)

        self.update()

    def femConsoleMessage(self, message="", color="#000000"):
        self.fem_console_message = self.fem_console_message + '<font color="#0000FF">{0:4.1f}:</font> <font color="{1}">{2}</font><br>'.\
            format(time.time() - self.Start, color, message.encode('utf-8', 'replace'))
        self.form.textEdit_Output.setText(self.fem_console_message)
        self.form.textEdit_Output.moveCursor(QtGui.QTextCursor.End)

    def printZ88stdout(self):
        out = self.Z88.readAllStandardOutput()
        if out.isEmpty():
            self.femConsoleMessage("Z88 stdout is empty", "#FF0000")
        else:
            try:
                out = unicode(out, 'utf-8', 'replace')
                rx = QtCore.QRegExp("\\*ERROR.*\\n\\n")
                rx.setMinimal(True)
                pos = rx.indexIn(out)
                while not pos < 0:
                    match = rx.cap(0)
                    FreeCAD.Console.PrintError(match.strip().replace('\n', ' ') + '\n')
                    pos = rx.indexIn(out, pos + 1)
                out = os.linesep.join([s for s in out.splitlines() if s])
                self.femConsoleMessage(out.replace('\n', '<br>'))
            except UnicodeDecodeError:
                self.femConsoleMessage("Error converting stdout from Z88", "#FF0000")

    def UpdateText(self):
        if(self.Z88.state() == QtCore.QProcess.ProcessState.Running):
            self.form.l_time.setText('Time: {0:4.1f}: '.format(time.time() - self.Start))

    def z88Error(self, error):
        print ("Error() {}".format(error))
        self.femConsoleMessage("Z88 execute error: {}".format(error), "#FF0000")

    def z88Started(self):
        print ("z88Started()")
        print (self.Z88.state())
        self.form.pb_run_z88.setText("Break Z88")

    def z88StateChanged(self, newState):
        if (newState == QtCore.QProcess.ProcessState.Starting):
                self.femConsoleMessage("Starting Z88...")
        if (newState == QtCore.QProcess.ProcessState.Running):
                self.femConsoleMessage("Z88 is running...")
        if (newState == QtCore.QProcess.ProcessState.NotRunning):
                self.femConsoleMessage("Z88 stopped.")

    def z88Finished(self, exitCode):
        print ("z88Finished() {}".format(exitCode))
        print (self.Z88.state())

        # Restore previous cwd
        QtCore.QDir.setCurrent(self.cwd)

        self.printZ88stdout()
        self.Timer.stop()

        self.femConsoleMessage("Z88 done!", "#00AA00")

        self.form.pb_run_z88.setText("Re-run Z88")
        self.femConsoleMessage("Loading result sets...")
        self.form.l_time.setText('Time: {0:4.1f}: '.format(time.time() - self.Start))
        fea = FemToolsZ88.FemToolsZ88()
        fea.reset_all()
        fea.inp_file_name = self.inp_file_name
        QApplication.setOverrideCursor(Qt.WaitCursor)
        fea.load_results()
        QApplication.restoreOverrideCursor()
        self.form.l_time.setText('Time: {0:4.1f}: '.format(time.time() - self.Start))

    def getStandardButtons(self):
        return int(QtGui.QDialogButtonBox.Close)

    def update(self):
        'fills the widgets'
        self.form.le_working_dir.setText(self.solver_object.WorkingDir)
        if self.solver_object.AnalysisType == 'static':
            self.form.rb_static_analysis.setChecked(True)
        elif self.solver_object.AnalysisType == 'frequency':
            self.form.rb_frequency_analysis.setChecked(True)
        return

    def accept(self):
        FreeCADGui.ActiveDocument.resetEdit()

    def reject(self):
        FreeCADGui.ActiveDocument.resetEdit()

    def choose_working_dir(self):
        current_wd = self.setup_working_dir()
        wd = QtGui.QFileDialog.getExistingDirectory(None, 'Choose Z88 working directory',
                                                    current_wd)
        if wd:
            self.solver_object.WorkingDir = wd
        else:
            self.solver_object.WorkingDir = current_wd
        self.form.le_working_dir.setText(self.solver_object.WorkingDir)

    def write_input_file_handler(self):
        QApplication.restoreOverrideCursor()
        if self.check_prerequisites_helper():
            QApplication.setOverrideCursor(Qt.WaitCursor)
            self.inp_file_name = ""
            fea = FemToolsZ88.FemToolsZ88()
            fea.set_analysis_type(self.solver_object.AnalysisType)
            fea.update_objects()
            fea.write_inp_file()
            if fea.inp_file_name != "":
                self.inp_file_name = fea.inp_file_name
                self.femConsoleMessage("Write completed.")
                self.form.pb_edit_inp.setEnabled(True)
                self.form.pb_run_z88.setEnabled(True)
            else:
                self.femConsoleMessage("Write .inp file failed!", "#FF0000")
            QApplication.restoreOverrideCursor()

    def check_prerequisites_helper(self):
        self.Start = time.time()
        self.femConsoleMessage("Check dependencies...")
        self.form.l_time.setText('Time: {0:4.1f}: '.format(time.time() - self.Start))

        fea = FemToolsZ88.FemToolsZ88()
        fea.update_objects()
        message = fea.check_prerequisites()
        if message != "":
            QtGui.QMessageBox.critical(None, "Missing prerequisit(s)", message)
            return False
        return True

    def editZ88InputFile(self):
        print ('not implemented')

    def runZ88(self):
        print ('runZ88')
        fea = FemToolsZ88.FemToolsZ88()
        fea.FemToolsZ88.setup_z88()
        print ('not implemented')
        '''
        self.Start = time.time()

        self.femConsoleMessage("Z88 binary: {}".format(self.Z88Binary))
        self.femConsoleMessage("Run Z88...")

        # run Z88
        print ('run Z88 at: {} with: {}'.format(self.Z88Binary, os.path.splitext(self.inp_file_name)[0]))
        # change cwd because z88 may crash if directory has no write permission
        # there is also a limit of the length of file names so jump to the document directory
        self.cwd = QtCore.QDir.currentPath()
        fi = QtCore.QFileInfo(self.inp_file_name)
        QtCore.QDir.setCurrent(fi.path())
        self.Z88.start(self.Z88Binary, ['-i', fi.baseName()])

        QApplication.restoreOverrideCursor()
        '''

    # That function overlaps with FemTools setup_working_dir and needs to be removed when we migrate fully to FemTools
    def setup_working_dir(self):
        wd = self.solver_object.WorkingDir
        if not (os.path.isdir(wd)):
            try:
                os.makedirs(wd)
            except:
                print ("Dir \'{}\' from FEM preferences doesn't exist and cannot be created.".format(wd))
                import tempfile
                wd = tempfile.gettempdir()
                print ("Dir \'{}\' will be used instead.".format(wd))
        return wd
