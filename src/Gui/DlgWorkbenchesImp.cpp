/***************************************************************************
 *   Copyright (c) 2015 FreeCAD Developers                                 *
 *   Author: Przemo Firszt <przemo@firszt.eu>                              *
 *   Based on DlgToolbars.cpp file                                         *
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
# include <QInputDialog>
# include <QHeaderView>
# include <QMessageBox>
# include <QToolBar>
#endif

#include "DlgWorkbenchesImp.h"
#include "Application.h"
#include "BitmapFactory.h"
#include "Command.h"
#include "ToolBarManager.h"
#include "MainWindow.h"
#include "Widgets.h"
#include "Workbench.h"
#include "WorkbenchManager.h"
#include "QListWidgetWithDragging.h"

using namespace Gui::Dialog;

/* TRANSLATOR Gui::Dialog::DlgWorkbenches */

DlgWorkbenches::DlgWorkbenches(DlgWorkbenches::Type t, QWidget* parent)
    : CustomizeActionPage(parent), type(t)
{
    this->setupUi(this);

    QStringList enabled_wbs = load_enabled_workbenches();

    add_to_enabled_workbenches_btn->setIcon(BitmapFactory().pixmap(":/icons/button_right.svg"));
    remove_from_enabled_workbenches_btn->setIcon(BitmapFactory().pixmap(":/icons/button_left.svg"));
    move_workbench_down_btn->setIcon(BitmapFactory().pixmap(":/icons/button_down.svg"));
    move_workbench_up_btn->setIcon(BitmapFactory().pixmap(":/icons/button_up.svg"));

    QStringList workbenches = Application::Instance->workbenches();
    int i = 0;
    QPixmap px;
    QString mt;
    for (QStringList::Iterator it = enabled_wbs.begin(); it != enabled_wbs.end(); ++it) {
        if (workbenches.contains(*it)){
            mt = Application::Instance->workbenchMenuText(*it);
            px = Application::Instance->workbenchIcon(*it);
            lw_enabled_workbenches->insertItem(i, new QListWidgetItem(QIcon(px), mt));
            lw_enabled_workbenches->item(i)->setData(Qt::UserRole, QVariant(*it));
            i++;
        } else {
            qDebug("Ignoring unknown " + it->toLatin1() + " workbench found in user preferences.");
        }
    }
    i = 0;
    for (QStringList::Iterator it = workbenches.begin(); it != workbenches.end(); ++it) {
        if (!enabled_wbs.contains(*it)){
            mt = Application::Instance->workbenchMenuText(*it);
            px = Application::Instance->workbenchIcon(*it);
            lw_disabled_workbenches->addItem(new QListWidgetItem(QIcon(px), mt));
            lw_disabled_workbenches->item(i)->setData(Qt::UserRole, QVariant(*it));
            i++;
        }
    }
    lw_enabled_workbenches->setDragDropMode(QAbstractItemView::DragDrop);
    lw_enabled_workbenches->setSelectionMode(QAbstractItemView::SingleSelection);
    lw_enabled_workbenches->viewport()->setAcceptDrops(true);
    lw_enabled_workbenches->setDropIndicatorShown(true);
    lw_enabled_workbenches->setDragEnabled(true);
    lw_enabled_workbenches->setDefaultDropAction(Qt::MoveAction);

    lw_disabled_workbenches->setDragDropMode(QAbstractItemView::DragDrop);
    lw_disabled_workbenches->setSelectionMode(QAbstractItemView::SingleSelection);
    lw_disabled_workbenches->viewport()->setAcceptDrops(true);
    lw_disabled_workbenches->setDropIndicatorShown(true);
    lw_disabled_workbenches->setDragEnabled(true);
    lw_disabled_workbenches->setDefaultDropAction(Qt::MoveAction);
    lw_disabled_workbenches->setSortingEnabled(true);
}

/** Destroys the object and frees any allocated resources */
DlgWorkbenches::~DlgWorkbenches()
{
}

void DlgWorkbenches::addCustomToolbar(const QString&)
{
}

void DlgWorkbenches::removeCustomToolbar(const QString&)
{
}

void DlgWorkbenches::renameCustomToolbar(const QString&, const QString&)
{
}

void DlgWorkbenches::addCustomCommand(const QString&, const QByteArray&)
{
}

void DlgWorkbenches::removeCustomCommand(const QString&, const QByteArray&)
{
}

void DlgWorkbenches::moveUpCustomCommand(const QString&, const QByteArray&)
{
}

void DlgWorkbenches::moveDownCustomCommand(const QString&, const QByteArray&)
{
}

void DlgWorkbenches::hideEvent(QHideEvent * event)
{
    save_enabled_workbenches();
}

