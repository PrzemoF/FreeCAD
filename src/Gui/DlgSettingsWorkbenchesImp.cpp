/***************************************************************************
 *   Copyright (c) 2015 FreeCAD Developers                                 *
 *   Autor: Przemo Firszt <przemo@firszt.eu>                               *
 *   Based on DlgSettingsUnitsImp.cpp file                                 *
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
# include <QRegExp>
#endif

#include "Action.h"
#include <Application.h>
#include "DlgSettingsWorkbenchesImp.h"
#include "NavigationStyle.h"
#include "PrefWidgets.h"
#include "ui_DlgSettingsWorkbenches.h"
#include <App/Application.h>
#include <Base/Console.h>
#include <Base/Parameter.h>

using namespace Gui::Dialog;
using namespace Base;

DlgSettingsWorkbenchesImp::DlgSettingsWorkbenchesImp(QWidget* parent)
    : PreferencePage( parent ), ui(new Ui_DlgSettingsWorkbenches)
{
    ui->setupUi(this);
}

DlgSettingsWorkbenchesImp::~DlgSettingsWorkbenchesImp()
{
    delete ui;
}

void DlgSettingsWorkbenchesImp::saveSettings()
{
    ParameterGrp::handle hGrp = App::GetApplication().GetParameterGroupByPath
        ("User parameter:BaseApp/Preferences/Workbenches");
    hGrp->SetBool("<none>", ui->cb_none_wb->isChecked());
    hGrp->SetBool("Arch", ui->cb_arch_wb->isChecked());
    hGrp->SetBool("Complete", ui->cb_complete_wb->isChecked());
    hGrp->SetBool("Draft", ui->cb_draft_wb->isChecked());
    hGrp->SetBool("Drawing", ui->cb_drawing_wb->isChecked());
    hGrp->SetBool("FEM", ui->cb_fem_wb->isChecked());
    hGrp->SetBool("Image", ui->cb_image_wb->isChecked());
    hGrp->SetBool("Inspection", ui->cb_inspection_wb->isChecked());
    hGrp->SetBool("Mesh design", ui->cb_mesh_design_wb->isChecked());
    hGrp->SetBool("OpenSCAD", ui->cb_openscad_wb->isChecked());
    hGrp->SetBool("Part", ui->cb_part_wb->isChecked());
    hGrp->SetBool("Part Design", ui->cb_part_design_wb->isChecked());
    hGrp->SetBool("Plot", ui->cb_plot_wb->isChecked());
    hGrp->SetBool("Points", ui->cb_points_wb->isChecked());
    hGrp->SetBool("Raytracing", ui->cb_raytracing_wb->isChecked());
    hGrp->SetBool("Reverse Engineering", ui->cb_reverse_engineering_wb->isChecked());
    hGrp->SetBool("Robot", ui->cb_robot_wb->isChecked());
    hGrp->SetBool("Ship", ui->cb_ship_wb->isChecked());
    hGrp->SetBool("Sketcher", ui->cb_sketcher_wb->isChecked());
    hGrp->SetBool("Spreadsheet", ui->cb_spreadsheet_wb->isChecked());
    hGrp->SetBool("Start", ui->cb_start_wb->isChecked());
    hGrp->SetBool("Test framework", ui->cb_test_framework_wb->isChecked());
    hGrp->SetBool("Web", ui->cb_web_wb->isChecked());
}

void DlgSettingsWorkbenchesImp::loadSettings()
{
    ParameterGrp::handle hGrp = App::GetApplication().GetParameterGroupByPath
        ("User parameter:BaseApp/Preferences/Workbenches");
    ui->cb_none_wb->setChecked(hGrp->GetBool("<none>", true));
    ui->cb_arch_wb->setChecked(hGrp->GetBool("Arch", true));
    ui->cb_complete_wb->setChecked(hGrp->GetBool("Complete", true));
    ui->cb_draft_wb->setChecked(hGrp->GetBool("Draft", true));
    ui->cb_drawing_wb->setChecked(hGrp->GetBool("Drawing", true));
    ui->cb_fem_wb->setChecked(hGrp->GetBool("FEM", true));
    ui->cb_image_wb->setChecked(hGrp->GetBool("Image", true));
    ui->cb_inspection_wb->setChecked(hGrp->GetBool("Inspection", true));
    ui->cb_mesh_design_wb->setChecked(hGrp->GetBool("Mesh design", true));
    ui->cb_openscad_wb->setChecked(hGrp->GetBool("OpenSCAD", true));
    ui->cb_part_wb->setChecked(hGrp->GetBool("Part", true));
    ui->cb_part_design_wb->setChecked(hGrp->GetBool("Part Design", true));
    ui->cb_plot_wb->setChecked(hGrp->GetBool("Plot", true));
    ui->cb_points_wb->setChecked(hGrp->GetBool("Points", true));
    ui->cb_raytracing_wb->setChecked(hGrp->GetBool("Raytracing", true));
    ui->cb_reverse_engineering_wb->setChecked(hGrp->GetBool("Reverse Engineering", true));
    ui->cb_robot_wb->setChecked(hGrp->GetBool("Robot", true));
    ui->cb_web_wb->setChecked(hGrp->GetBool("Ship", true));
    ui->cb_sketcher_wb->setChecked(hGrp->GetBool("Sketcher", true));
    ui->cb_spreadsheet_wb->setChecked(hGrp->GetBool("Spreadsheet", true));
    ui->cb_start_wb->setChecked(hGrp->GetBool("Start", true));
    ui->cb_test_framework_wb->setChecked(hGrp->GetBool("Test framework", true));
    ui->cb_web_wb->setChecked(hGrp->GetBool("Web", true));
}

void DlgSettingsWorkbenchesImp::changeEvent(QEvent *e)
{
    /*if (e->type() == QEvent::?????????) {
	//Call WorkbenchGroup::refreshWorkbenchList();
    }
    else {
        QWidget::changeEvent(e);
    }*/
}

#include "moc_DlgSettingsWorkbenchesImp.cpp"
