#all required modules are imported in Init.py?
import FreeCADGui

class ResultControlTaskPanel:
    '''The control for the displacement post-processing'''
    def __init__(self):
        self.form = FreeCADGui.PySideUic.loadUi(FreeCAD.getHomePath() + "Mod/Fem/ShowDisplacement.ui")

        #Connect Signals and Slots
        QtCore.QObject.connect(self.form.rb_none, QtCore.SIGNAL("toggled(bool)"), self.none_selected)
        QtCore.QObject.connect(self.form.rb_x_displacement, QtCore.SIGNAL("toggled(bool)"), self.x_displacement_selected)
        QtCore.QObject.connect(self.form.rb_y_displacement, QtCore.SIGNAL("toggled(bool)"), self.y_displacement_selected)
        QtCore.QObject.connect(self.form.rb_z_displacement, QtCore.SIGNAL("toggled(bool)"), self.z_displacement_selected)
        QtCore.QObject.connect(self.form.rb_abs_displacement, QtCore.SIGNAL("toggled(bool)"), self.abs_displacement_selected)
        QtCore.QObject.connect(self.form.rb_vm_stress, QtCore.SIGNAL("toggled(bool)"), self.vm_stress_selected)

        QtCore.QObject.connect(self.form.cb_show_displacement, QtCore.SIGNAL("clicked(bool)"), self.show_displacement)
        QtCore.QObject.connect(self.form.hsb_displacement_factor, QtCore.SIGNAL("valueChanged(int)"), self.hsb_disp_factor_changed)
        QtCore.QObject.connect(self.form.sb_displacement_factor, QtCore.SIGNAL("valueChanged(int)"), self.sb_disp_factor_changed)
        QtCore.QObject.connect(self.form.sb_displacement_factor_max, QtCore.SIGNAL("valueChanged(int)"), self.sb_disp_factor_max_changed)

        self.update()
        self.restore_result_dialog()

    def restore_result_dialog(self):
        try:
            rt = FreeCAD.FEM_dialog["results_type"]
            if rt == "None":
                self.form.rb_none.setChecked(True)
                self.none_selected(True)
            elif rt == "Uabs":
                self.form.rb_abs_displacement.setChecked(True)
                self.abs_displacement_selected(True)
            elif rt == "U1":
                self.form.rb_x_displacement.setChecked(True)
                self.x_displacement_selected(True)
            elif rt == "U2":
                self.form.rb_y_displacement.setChecked(True)
                self.y_displacement_selected(True)
            elif rt == "U3":
                self.form.rb_z_displacement.setChecked(True)
                self.z_displacement_selected(True)
            elif rt == "Sabs":
                self.form.rb_vm_stress.setChecked(True)
                self.vm_stress_selected(True)

            sd = FreeCAD.FEM_dialog["show_disp"]
            self.form.cb_show_displacement.setChecked(sd)
            self.show_displacement(sd)

            df = FreeCAD.FEM_dialog["disp_factor"]
            dfm = FreeCAD.FEM_dialog["disp_factor_max"]
            self.form.hsb_displacement_factor.setMaximum(dfm)
            self.form.hsb_displacement_factor.setValue(df)
            self.form.sb_displacement_factor_max.setValue(dfm)
            self.form.sb_displacement_factor.setValue(df)
        except:
            FreeCAD.FEM_dialog = {"results_type": "None", "show_disp": False,
                                  "disp_factor": 0, "disp_factor_max": 100}

    def getStandardButtons(self):
        return int(QtGui.QDialogButtonBox.Close)

    def get_result_stats(self, type_name, analysis=None):
        if analysis is None:
            analysis = FemGui.getActiveAnalysis()
        for i in analysis.Member:
            if (i.isDerivedFrom("Fem::FemResultObject")) and ("Stats" in i.PropertiesList):
                match_table = {"U1": (i.Stats[0], i.Stats[1], i.Stats[2]),
                               "U2": (i.Stats[3], i.Stats[4], i.Stats[5]),
                               "U3": (i.Stats[6], i.Stats[7], i.Stats[8]),
                               "Uabs": (i.Stats[9], i.Stats[10], i.Stats[11]),
                               "Sabs": (i.Stats[12], i.Stats[13], i.Stats[14]),
                               "None": (0.0, 0.0, 0.0)}
                return match_table[type_name]
        return (0.0, 0.0, 0.0)

    def none_selected(self, state):
        FreeCAD.FEM_dialog["results_type"] = "None"
        self.set_result_stats("mm", 0.0, 0.0, 0.0)
        fea = FemTools()
        fea.reset_mesh_color()

    def abs_displacement_selected(self, state):
        FreeCAD.FEM_dialog["results_type"] = "Uabs"
        self.select_displacement_type("Uabs")

    def x_displacement_selected(self, state):
        FreeCAD.FEM_dialog["results_type"] = "U1"
        self.select_displacement_type("U1")

    def y_displacement_selected(self, state):
        FreeCAD.FEM_dialog["results_type"] = "U2"
        self.select_displacement_type("U2")

    def z_displacement_selected(self, state):
        FreeCAD.FEM_dialog["results_type"] = "U3"
        self.select_displacement_type("U3")

    def vm_stress_selected(self, state):
        FreeCAD.FEM_dialog["results_type"] = "Sabs"
        QApplication.setOverrideCursor(Qt.WaitCursor)
        self.MeshObject.ViewObject.setNodeColorByScalars(self.result_object.ElementNumbers, self.result_object.StressValues)
        (minm, avg, maxm) = self.get_result_stats("Sabs")
        self.set_result_stats("MPa", minm, avg, maxm)
        QtGui.qApp.restoreOverrideCursor()

    def select_displacement_type(self, disp_type):
        QApplication.setOverrideCursor(Qt.WaitCursor)
        if disp_type == "Uabs":
            self.MeshObject.ViewObject.setNodeColorByScalars(self.result_object.ElementNumbers, self.result_object.DisplacementLengths)
        else:
            match = {"U1": 0, "U2": 1, "U3": 2}
            d = zip(*self.result_object.DisplacementVectors)
            displacements = list(d[match[disp_type]])
            self.MeshObject.ViewObject.setNodeColorByScalars(self.result_object.ElementNumbers, displacements)
        (minm, avg, maxm) = self.get_result_stats(disp_type)
        self.set_result_stats("mm", minm, avg, maxm)
        QtGui.qApp.restoreOverrideCursor()

    def set_result_stats(self, unit, minm, avg, maxm):
        self.form.le_min.setProperty("unit", unit)
        self.form.le_min.setText("{:.6} {}".format(minm, unit))
        self.form.le_avg.setProperty("unit", unit)
        self.form.le_avg.setText("{:.6} {}".format(avg, unit))
        self.form.le_max.setProperty("unit", unit)
        self.form.le_max.setText("{:.6} {}".format(maxm, unit))

    def update_displacement(self, factor=None):
        if factor is None:
            if FreeCAD.FEM_dialog["show_disp"]:
                factor = self.form.hsb_displacement_factor.value()
            else:
                factor = 0.0
        self.MeshObject.ViewObject.applyDisplacement(factor)

    def show_displacement(self, checked):
        QApplication.setOverrideCursor(Qt.WaitCursor)
        FreeCAD.FEM_dialog["show_disp"] = checked
        if "result_object" in FreeCAD.FEM_dialog:
            if FreeCAD.FEM_dialog["result_object"] != self.result_object:
                self.update_displacement()
        FreeCAD.FEM_dialog["result_object"] = self.result_object
        self.MeshObject.ViewObject.setNodeDisplacementByVectors(self.result_object.ElementNumbers, self.result_object.DisplacementVectors)
        self.update_displacement()
        QtGui.qApp.restoreOverrideCursor()

    def hsb_disp_factor_changed(self, value):
        self.form.sb_displacement_factor.setValue(value)
        self.update_displacement()

    def sb_disp_factor_max_changed(self, value):
        FreeCAD.FEM_dialog["disp_factor_max"] = value
        self.form.hsb_displacement_factor.setMaximum(value)

    def sb_disp_factor_changed(self, value):
        FreeCAD.FEM_dialog["disp_factor"] = value
        self.form.hsb_displacement_factor.setValue(value)

    def update(self):
        self.MeshObject = None
        self.result_object = get_results_object(FreeCADGui.Selection.getSelection())

        for i in FemGui.getActiveAnalysis().Member:
            if i.isDerivedFrom("Fem::FemMeshObject"):
                self.MeshObject = i
                break

    def accept(self):
        FreeCADGui.Control.closeDialog()

    def reject(self):
        FreeCADGui.Control.closeDialog()

# Helpers