/***************************************************************************
 *   Copyright (c) 2015 Przemo Firszt <przemo@firszt.eu>                   *
 *   Based on Force constraint                                             *
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
# include <sstream>
# include <QRegExp>
# include <QTextStream>
# include <QMessageBox>
# include <Precision.hxx>
# include <TopoDS.hxx>
# include <BRepAdaptor_Surface.hxx>
# include <Geom_Plane.hxx>
# include <gp_Pln.hxx>
# include <gp_Ax1.hxx>
# include <BRepAdaptor_Curve.hxx>
# include <Geom_Line.hxx>
# include <gp_Lin.hxx>
#endif

#include "ui_TaskFemConstraintNormalStress.h"
#include "TaskFemConstraintNormalStress.h"
#include <App/Application.h>
#include <App/Document.h>
#include <App/PropertyGeo.h>
#include <Gui/Application.h>
#include <Gui/Document.h>
#include <Gui/BitmapFactory.h>
#include <Gui/ViewProvider.h>
#include <Gui/WaitCursor.h>
#include <Gui/Selection.h>
#include <Gui/Command.h>
#include <Mod/Fem/App/FemConstraintNormalStress.h>
#include <Mod/Part/App/PartFeature.h>

#include <Base/Console.h>

using namespace FemGui;
using namespace Gui;

/* TRANSLATOR FemGui::TaskFemConstraintNormalStress */

TaskFemConstraintNormalStress::TaskFemConstraintNormalStress(ViewProviderFemConstraintNormalStress *ConstraintView,QWidget *parent)
    : TaskFemConstraint(ConstraintView, parent, "Fem_ConstraintNormalStress")
{
    // we need a separate container widget to add all controls to
    proxy = new QWidget(this);
    ui = new Ui_TaskFemConstraintNormalStress();
    ui->setupUi(proxy);
    QMetaObject::connectSlotsByName(this);

    // Create a context menu for the listview of the references
    QAction* action = new QAction(tr("Delete"), ui->lw_references);
    action->connect(action, SIGNAL(triggered()),
                    this, SLOT(onReferenceDeleted()));
    ui->lw_references->addAction(action);
    ui->lw_references->setContextMenuPolicy(Qt::ActionsContextMenu);

    connect(ui->if_normal_stress, SIGNAL(valueChanged(double)),
            this, SLOT(onNormalStressChanged(double)));
    connect(ui->b_add_reference, SIGNAL(pressed()),
            this, SLOT(onButtonReference()));
//    connect(ui->buttonDirection, SIGNAL(pressed()),
//            this, SLOT(onButtonDirection()));
    connect(ui->cb_reverse_direction, SIGNAL(toggled(bool)),
            this, SLOT(onCheckReverse(bool)));

    this->groupLayout()->addWidget(proxy);

    // Temporarily prevent unnecessary feature recomputes
    ui->if_normal_stress->blockSignals(true);
    ui->lw_references->blockSignals(true);
    ui->b_add_reference->blockSignals(true);
   // ui->buttonDirection->blockSignals(true);
    ui->cb_reverse_direction->blockSignals(true);

    // Get the feature data
    Fem::ConstraintNormalStress* pcConstraint = static_cast<Fem::ConstraintNormalStress*>(ConstraintView->getObject());
    double f = pcConstraint->NormalStress.getValue();
    std::vector<App::DocumentObject*> Objects = pcConstraint->References.getValues();
    std::vector<std::string> SubElements = pcConstraint->References.getSubValues();
    std::vector<std::string> dirStrings = pcConstraint->Direction.getSubValues();
    QString dir;
    if (!dirStrings.empty())
        dir = makeRefText(pcConstraint->Direction.getValue(), dirStrings.front());
    bool reversed = pcConstraint->Reversed.getValue();

    // Fill data into dialog elements
    ui->if_normal_stress->setMinimum(0);
    ui->if_normal_stress->setMaximum(FLOAT_MAX);
    ui->if_normal_stress->setValue(f);
    ui->lw_references->clear();
    for (std::size_t i = 0; i < Objects.size(); i++)
        ui->lw_references->addItem(makeRefText(Objects[i], SubElements[i]));
    if (Objects.size() > 0)
        ui->lw_references->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);
   // ui->lineDirection->setText(dir.isEmpty() ? tr("") : dir);
    ui->cb_reverse_direction->setChecked(reversed);

    ui->if_normal_stress->blockSignals(false);
    ui->lw_references->blockSignals(false);
    ui->b_add_reference->blockSignals(false);
//    ui->buttonDirection->blockSignals(false);
    ui->cb_reverse_direction->blockSignals(false);

    updateUI();
}

