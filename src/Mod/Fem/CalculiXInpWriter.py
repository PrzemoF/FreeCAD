import FemGui
import FreeCAD
import FreeCADGui
import os
import time


class writer:
    def __init__(self, dir_name, mesh_object, fixed_objects, force_objects, material_object):
        self.dir_name = dir_name
        self.mesh_object = mesh_object
        self.fixed_objects = fixed_objects
        self.force_objects = force_objects
        self.material_object = material_object
        pass

    def write_calculix_input_file(self):
        print 'write_calculix_input_file'
        print 'CalculiX run directory: ',self.dir_name

        matmap = self.material_object.Material

        self.basename = self.dir_name + '/' + self.mesh_object.Name
        filename = self.basename + '.inp'

        # self.femConsoleMessage(self.basename)
        # self.femConsoleMessage("Write mesh...")

        # write mesh
        self.mesh_object.FemMesh.writeABAQUS(filename)

        # reopen file with "append" and add the analysis definition
        inpfile = open(filename, 'a')

        # self.femConsoleMessage("Write loads & Co...")

        # write fixed node sets
        inpfile.write('\n\n\n\n***********************************************************\n')
        inpfile.write('** node set for fixed constraint\n')
        for fixed_object in self.fixed_objects:
            print fixed_object['Object'].Name
            inpfile.write('*NSET,NSET=' + fixed_object['Object'].Name + '\n')
            for o,f in fixed_object['Object'].References:
                fo = o.Shape.getElement(f)
                n = []
                if fo.ShapeType == 'Face':
                    print '  Face Support (fixed face) on: ', f
                    n = self.mesh_object.FemMesh.getNodesByFace(fo)
                elif fo.ShapeType == 'Edge':
                    print '  Line Support (fixed edge) on: ', f
                    n = self.mesh_object.FemMesh.getNodesByEdge(fo)
                elif fo.ShapeType == 'Vertex':
                    print '  Point Support (fixed vertex) on: ', f
                    n = self.mesh_object.FemMesh.getNodesByVertex(fo)
                for i in n:
                    inpfile.write(str(i) + ',\n')
            inpfile.write('\n\n')

        # write load node sets and calculate node loads
        inpfile.write('\n\n***********************************************************\n')
        inpfile.write('** node sets for loads\n')
        for force_object in self.force_objects:
            print force_object['Object'].Name
            inpfile.write('*NSET,NSET=' + force_object['Object'].Name + '\n')
            NbrForceNodes = 0
            for o,f in force_object['Object'].References:
                fo = o.Shape.getElement(f)
                n = []
                if fo.ShapeType == 'Face':
                    print '  AreaLoad (face load) on: ', f
                    n = self.mesh_object.FemMesh.getNodesByFace(fo)
                elif fo.ShapeType == 'Edge':
                    print '  Line Load (edge load) on: ', f
                    n = self.mesh_object.FemMesh.getNodesByEdge(fo)
                elif fo.ShapeType == 'Vertex':
                    print '  Point Load (vertex load) on: ', f
                    n = self.mesh_object.FemMesh.getNodesByVertex(fo)
                for i in n:
                    inpfile.write(str(i) + ',\n')
                    NbrForceNodes = NbrForceNodes + 1   # NodeSum of mesh-nodes of ALL reference shapes from force_object
            # calculate node load
            if NbrForceNodes == 0:
                print '  Warning --> no FEM-Mesh-node to apply the load to was found?'
            else:
                force_object['NodeLoad'] = (force_object['Object'].Force) / NbrForceNodes
                inpfile.write('** concentrated load [N] distributed on all mesh nodes of the given shapes\n')
                inpfile.write('** ' + str(force_object['Object'].Force) + ' N / ' + str(NbrForceNodes) + ' Nodes = ' + str(force_object['NodeLoad']) + ' N on each node\n')
            if force_object['Object'].Force == 0:
                print '  Warning --> Force = 0'
            inpfile.write('\n\n')

        # get material properties
        YM = FreeCAD.Units.Quantity(self.material_object.Material['YoungsModulus'])
        if YM.Unit.Type == '':
            print 'Material "YoungsModulus" has no Unit, asuming kPa!'
            YM = FreeCAD.Units.Quantity(YM.Value, FreeCAD.Units.Unit('Pa'))
        else:
            print 'YM unit: ', YM.Unit.Type
        print 'YM = ', YM

        PR = float(self.material_object.Material['PoissonRatio'])
        print 'PR = ', PR

        material_name = matmap['Name'][:80]
        # write material properties
        inpfile.write('\n\n***********************************************************\n')
        inpfile.write('** material\n')
        inpfile.write('** unit is MPa = N/mm2\n')
        inpfile.write('*MATERIAL, Name=' + material_name + '\n')
        inpfile.write('*ELASTIC \n')
        inpfile.write('{0:.3f}, '.format(YM.Value * 1E-3))
        inpfile.write('{0:.3f}\n'.format(PR))
        inpfile.write('*SOLID SECTION, Elset=Eall, Material=' + material_name + '\n')

        # write step beginn
        inpfile.write('\n\n\n\n***********************************************************\n')
        inpfile.write('** one step is needed to calculate the mechanical analysis of FreeCAD\n')
        inpfile.write('** loads are applied quasi-static, means without involving the time dimension\n')
        inpfile.write('*STEP\n')
        inpfile.write('*STATIC\n\n')

        # write constaints
        inpfile.write('\n** constaints\n')
        for fixed_object in self.fixed_objects:
            inpfile.write('*BOUNDARY\n')
            inpfile.write(fixed_object['Object'].Name + ',1\n')
            inpfile.write(fixed_object['Object'].Name + ',2\n')
            inpfile.write(fixed_object['Object'].Name + ',3\n\n')

        # write loads
        #inpfile.write('*DLOAD\n')
        #inpfile.write('Eall,NEWTON\n')
        inpfile.write('\n** loads\n')
        inpfile.write('** node loads, see load node sets for how the value is calculated!\n')
        for force_object in self.force_objects:
            if 'NodeLoad' in force_object:
                vec = force_object['Object'].DirectionVector
                inpfile.write('*CLOAD\n')
                inpfile.write('** force: ' + str(force_object['NodeLoad']) + ' N,  direction: ' + str(vec) + '\n')
                v1 = "{:.15}".format(repr(vec.x * force_object['NodeLoad']))
                v2 = "{:.15}".format(repr(vec.y * force_object['NodeLoad']))
                v3 = "{:.15}".format(repr(vec.z * force_object['NodeLoad']))
                inpfile.write(force_object['Object'].Name + ',1,' + v1 + '\n')
                inpfile.write(force_object['Object'].Name + ',2,' + v2 + '\n')
                inpfile.write(force_object['Object'].Name + ',3,' + v3 + '\n\n')

        # write outputs, both are needed by FreeCAD
        inpfile.write('\n** outputs --> frd file\n')
        inpfile.write('*NODE FILE\n')
        inpfile.write('U\n')
        inpfile.write('*EL FILE\n')
        inpfile.write('S, E\n')
        inpfile.write('** outputs --> dat file\n')
        inpfile.write('*NODE PRINT , NSET=Nall \n')
        inpfile.write('U \n')
        inpfile.write('*EL PRINT , ELSET=Eall \n')
        inpfile.write('S \n')
        inpfile.write('\n\n')

        # write step end
        inpfile.write('*END STEP \n')

        # write some informations
        FcVersionInfo = FreeCAD.Version()
        inpfile.write('\n\n\n\n***********************************************************\n')
        inpfile.write('**\n')
        inpfile.write('**   CalculiX Inputfile\n')
        inpfile.write('**\n')
        inpfile.write('**   written by    --> FreeCAD ' + FcVersionInfo[0] + '.' + FcVersionInfo[1] + '.' + FcVersionInfo[2] + '\n')
        inpfile.write('**   written on    --> ' + time.ctime() + '\n')
        inpfile.write('**   file name     --> ' + os.path.basename(FreeCAD.ActiveDocument.FileName) + '\n')
        inpfile.write('**   analysis name --> ' + FemGui.getActiveAnalysis().Name + '\n')
        inpfile.write('**\n')
        inpfile.write('**\n')
        inpfile.write('**   Units\n')
        inpfile.write('**\n')
        inpfile.write('**   Geometry (mesh data)        --> mm\n')
        inpfile.write("**   Materials (young's modulus) --> N/mm2 = MPa\n")
        inpfile.write('**   Loads (nodal loads)         --> N\n')
        inpfile.write('**\n')
        inpfile.write('**\n')

        inpfile.close()
        # self.femConsoleMessage("Write completed.")
        return self.basename

FreeCADGui.addModule("CalculiXInpWriter")
