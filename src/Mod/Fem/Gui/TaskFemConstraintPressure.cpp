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

#include "ui_TaskFemConstraintPressure.h"
#include "TaskFemConstraintPressure.h"
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
#include <Mod/Fem/App/FemConstraintPressure.h>
#include <Mod/Part/App/PartFeature.h>

#include <Base/Console.h>

using namespace FemGui;
using namespace Gui;

/* TRANSLATOR FemGui::TaskFemConstraintPressure */

TaskFemConstraintPressure::TaskFemConstraintPressure(ViewProviderFemConstraintPressure *ConstraintView,QWidget *parent)
    : TaskFemConstraint(ConstraintView, parent, "Fem_ConstraintPressure")
{
    // we need a separate container widget to add all controls to
    proxy = new QWidget(this);
    ui = new Ui_TaskFemConstraintPressure();
    ui->setupUi(proxy);
    QMetaObject::connectSlotsByName(this);

    // Create a context menu for the listview of the references
    QAction* action = new QAction(tr("Delete"), ui->lw_references);
    action->connect(action, SIGNAL(triggered()),
                    this, SLOT(onReferenceDeleted()));
    ui->lw_references->addAction(action);
    ui->lw_references->setContextMenuPolicy(Qt::ActionsContextMenu);

    connect(ui->if_pressure, SIGNAL(valueChanged(Base::Quantity)),
            this, SLOT(onPressureChanged(Base::Quantity)));
    connect(ui->b_add_reference, SIGNAL(pressed()),
            this, SLOT(onButtonReference()));
//    connect(ui->buttonDirection, SIGNAL(pressed()),
//            this, SLOT(onButtonDirection()));
    connect(ui->cb_reverse_direction, SIGNAL(toggled(bool)),
            this, SLOT(onCheckReverse(bool)));

    this->groupLayout()->addWidget(proxy);

    // Temporarily prevent unnecessary feature recomputes
    ui->if_pressure->blockSignals(true);
    ui->lw_references->blockSignals(true);
    ui->b_add_reference->blockSignals(true);
   // ui->buttonDirection->blockSignals(true);
    ui->cb_reverse_direction->blockSignals(true);

    // Get the feature data
    Fem::ConstraintPressure* pcConstraint = static_cast<Fem::ConstraintPressure*>(ConstraintView->getObject());
    double f = pcConstraint->Pressure.getValue();
    std::vector<App::DocumentObject*> Objects = pcConstraint->References.getValues();
    std::vector<std::string> SubElements = pcConstraint->References.getSubValues();
    std::vector<std::string> dirStrings = pcConstraint->Direction.getSubValues();
    QString dir;
    if (!dirStrings.empty())
        dir = makeRefText(pcConstraint->Direction.getValue(), dirStrings.front());
    bool reversed = pcConstraint->Reversed.getValue();

    // Fill data into dialog elements
    ui->if_pressure->setMinimum(0);
    ui->if_pressure->setMaximum(FLOAT_MAX);
//FIXME - why it's defaulting to kPa??
    qDebug("Setting pressure to f = %f in [kPa]", f);
    Base::Quantity p = Base::Quantity(1000 * f, Base::Unit::Stress);
    double val = p.getValueAs(Base::Quantity::MegaPascal);
    qDebug("Setting pressure to val = %f in MPa", val);
    ui->if_pressure->setValue(p);
    ui->lw_references->clear();
    for (std::size_t i = 0; i < Objects.size(); i++)
        ui->lw_references->addItem(makeRefText(Objects[i], SubElements[i]));
    if (Objects.size() > 0)
        ui->lw_references->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);
   // ui->lineDirection->setText(dir.isEmpty() ? tr("") : dir);
    ui->cb_reverse_direction->setChecked(reversed);

    ui->if_pressure->blockSignals(false);
    ui->lw_references->blockSignals(false);
    ui->b_add_reference->blockSignals(false);
//    ui->buttonDirection->blockSignals(false);
    ui->cb_reverse_direction->blockSignals(false);

    updateUI();
}

void TaskFemConstraintPressure::updateUI()
{
    if (ui->lw_references->model()->rowCount() == 0) {
        // Go into reference selection mode if no reference has been selected yet
        onButtonReference(true);
        return;
    }

//    std::string ref = ui->lw_references->item(0)->text().toStdString();
//    int pos = ref.find_last_of(":");
/*    if (ref.substr(pos+1, 6) == "Vertex")
        ui->labelPressure->setText(tr("Point load"));
    else if (ref.substr(pos+1, 4) == "Edge")
        ui->labelPressure->setText(tr("Line load"));
    else if (ref.substr(pos+1, 4) == "Face")*/
       // ui->l_pressure->setText(tr("Area load"));
}

void TaskFemConstraintPressure::onSelectionChanged(const Gui::SelectionChanges& msg)
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
        Fem::ConstraintPressure* pcConstraint = static_cast<Fem::ConstraintPressure*>(ConstraintView->getObject());
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
/*Direction selection */
        } /*else if (selectionMode == seldir) {
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
                QMessageBox::warning(this, tr("Selection error"), tr("Only faces can be picked"));
                return;
            }
            pcConstraint->Direction.setValue(obj, references);
            //ui->lineDirection->setText(makeRefText(obj, subName));

            // Turn off direction selection mode
            //onButtonDirection(false);
        }*/

        Gui::Selection().clearSelection();
        updateUI();
    }
}