void TaskFemConstraintNormalStress::updateUI()
{
    if (ui->lw_references->model()->rowCount() == 0) {
        // Go into reference selection mode if no reference has been selected yet
        onButtonReference(true);
        return;
    }

//    std::string ref = ui->lw_references->item(0)->text().toStdString();
//    int pos = ref.find_last_of(":");
/*    if (ref.substr(pos+1, 6) == "Vertex")
        ui->labelNormalStress->setText(tr("Point load"));
    else if (ref.substr(pos+1, 4) == "Edge")
        ui->labelNormalStress->setText(tr("Line load"));
    else if (ref.substr(pos+1, 4) == "Face")*/
        ui->l_normal_stress->setText(tr("Area load"));
}

void TaskFemConstraintNormalStress::onSelectionChanged(const Gui::SelectionChanges& msg)
{
    if (msg.Type == Gui::SelectionChanges::AddSelection) {
        // Don't allow selection in other document
        if (strcmp(msg.pDocName, ConstraintView->getObject()->getDocument()->getName()) != 0)
            return;

        if (!msg.pSubName || msg.pSubName[0] == '\0')
            return;
        std::string subName(msg.pSubName);

        if (selectionMode == selnone)
            return;

        std::vector<std::string> references(1,subName);
        Fem::ConstraintNormalStress* pcConstraint = static_cast<Fem::ConstraintNormalStress*>(ConstraintView->getObject());
        App::DocumentObject* obj = ConstraintView->getObject()->getDocument()->getObject(msg.pObjectName);
        Part::Feature* feat = static_cast<Part::Feature*>(obj);
        TopoDS_Shape ref = feat->Shape.getShape().getSubShape(subName.c_str());

        if (selectionMode == selref) {
            std::vector<App::DocumentObject*> Objects = pcConstraint->References.getValues();
            std::vector<std::string> SubElements = pcConstraint->References.getSubValues();

            // Ensure we don't have mixed reference types
            if (SubElements.size() > 0) {
                if (subName.substr(0,4) != SubElements.front().substr(0,4)) {
                    QMessageBox::warning(this, tr("Selection error"), tr("Mixed shape types are not possible. Use a second constraint instead"));
                    return;
                }
            } else {
                if ((subName.substr(0,4) != "Face")/* && (subName.substr(0,4) != "Edge") && (subName.substr(0,6) != "Vertex")*/) {
                    QMessageBox::warning(this, tr("Selection error"), tr("Only faces can be picked"));
                    return;
                }
            }

            // Avoid duplicates
            std::size_t pos = 0;
            for (; pos < Objects.size(); pos++)
                if (obj == Objects[pos])
                    break;

            if (pos != Objects.size())
                if (subName == SubElements[pos])
                    return;

            // add the new reference
            Objects.push_back(obj);
            SubElements.push_back(subName);
            pcConstraint->References.setValues(Objects,SubElements);
            ui->lw_references->addItem(makeRefText(obj, subName));

            // Turn off reference selection mode
            onButtonReference(false);
        } else if (selectionMode == seldir) {
            if (subName.substr(0,4) == "Face") {
                BRepAdaptor_Surface surface(TopoDS::Face(ref));
                if (surface.GetType() != GeomAbs_Plane) {
                    QMessageBox::warning(this, tr("Selection error"), tr("Only planar faces can be picked"));
                    return;
                }
            } else if (subName.substr(0,4) == "Edge") {
                BRepAdaptor_Curve line(TopoDS::Edge(ref));
                if (line.GetType() != GeomAbs_Line) {
                    QMessageBox::warning(this, tr("Selection error"), tr("Only linear edges can be picked"));
                    return;
                }
            } else {
                QMessageBox::warning(this, tr("Selection error"), tr("Only faces and edges can be picked"));
                return;
            }
            pcConstraint->Direction.setValue(obj, references);
            //ui->lineDirection->setText(makeRefText(obj, subName));

            // Turn off direction selection mode
            //onButtonDirection(false);
        }

        Gui::Selection().clearSelection();
        updateUI();
    }
}

void TaskFemConstraintNormalStress::onNormalStressChanged(double f)
{
    Fem::ConstraintNormalStress* pcConstraint = static_cast<Fem::ConstraintNormalStress*>(ConstraintView->getObject());
    pcConstraint->NormalStress.setValue(f);
}

