/***************************************************************************
 *   Copyright (c) 2013 Jan Rheinländer <jrheinlaender[at]users.sourceforge.net>     *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/


#include "PreCompiled.h"

#ifndef _PreComp_
#include <gp_Pnt.hxx>
#include <gp_Pln.hxx>
#include <gp_Lin.hxx>
#include <TopoDS.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <Precision.hxx>
#endif

#include "FemConstraintPressure.h"

#include <Mod/Part/App/PartFeature.h>
#include <Base/Console.h>

using namespace Fem;

PROPERTY_SOURCE(Fem::ConstraintPressure, Fem::Constraint);

ConstraintPressure::ConstraintPressure()
{
    ADD_PROPERTY(Pressure,(0.0));
    ADD_PROPERTY_TYPE(Direction,(0),"ConstraintPressure",(App::PropertyType)(App::Prop_None),
                      "Element giving direction of constraint");
    ADD_PROPERTY(Reversed,(0));
    ADD_PROPERTY_TYPE(Points,(Base::Vector3d()),"ConstraintPressure",App::PropertyType(App::Prop_ReadOnly|App::Prop_Output),
                      "Points where arrows are drawn");
    ADD_PROPERTY_TYPE(Normals,(Base::Vector3d()),"ConstraintPressure",App::PropertyType(App::Prop_ReadOnly|App::Prop_Output),
                                                                             "Normals where symbols are drawn");
    //ADD_PROPERTY_TYPE(DirectionVector,(Base::Vector3d(0,0,1)),"ConstraintPressure",App::PropertyType(App::Prop_ReadOnly|App::Prop_Output),
    //                  "Direction of arrows");
    //naturalDirectionVector = Base::Vector3d(0,0,0); // by default use the null vector to indication an invalid value
    Points.setValues(std::vector<Base::Vector3d>());
    Normals.setValues(std::vector<Base::Vector3d>());
}

App::DocumentObjectExecReturn *ConstraintPressure::execute(void)
{
    return Constraint::execute();
}

void ConstraintPressure::onChanged(const App::Property* prop)
{
    // Note: If we call this at the end, then the arrows are not oriented correctly initially
    // because the NormalDirection has not been calculated yet
    Constraint::onChanged(prop);

    if (prop == &References) {
        std::vector<Base::Vector3d> points;
        std::vector<Base::Vector3d> normals;
        if (getPoints(points, normals)) {
            Points.setValues(points);
            Normals.setValues(normals);
        }
    } else if (prop == &Direction) {
        Base::Vector3d direction = getDirection(Direction);
        if (direction.Length() < Precision::Confusion())
            return;
        naturalDirectionVector = direction;
        if (Reversed.getValue())
            direction = -direction;
        DirectionVector.setValue(direction);
    } else if (prop == &Reversed) {
        // if the direction is invalid try to compute it again
        if (naturalDirectionVector.Length() < Precision::Confusion()) {
            naturalDirectionVector = getDirection(Direction);
        }
        if (naturalDirectionVector.Length() >= Precision::Confusion()) {
            if (Reversed.getValue() && (DirectionVector.getValue() == naturalDirectionVector)) {
                DirectionVector.setValue(-naturalDirectionVector);
            } else if (!Reversed.getValue() && (DirectionVector.getValue() != naturalDirectionVector)) {
                DirectionVector.setValue(naturalDirectionVector);
            }
        }
    } else if (prop == &NormalDirection) {
        // Set a default direction if no direction reference has been given
        if (Direction.getValue() == NULL) {
            Base::Vector3d direction = NormalDirection.getValue();
            if (Reversed.getValue())
                direction = -direction;
            DirectionVector.setValue(direction);
            naturalDirectionVector = direction;
        }
    }
}