void TaskFemConstraintPressure::onPressureChanged(const Base::Quantity& f)
{
    Fem::ConstraintPressure* pcConstraint = static_cast<Fem::ConstraintPressure*>(ConstraintView->getObject());
	qDebug("onPressurechanged" );
    double val = f.getValueAs(Base::Quantity::MegaPascal);
    QString unit_string = f.getUnit().getString();
    qDebug("Pressure");
    qDebug("%f", val);
    qDebug("Unit");
    qDebug(unit_string.toUtf8());
	qDebug("end onPressurechanged" );
    pcConstraint->Pressure.setValue(val);
}

void TaskFemConstraintPressure::onReferenceDeleted() {
    int row = ui->lw_references->currentIndex().row();
    TaskFemConstraint::onReferenceDeleted(row);
    ui->lw_references->model()->removeRow(row);
    ui->lw_references->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);
}
/*
void TaskFemConstraintPressure::onButtonDirection(const bool pressed) {
    if (pressed) {
        selectionMode = seldir;
    } else {
        selectionMode = selnone;
    }
    ui->buttonDirection->setChecked(pressed);
    Gui::Selection().clearSelection();
}
*/
void TaskFemConstraintPressure::onCheckReverse(const bool pressed)
{
    Fem::ConstraintPressure* pcConstraint = static_cast<Fem::ConstraintPressure*>(ConstraintView->getObject());
    pcConstraint->Reversed.setValue(pressed);
}

double TaskFemConstraintPressure::getPressure(void) const
{
    Base::Quantity value =  ui->if_pressure->getQuantity();
    double val = value.getValueAs(Base::Quantity::MegaPascal);
    QString unit_string = value.getUserString();
    qDebug("getPressure");
    qDebug("Pressure");
    qDebug("%f", val);
    qDebug("Unit");
    qDebug(unit_string.toUtf8());
    qDebug("end of getPressure");
    return val;
}

const std::string TaskFemConstraintPressure::getReferences() const
{
    int rows = ui->lw_references->model()->rowCount();

    std::vector<std::string> items;
    for (int r = 0; r < rows; r++)
        items.push_back(ui->lw_references->item(r)->text().toStdString());
    return TaskFemConstraint::getReferences(items);
}
/*
const std::string TaskFemConstraintPressure::getDirectionName(void) const
{
    std::string dir = ui->lineDirection->text().toStdString();
    if (dir.empty())
        return "";

    int pos = dir.find_last_of(":");
    return dir.substr(0, pos).c_str();
}
*/
/*
const std::string TaskFemConstraintPressure::getDirectionObject(void) const
{
    std::string dir = ui->lineDirection->text().toStdString();
    if (dir.empty())
        return "";

    int pos = dir.find_last_of(":");
    return dir.substr(pos+1).c_str();
}
*/
bool TaskFemConstraintPressure::getReverse() const
{
    return ui->cb_reverse_direction->isChecked();
}

TaskFemConstraintPressure::~TaskFemConstraintPressure()
{
    delete ui;
}

void TaskFemConstraintPressure::changeEvent(QEvent *e)
{
    TaskBox::changeEvent(e);
    if (e->type() == QEvent::LanguageChange) {
        ui->if_pressure->blockSignals(true);
        ui->retranslateUi(proxy);
        ui->if_pressure->blockSignals(false);
    }
}

//**************************************************************************
//**************************************************************************
// TaskDialog
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TaskDlgFemConstraintPressure::TaskDlgFemConstraintPressure(ViewProviderFemConstraintPressure *ConstraintView)
{
    this->ConstraintView = ConstraintView;
    assert(ConstraintView);
    this->parameter = new TaskFemConstraintPressure(ConstraintView);;

    Content.push_back(parameter);
}

//==== calls from the TaskView ===============================================================

void TaskDlgFemConstraintPressure::open()
{
    // a transaction is already open at creation time of the panel
    if (!Gui::Command::hasPendingCommand()) {
        QString msg = QObject::tr("Constraint normal stress");
        Gui::Command::openCommand((const char*)msg.toUtf8());
    }
}

bool TaskDlgFemConstraintPressure::accept()
{
    std::string name = ConstraintView->getObject()->getNameInDocument();
    const TaskFemConstraintPressure* parameterPressure = static_cast<const TaskFemConstraintPressure*>(parameter);

    try {
        //Gui::Command::openCommand("FEM force constraint changed");
        Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.Pressure = %f",name.c_str(), parameterPressure->getPressure());

       /* std::string dirname = parameterPressure->getDirectionName().data();
        std::string dirobj = parameterPressure->getDirectionObject().data();

        if (!dirname.empty()) {
            QString buf = QString::fromUtf8("(App.ActiveDocument.%1,[\"%2\"])");
            buf = buf.arg(QString::fromStdString(dirname));
            buf = buf.arg(QString::fromStdString(dirobj));
            Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.Direction = %s", name.c_str(), buf.toStdString().c_str());
        } else {*/
            Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.Direction = None", name.c_str());
   //     }

        Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.Reversed = %s", name.c_str(), parameterPressure->getReverse() ? "True" : "False");
    }
    catch (const Base::Exception& e) {
        QMessageBox::warning(parameter, tr("Input error"), QString::fromAscii(e.what()));
        return false;
    }

    return TaskDlgFemConstraint::accept();
}

bool TaskDlgFemConstraintPressure::reject()
{
    // roll back the changes
    Gui::Command::abortCommand();
    Gui::Command::doCommand(Gui::Command::Gui,"Gui.activeDocument().resetEdit()");
    Gui::Command::updateActive();

    return true;
}

#include "moc_TaskFemConstraintPressure.cpp"