QStringList DlgWorkbenches::load_enabled_workbenches()
{
    QString enabled_wbs;
    QStringList enabled_wbs_list;
    ParameterGrp::handle hGrp;

    hGrp = App::GetApplication().GetParameterGroupByPath("User parameter:BaseApp/Workbenches");
    enabled_wbs = QString::fromStdString(hGrp->GetASCII("Enabled", "").c_str());
    enabled_wbs_list = enabled_wbs.split(QLatin1String(","), QString::SkipEmptyParts);
    return enabled_wbs_list;
}

void DlgWorkbenches::save_enabled_workbenches()
{
    QString enabled_wbs;
    ParameterGrp::handle hGrp;

    hGrp = App::GetApplication().GetParameterGroupByPath("User parameter:BaseApp/Workbenches");
    hGrp->Clear();

    for (int i = 0; i < lw_enabled_workbenches->count(); i++) {
        QListWidgetItem *item = lw_enabled_workbenches->item(i);
        QVariant item_data = lw_enabled_workbenches->item(i)->data(Qt::UserRole);
        QString name = item_data.toString();
	qDebug("Adding " + name.toUtf8() + " to Preferences");
	enabled_wbs.append(name + QString::fromAscii(","));
    }
    hGrp->SetASCII("Enabled", enabled_wbs.toAscii());
    qDebug("Complete list save to Preferences:");
}

void DlgWorkbenches::on_add_to_enabled_workbenches_btn_clicked()
{
    if (lw_disabled_workbenches->currentItem()) {
        QListWidgetItem* item = lw_disabled_workbenches->currentItem()->clone();
	lw_enabled_workbenches->addItem(item);
	lw_enabled_workbenches->setCurrentItem(item);
	delete lw_disabled_workbenches->currentItem();
    }
}

void DlgWorkbenches::on_remove_from_enabled_workbenches_btn_clicked()
{
    if (lw_enabled_workbenches->currentItem()) {
        QListWidgetItem* item = lw_enabled_workbenches->currentItem()->clone();
	lw_disabled_workbenches->addItem(item);
	lw_disabled_workbenches->setCurrentItem(item);
	delete lw_enabled_workbenches->currentItem();
    }
}

void DlgWorkbenches::on_move_workbench_up_btn_clicked()
{
    if (lw_enabled_workbenches->currentItem()) {
        int index = lw_enabled_workbenches->currentRow();
        QListWidgetItem *item = lw_enabled_workbenches->takeItem(index);
        lw_enabled_workbenches->insertItem(index - 1, item);
        lw_enabled_workbenches->setCurrentRow(index - 1);
    }
}

void DlgWorkbenches::on_move_workbench_down_btn_clicked()
{
    if (lw_enabled_workbenches->currentItem()) {
        int index = lw_enabled_workbenches->currentRow();
        QListWidgetItem *item = lw_enabled_workbenches->takeItem(index);
        lw_enabled_workbenches->insertItem(index + 1, item);
        lw_enabled_workbenches->setCurrentRow(index + 1);
    }
}

void DlgWorkbenches::onAddMacroAction(const QByteArray& macro)
{
}

void DlgWorkbenches::onRemoveMacroAction(const QByteArray& macro)
{
}

void DlgWorkbenches::onModifyMacroAction(const QByteArray& macro)
{
}

void DlgWorkbenches::changeEvent(QEvent *e)
{
}

// -------------------------------------------------------------

/* TRANSLATOR Gui::Dialog::DlgWorkbenchesImp */

DlgWorkbenchesImp::DlgWorkbenchesImp( QWidget* parent )
    : DlgWorkbenches(DlgWorkbenches::Toolbar, parent)
{
}

/** Destroys the object and frees any allocated resources */
DlgWorkbenchesImp::~DlgWorkbenchesImp()
{
}

void DlgWorkbenchesImp::addCustomToolbar(const QString& name)
{
}

void DlgWorkbenchesImp::removeCustomToolbar(const QString& name)
{
}

void DlgWorkbenchesImp::renameCustomToolbar(const QString& old_name, const QString& new_name)
{

void DlgWorkbenchesImp::addCustomCommand(const QString& name, const QByteArray& cmd)
{
}

void DlgWorkbenchesImp::removeCustomCommand(const QString& name, const QByteArray& userdata)
{
}

void DlgWorkbenchesImp::moveUpCustomCommand(const QString& name, const QByteArray& userdata)
{
}

void DlgWorkbenchesImp::moveDownCustomCommand(const QString& name, const QByteArray& userdata)
{
}

void DlgWorkbenchesImp::changeEvent(QEvent *e)
{
}

#include "moc_DlgWorkbenchesImp.cpp"