void TaskFemConstraintNormalStress::onReferenceDeleted() {
    int row = ui->lw_references->currentIndex().row();
    TaskFemConstraint::onReferenceDeleted(row);
    ui->lw_references->model()->removeRow(row);
    ui->lw_references->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);
}
/*
void TaskFemConstraintNormalStress::onButtonDirection(const bool pressed) {
    if (pressed) {
        selectionMode = seldir;
    } else {
        selectionMode = selnone;
    }
    ui->buttonDirection->setChecked(pressed);
    Gui::Selection().clearSelection();
}
*/
void TaskFemConstraintNormalStress::onCheckReverse(const bool pressed)
{
    Fem::ConstraintNormalStress* pcConstraint = static_cast<Fem::ConstraintNormalStress*>(ConstraintView->getObject());
    pcConstraint->Reversed.setValue(pressed);
}

double TaskFemConstraintNormalStress::getNormalStress(void) const
{
    Base::Quantity value =  ui->if_normal_stress->getQuantity();
//FIXME
    return 10.0; //value.getValueAs(Units::Pressure("MPa"));
}

const std::string TaskFemConstraintNormalStress::getReferences() const
{
    int rows = ui->lw_references->model()->rowCount();

    std::vector<std::string> items;
    for (int r = 0; r < rows; r++)
        items.push_back(ui->lw_references->item(r)->text().toStdString());
    return TaskFemConstraint::getReferences(items);
}
/*
const std::string TaskFemConstraintNormalStress::getDirectionName(void) const
{
    std::string dir = ui->lineDirection->text().toStdString();
    if (dir.empty())
        return "";

    int pos = dir.find_last_of(":");
    return dir.substr(0, pos).c_str();
}
*/
/*
const std::string TaskFemConstraintNormalStress::getDirectionObject(void) const
{
    std::string dir = ui->lineDirection->text().toStdString();
    if (dir.empty())
        return "";

    int pos = dir.find_last_of(":");
    return dir.substr(pos+1).c_str();
}
*/
bool TaskFemConstraintNormalStress::getReverse() const
{
    return ui->cb_reverse_direction->isChecked();
}

TaskFemConstraintNormalStress::~TaskFemConstraintNormalStress()
{
    delete ui;
}

void TaskFemConstraintNormalStress::changeEvent(QEvent *e)
{
    TaskBox::changeEvent(e);
    if (e->type() == QEvent::LanguageChange) {
        ui->if_normal_stress->blockSignals(true);
        ui->retranslateUi(proxy);
        ui->if_normal_stress->blockSignals(false);
    }
}

//**************************************************************************
//**************************************************************************
// TaskDialog
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TaskDlgFemConstraintNormalStress::TaskDlgFemConstraintNormalStress(ViewProviderFemConstraintNormalStress *ConstraintView)
{
    this->ConstraintView = ConstraintView;
    assert(ConstraintView);
    this->parameter = new TaskFemConstraintNormalStress(ConstraintView);;

    Content.push_back(parameter);
}

//==== calls from the TaskView ===============================================================

void TaskDlgFemConstraintNormalStress::open()
{
    // a transaction is already open at creation time of the panel
    if (!Gui::Command::hasPendingCommand()) {
        QString msg = QObject::tr("Constraint normal stress");
        Gui::Command::openCommand((const char*)msg.toUtf8());
    }
}

bool TaskDlgFemConstraintNormalStress::accept()
{
    std::string name = ConstraintView->getObject()->getNameInDocument();
    const TaskFemConstraintNormalStress* parameterNormalStress = static_cast<const TaskFemConstraintNormalStress*>(parameter);

    try {
        //Gui::Command::openCommand("FEM force constraint changed");
        Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.NormalStress = %f",name.c_str(), parameterNormalStress->getNormalStress());

       /* std::string dirname = parameterNormalStress->getDirectionName().data();
        std::string dirobj = parameterNormalStress->getDirectionObject().data();

        if (!dirname.empty()) {
            QString buf = QString::fromUtf8("(App.ActiveDocument.%1,[\"%2\"])");
            buf = buf.arg(QString::fromStdString(dirname));
            buf = buf.arg(QString::fromStdString(dirobj));
            Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.Direction = %s", name.c_str(), buf.toStdString().c_str());
        } else {*/
            Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.Direction = None", name.c_str());
   //     }

        Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.Reversed = %s", name.c_str(), parameterNormalStress->getReverse() ? "True" : "False");
    }
    catch (const Base::Exception& e) {
        QMessageBox::warning(parameter, tr("Input error"), QString::fromAscii(e.what()));
        return false;
    }

    return TaskDlgFemConstraint::accept();
}

bool TaskDlgFemConstraintNormalStress::reject()
{
    // roll back the changes
    Gui::Command::abortCommand();
    Gui::Command::doCommand(Gui::Command::Gui,"Gui.activeDocument().resetEdit()");
    Gui::Command::updateActive();

    return true;
}

#include "moc_TaskFemConstraintNormalStress.cpp"
