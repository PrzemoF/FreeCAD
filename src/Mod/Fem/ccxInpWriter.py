import FemGui
import FreeCAD
import os
import time
import sys
import Part


class inp_writer:
    def __init__(self, dir_name, mesh_obj, mat_obj, fixed_obj, force_obj):
        self.mesh_object = mesh_obj
        self.material_objects = mat_obj
        self.fixed_objects = fixed_obj
        self.force_objects = force_obj
        self.base_name = dir_name + '/' + self.mesh_object.Name
        self.file_name = self.base_name + '.inp'
        print 'CalculiX .inp file will be written to: ', self.file_name

    def write_calculix_input_file(self):
        print 'write_calculix_input_file'
        self.mesh_object.FemMesh.writeABAQUS(self.file_name)

        # reopen file with "append" and add the analysis definition
        inpfile = open(self.file_name, 'a')
        inpfile.write('\n\n')
        self.write_material_element_sets(inpfile)
        self.write_fixed_node_sets(inpfile)
        self.write_load_node_sets(inpfile)
        self.write_materials(inpfile)
        self.write_step_begin(inpfile)
        self.write_constraints_fixed(inpfile)
        self.write_constraints_force(inpfile)
        #self.write_face_load(inpfile)
        self.write_outputs_types(inpfile)
        self.write_step_end(inpfile)
        self.write_footer(inpfile)
        inpfile.close()
        return self.base_name

    def write_material_element_sets(self, f):
        f.write('\n***********************************************************\n')
        f.write('** Element sets for materials\n')
        f.write('** written by {} function\n'.format(sys._getframe().f_code.co_name))
        for m in self.material_objects:
            mat_obj = m['Object']
            mat_obj_name = mat_obj.Name
            mat_name = mat_obj.Material['Name'][:80]

            print mat_obj_name, ':  ', mat_name
            f.write('*ELSET,ELSET=' + mat_obj_name + '\n')
            if len(self.material_objects) == 1:
                f.write('Eall\n')
            else:
                if mat_obj_name == 'MechanicalMaterial':
                    f.write('Eall\n')

    def write_fixed_node_sets(self, f):
        f.write('\n***********************************************************\n')
        f.write('** Node set for fixed constraint\n')
        f.write('** written by {} function\n'.format(sys._getframe().f_code.co_name))
        for fobj in self.fixed_objects:
            fix_obj = fobj['Object']
            print fix_obj.Name
            f.write('*NSET,NSET=' + fix_obj.Name + '\n')
            for o, elem in fix_obj.References:
                fo = o.Shape.getElement(elem)
                n = []
                if fo.ShapeType == 'Face':
                    print '  Face Support (fixed face) on: ', elem
                    n = self.mesh_object.FemMesh.getNodesByFace(fo)
                elif fo.ShapeType == 'Edge':
                    print '  Line Support (fixed edge) on: ', elem
                    n = self.mesh_object.FemMesh.getNodesByEdge(fo)
                elif fo.ShapeType == 'Vertex':
                    print '  Point Support (fixed vertex) on: ', elem
                    n = self.mesh_object.FemMesh.getNodesByVertex(fo)
                for i in n:
                    f.write(str(i) + ',\n')

    def write_load_node_sets(self, f):
        f.write('\n***********************************************************\n')
        f.write('** Node sets for loads\n')
        f.write('** written by {} function\n'.format(sys._getframe().f_code.co_name))
        for fobj in self.force_objects:
            frc_obj = fobj['Object']
            print frc_obj.Name
            f.write('*NSET,NSET=' + frc_obj.Name + '\n')
            NbrForceNodes = 0
            for o, elem in frc_obj.References:
                fo = o.Shape.getElement(elem)
                n = []
                if fo.ShapeType == 'Edge':
                    print '  Line Load (edge load) on: ', elem
                    n = self.mesh_object.FemMesh.getNodesByEdge(fo)
                elif fo.ShapeType == 'Vertex':
                    print '  Point Load (vertex load) on: ', elem
                    n = self.mesh_object.FemMesh.getNodesByVertex(fo)
                for i in n:
                    f.write(str(i) + ',\n')
                    NbrForceNodes = NbrForceNodes + 1   # NodeSum of mesh-nodes of ALL reference shapes from force_object
            # calculate node load
            if NbrForceNodes == 0:
                print 'No Line Loads or Point Loads in the model'
            else:
                fobj['NodeLoad'] = (frc_obj.Force) / NbrForceNodes
                #  FIXME this method is incorrect, but we don't have anything else right now
                #  Please refer to thread "CLOAD and DLOAD for the detailed description
                #  http://forum.freecadweb.org/viewtopic.php?f=18&t=10692
                f.write('** concentrated load [N] distributed on all mesh nodes of the given shapes\n')
                f.write('** ' + str(frc_obj.Force) + ' N / ' + str(NbrForceNodes) + ' Nodes = ' + str(fobj['NodeLoad']) + ' N on each node\n')
            if frc_obj.Force == 0:
                print '  Warning --> Force = 0'

    def write_materials(self, f):
        f.write('\n***********************************************************\n')
        f.write('** Materials\n')
        f.write('** written by {} function\n'.format(sys._getframe().f_code.co_name))
        f.write('** Young\'s modulus unit is MPa = N/mm2\n')
        for m in self.material_objects:
            mat_obj = m['Object']
            # get material properties
            YM = FreeCAD.Units.Quantity(mat_obj.Material['YoungsModulus'])
            YM_in_MPa = YM.getValueAs('MPa')
            PR = float(mat_obj.Material['PoissonRatio'])
            mat_obj_name = mat_obj.Name
            mat_name = mat_obj.Material['Name'][:80]
            # write material properties
            f.write('*MATERIAL, NAME=' + mat_name + '\n')
            f.write('*ELASTIC \n')
            f.write('{}, '.format(YM_in_MPa))
            f.write('{0:.3f}\n'.format(PR))
            # write element properties
            if len(self.material_objects) == 1:
                f.write('*SOLID SECTION, ELSET=' + mat_obj_name + ', MATERIAL=' + mat_name + '\n')
            else:
                if mat_obj_name == 'MechanicalMaterial':
                    f.write('*SOLID SECTION, ELSET=' + mat_obj_name + ', MATERIAL=' + mat_name + '\n')

    def write_step_begin(self, f):
        f.write('\n***********************************************************\n')
        f.write('** One step is needed to calculate the mechanical analysis of FreeCAD\n')
        f.write('** loads are applied quasi-static, means without involving the time dimension\n')
        f.write('** written by {} function\n'.format(sys._getframe().f_code.co_name))
        f.write('*STEP\n')
        f.write('*STATIC\n')

    def write_constraints_fixed(self, f):
        f.write('\n***********************************************************\n')
        f.write('** Constaints\n')
        f.write('** written by {} function\n'.format(sys._getframe().f_code.co_name))
        for fixed_object in self.fixed_objects:
            fix_obj_name = fixed_object['Object'].Name
            f.write('*BOUNDARY\n')
            f.write(fix_obj_name + ',1\n')
            f.write(fix_obj_name + ',2\n')
            f.write(fix_obj_name + ',3\n\n')

    def write_constraints_force(self, f):
        f.write('\n***********************************************************\n')
        f.write('** Node loads\n')
        f.write('** written by {} function\n'.format(sys._getframe().f_code.co_name))
        for fobj in self.force_objects:
            frc_obj = fobj['Object']
            if 'NodeLoad' in fobj:
                node_load = fobj['NodeLoad']
                frc_obj_name = frc_obj.Name
                vec = frc_obj.DirectionVector
                f.write('*CLOAD\n')
                f.write('** force: ' + str(node_load) + ' N,  direction: ' + str(vec) + '\n')
                v1 = "{:.13E}".format(vec.x * node_load)
                v2 = "{:.13E}".format(vec.y * node_load)
                v3 = "{:.13E}".format(vec.z * node_load)
                f.write(frc_obj_name + ',1,' + v1 + '\n')
                f.write(frc_obj_name + ',2,' + v2 + '\n')
                f.write(frc_obj_name + ',3,' + v3 + '\n\n')

            sum_ref_face_area = 0
            for o, elem in frc_obj.References:
                elem_o = o.Shape.getElement(elem)
                if elem_o.ShapeType == 'Face':
                    sum_ref_face_area += elem_o.Area

            for o, elem in frc_obj.References:
                elem_o = o.Shape.getElement(elem)
                if elem_o.ShapeType == 'Face':
                    ref_face = elem_o
                    print frc_obj.Name
                    print '  AreaLoad (face load) on: ', o.Name, '.', elem
                    print '  CLOAD is used'
                    f.write('** ' + frc_obj.Name + '\n')
                    f.write('*CLOAD\n')
                    f.write('** node loads on: ' + o.Name + '.' + elem + '\n')

                    force_per_sum_ref_face_area = frc_obj.Force / sum_ref_face_area
                    vec = frc_obj.DirectionVector

                    face_table = {}  # { meshfaceID : ( nodeID, ... , nodeID ) }
                    # we onle need the meshfaces, but there is no method which only returns the faceIDs
                    vol_type = ''
                    volume_faces = self.mesh_object.FemMesh.getVolumesByFace(ref_face)
                    for mv,mf in volume_faces:
                        face_table[mf] = self.mesh_object.FemMesh.getElementNodes(mf)
                        if len(face_table[mf]) == 3:  # we just check the last face of the tabele!
                            vol_type = 'C3D4'
                        elif len(face_table[mf]) == 6:
                            vol_type = 'C3D10'

                    # for every node of every meshface calulate the appropriate node_areas
                    node_area_table = []  # [ (nodeID,Area), ... , (nodeID,Area) ]  some nodes will have more than one entries
                    node_sumarea_table = {}  # { nodeID : Area, ... , nodeID:Area }  AreaSum for each node, one entry for each node
                    for mf in face_table:
                        #print mf, ' --> ', face_table[mf]
                        if vol_type == 'C3D4':
                            P1 = self.mesh_object.FemMesh.Nodes[face_table[mf][0]]
                            P2 = self.mesh_object.FemMesh.Nodes[face_table[mf][1]]
                            P3 = self.mesh_object.FemMesh.Nodes[face_table[mf][2]]

                            #get the Area and CenterOfMass
                            L1 = Part.makeLine(P1,P2)
                            L2 = Part.makeLine(P2,P3)
                            L3 = Part.makeLine(P3,P1)
                            W = Part.Wire([L1,L2,L3])
                            F = Part.Face(W)
                            A = F.Area
                            PS = F.CenterOfMass

                            #helperpoints
                            PL1 = L1.CenterOfMass
                            PL2 = L2.CenterOfMass
                            PL3 = L3.CenterOfMass

                            #cornerface1, Point face_table[ef][0]
                            C1L1 = Part.makeLine(P1,PL1)
                            C1L2 = Part.makeLine(PL1,PS)
                            C1L3 = Part.makeLine(PS,PL3)
                            C1L4 = Part.makeLine(PL3,P1)
                            C1W = Part.Wire([C1L1,C1L2,C1L3,C1L4])
                            C1F = Part.Face(C1W)
                            C1A = C1F.Area

                            #cornerface2, Point face_table[ef][1]
                            C2L1 = Part.makeLine(P2,PL2)
                            C2L2 = Part.makeLine(PL2,PS)
                            C2L3 = Part.makeLine(PS,PL1)
                            C2L4 = Part.makeLine(PL1,P2)
                            C2W = Part.Wire([C2L1,C2L2,C2L3,C2L4])
                            C2F = Part.Face(C2W)
                            C2A = C2F.Area

                            #cornerface3, Point face_table[ef][2]
                            C3L1 = Part.makeLine(P3,PL3)
                            C3L2 = Part.makeLine(PL3,PS)
                            C3L3 = Part.makeLine(PS,PL2)
                            C3L4 = Part.makeLine(PL2,P3)
                            C3W = Part.Wire([C3L1,C3L2,C3L3,C3L4])
                            C3F = Part.Face(C3W)
                            C3A = C3F.Area

                            #node_area_table
                            node_area_table.append((face_table[mf][0], C1A))
                            node_area_table.append((face_table[mf][1], C2A))
                            node_area_table.append((face_table[mf][2], C3A))
                            print '  ', A, '  ', (C1A + C2A + C3A)

                        if vol_type == 'C3D10':
                            # workaraound FIXME
                            # instead of the quadratic shape functions
                            # linear lines are used to connect all nodes
                            # the the corner nodes get to much, the middle nodes to less force
                            P1 = self.mesh_object.FemMesh.Nodes[face_table[mf][0]]
                            P2 = self.mesh_object.FemMesh.Nodes[face_table[mf][1]]
                            P3 = self.mesh_object.FemMesh.Nodes[face_table[mf][2]]
                            P4 = self.mesh_object.FemMesh.Nodes[face_table[mf][3]]
                            P5 = self.mesh_object.FemMesh.Nodes[face_table[mf][4]]
                            P6 = self.mesh_object.FemMesh.Nodes[face_table[mf][5]]

                            #get the Area and CenterOfMass
                            L1 = Part.makeLine(P1,P4)
                            L2 = Part.makeLine(P4,P2)
                            L3 = Part.makeLine(P2,P5)
                            L4 = Part.makeLine(P5,P3)
                            L5 = Part.makeLine(P3,P6)
                            L6 = Part.makeLine(P6,P1)
                            W = Part.Wire([L1,L2,L3,L4,L5,L6])
                            F = Part.Face(W)
                            A = F.Area
                            PS = F.CenterOfMass

                            #helperpoints
                            PL1 = L1.CenterOfMass
                            PL2 = L2.CenterOfMass
                            PL3 = L3.CenterOfMass
                            PL4 = L4.CenterOfMass
                            PL5 = L5.CenterOfMass
                            PL6 = L6.CenterOfMass

                            #cornerface1, Point face_table[ef][0]
                            C1L1 = Part.makeLine(P1,PL1)
                            C1L2 = Part.makeLine(PL1,PS)
                            C1L3 = Part.makeLine(PS,PL6)
                            C1L4 = Part.makeLine(PL6,P1)
                            C1W = Part.Wire([C1L1,C1L2,C1L3,C1L4])
                            C1F = Part.Face(C1W)
                            C1A = C1F.Area

                            #middlepointface4, Point face_table[ef][4]
                            C4L1 = Part.makeLine(P4,PL2)
                            C4L2 = Part.makeLine(PL2,PS)
                            C4L3 = Part.makeLine(PS,PL1)
                            C4L4 = Part.makeLine(PL1,P4)
                            C4W = Part.Wire([C4L1,C4L2,C4L3,C4L4])
                            C4F = Part.Face(C4W)
                            C4A = C4F.Area

                            #cornerface2, Point face_table[ef][1]
                            C2L1 = Part.makeLine(P2,PL3)
                            C2L2 = Part.makeLine(PL3,PS)
                            C2L3 = Part.makeLine(PS,PL2)
                            C2L4 = Part.makeLine(PL2,P2)
                            C2W = Part.Wire([C2L1,C2L2,C2L3,C2L4])
                            C2F = Part.Face(C2W)
                            C2A = C2F.Area

                            #middlepointface5, Point face_table[ef][5]
                            C5L1 = Part.makeLine(P5,PL4)
                            C5L2 = Part.makeLine(PL4,PS)
                            C5L3 = Part.makeLine(PS,PL3)
                            C5L4 = Part.makeLine(PL3,P5)
                            C5W = Part.Wire([C5L1,C5L2,C5L3,C5L4])
                            C5F = Part.Face(C5W)
                            C5A = C5F.Area

                            #cornerface3, Point face_table[ef][2]
                            C3L1 = Part.makeLine(P3,PL5)
                            C3L2 = Part.makeLine(PL5,PS)
                            C3L3 = Part.makeLine(PS,PL4)
                            C3L4 = Part.makeLine(PL4,P3)
                            C3W = Part.Wire([C3L1,C3L2,C3L3,C3L4])
                            C3F = Part.Face(C3W)
                            C3A = C3F.Area

                            #middlepointface6, Point face_table[ef][6]
                            C6L1 = Part.makeLine(P6,PL6)
                            C6L2 = Part.makeLine(PL6,PS)
                            C6L3 = Part.makeLine(PS,PL5)
                            C6L4 = Part.makeLine(PL5,P6)
                            C6W = Part.Wire([C6L1,C6L2,C6L3,C6L4])
                            C6F = Part.Face(C6W)
                            C6A = C6F.Area

                            #node_area_table
                            node_area_table.append((face_table[mf][0], C1A))
                            node_area_table.append((face_table[mf][1], C2A))
                            node_area_table.append((face_table[mf][2], C3A))
                            node_area_table.append((face_table[mf][3], C4A))
                            node_area_table.append((face_table[mf][4], C5A))
                            node_area_table.append((face_table[mf][5], C6A))
                            print '  ', A, '  ', (C1A + C2A + C3A + C4A + C5A + C6A)

                    #node_sumarea_table
                    for n, A in node_area_table:
                        #print n, ' --> ', A
                        if n in node_sumarea_table:
                            node_sumarea_table[n] = node_sumarea_table[n] + A
                        else:
                            node_sumarea_table[n] = A

                    # debug: check the total sum of all node_sumarea_table == ref_face.Area
                    sum_node_areas = 0
                    for n in node_sumarea_table:
                        #print n, ' --> ', node_sumarea_table[n]
                        sum_node_areas = sum_node_areas + node_sumarea_table[n]
                    if 0.9999 < sum_node_areas / ref_face.Area < 1.0001:
                        pass
                    else:
                        print 'ERROR:  ', sum_node_areas, ' != ', ref_face.Area

                    # write the CLOAD lines to file
                    for n in sorted(node_sumarea_table):
                        node_load = node_sumarea_table[n] * force_per_sum_ref_face_area
                        if (vec.x != 0.0):
                            v1 = "{:.13E}".format(vec.x * node_load)
                            f.write(str(n) + ',1,' + v1 + '\n')
                        if (vec.y != 0.0):
                            v2 = "{:.13E}".format(vec.y * node_load)
                            f.write(str(n) + ',2,' + v2 + '\n')
                        if (vec.z != 0.0):
                            v3 = "{:.13E}".format(vec.z * node_load)
                            f.write(str(n) + ',3,' + v3 + '\n')
                f.write('\n')
            f.write('\n')

    def write_face_load(self, f):
        f.write('\n***********************************************************\n')
        f.write('** Element + CalculiX face + load in [MPa]\n')
        f.write('** written by {} function\n'.format(sys._getframe().f_code.co_name))
        for fobj in self.force_objects:
            frc_obj = fobj['Object']
            f.write('*DLOAD\n')
            for o, e in frc_obj.References:
                elem = o.Shape.getElement(e)
                if elem.ShapeType == 'Face':
                    v = self.mesh_object.FemMesh.getccxVolumesByFace(elem)
                    f.write("** Load on face {}\n".format(e))
                    for i in v:
                        f.write("{},P{},{}\n".format(i[0], i[1], frc_obj.Force))

    def write_outputs_types(self, f):
        f.write('\n***********************************************************\n')
        f.write('** Outputs --> frd file\n')
        f.write('** written by {} function\n'.format(sys._getframe().f_code.co_name))
        f.write('*NODE FILE\n')
        f.write('U\n')
        f.write('*EL FILE\n')
        f.write('S, E\n')
        f.write('** outputs --> dat file\n')
        f.write('*NODE PRINT , NSET=Nall \n')
        f.write('U \n')
        f.write('*EL PRINT , ELSET=Eall \n')
        f.write('S \n')

    def write_step_end(self, f):
        f.write('\n***********************************************************\n')
        f.write('** written by {} function\n'.format(sys._getframe().f_code.co_name))
        f.write('*END STEP \n')

    def write_footer(self, f):
        FcVersionInfo = FreeCAD.Version()
        f.write('\n***********************************************************\n')
        f.write('** CalculiX Input file\n')
        f.write('** written by {} function\n'.format(sys._getframe().f_code.co_name))
        f.write('**\n')
        f.write('**   written by    --> FreeCAD ' + FcVersionInfo[0] + '.' + FcVersionInfo[1] + '.' + FcVersionInfo[2] + '\n')
        f.write('**   written on    --> ' + time.ctime() + '\n')
        f.write('**   file name     --> ' + os.path.basename(FreeCAD.ActiveDocument.FileName) + '\n')
        f.write('**   analysis name --> ' + FemGui.getActiveAnalysis().Name + '\n')
        f.write('**\n')
        f.write('**\n')
        f.write('**   Units\n')
        f.write('**\n')
        f.write('**   Geometry (mesh data)        --> mm\n')
        f.write("**   Materials (Young's modulus) --> N/mm2 = MPa\n")
        f.write('**   Loads (nodal loads)         --> N\n')
        f.write('**\n')
