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
#endif

#include "DlgWorkbenchesImp.h"
#include "Application.h"
#include "BitmapFactory.h"
#include "Command.h"
#include "MainWindow.h"
#include "Widgets.h"
#include "Workbench.h"
#include "WorkbenchManager.h"
#include "QListWidgetCustom.h"

using namespace Gui::Dialog;

/* TRANSLATOR Gui::Dialog::DlgWorkbenches */

DlgWorkbenchesImp::DlgWorkbenchesImp(QWidget* parent)
    : CustomizeActionPage(parent)
{
    this->setupUi(this);
    lw_enabled_workbenches->setDragDropMode(QAbstractItemView::DragDrop);
    lw_enabled_workbenches->setSelectionMode(QAbstractItemView::SingleSelection);
    lw_enabled_workbenches->viewport()->setAcceptDrops(true);
    lw_enabled_workbenches->setDropIndicatorShown(true);
    lw_enabled_workbenches->setDragEnabled(true);
    lw_enabled_workbenches->setDefaultDropAction(Qt::MoveAction);

    const QString lw_disabled_name = QString::fromAscii("disabled workbenches");
    lw_disabled_workbenches->setAccessibleName(lw_disabled_name);
    lw_disabled_workbenches->setDragDropMode(QAbstractItemView::DragDrop);
    lw_disabled_workbenches->setSelectionMode(QAbstractItemView::SingleSelection);
    lw_disabled_workbenches->viewport()->setAcceptDrops(true);
    lw_disabled_workbenches->setDropIndicatorShown(true);
    lw_disabled_workbenches->setDragEnabled(true);
    lw_disabled_workbenches->setDefaultDropAction(Qt::MoveAction);
    lw_disabled_workbenches->setSortingEnabled(true);

    QStringList enabled_wbs_list = load_enabled_workbenches();
    QStringList workbenches = Application::Instance->workbenches();

    int i = 0;
    QPixmap px;
    QString mt;
    if (enabled_wbs_list.at(0) == QString::fromAscii("ALL")) {
        enabled_wbs_list.removeFirst();
        for (QStringList::Iterator it = workbenches.begin(); it != workbenches.end(); ++it) {
            enabled_wbs_list.append(*it);
        }
        enabled_wbs_list.sort();
    }

    for (QStringList::Iterator it = enabled_wbs_list.begin(); it != enabled_wbs_list.end(); ++it) {
        if (workbenches.contains(*it)){
            mt = Application::Instance->workbenchMenuText(*it);
            px = Application::Instance->workbenchIcon(*it);
            QListWidgetItem *wi = (new QListWidgetItem(QIcon(px), mt));
            wi->setData(Qt::UserRole, QVariant(*it));
            lw_enabled_workbenches->addItem(wi);
            i++;
        } else {
            qDebug("Ignoring unknown " + it->toLatin1() + " workbench found in user preferences.");
        }
    }
    i = 0;
    for (QStringList::Iterator it = workbenches.begin(); it != workbenches.end(); ++it) {
        if (!enabled_wbs_list.contains(*it)){
            mt = Application::Instance->workbenchMenuText(*it);
            px = Application::Instance->workbenchIcon(*it);
            QListWidgetItem *wi = (new QListWidgetItem(QIcon(px), mt));
            wi->setData(Qt::UserRole, QVariant(*it));
            lw_disabled_workbenches->addItem(wi);
            i++;
        }
    }
    lw_enabled_workbenches->setCurrentItem(0);
    lw_disabled_workbenches->setCurrentItem(0);
}

/** Destroys the object and frees any allocated resources */
DlgWorkbenchesImp::~DlgWorkbenchesImp()
{
}

void DlgWorkbenchesImp::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::LanguageChange) {
        retranslateUi(this);
    }
    else {
        QWidget::changeEvent(e);
    }
}

void DlgWorkbenchesImp::hideEvent(QHideEvent * event)
{
    save_enabled_workbenches();
}

void DlgWorkbenchesImp::onAddMacroAction(const QByteArray& macro)
{
}

void DlgWorkbenchesImp::onRemoveMacroAction(const QByteArray& macro)
{
}

void DlgWorkbenchesImp::onModifyMacroAction(const QByteArray& macro)
{
}

void DlgWorkbenchesImp::add_to_enabled_workbenches(QListWidgetItem *wi)
{
    if (wi) {
        QListWidgetItem* item = wi->clone();
        lw_enabled_workbenches->addItem(item);
        lw_enabled_workbenches->setCurrentItem(item);
        delete wi;
    }
}

void DlgWorkbenchesImp::on_add_to_enabled_workbenches_btn_clicked()
{
    if (lw_disabled_workbenches->currentItem()) {
        QListWidgetItem* item = lw_disabled_workbenches->currentItem()->clone();
        lw_enabled_workbenches->addItem(item);
        lw_enabled_workbenches->setCurrentItem(item);
        delete lw_disabled_workbenches->currentItem();
    }
}

void DlgWorkbenchesImp::on_remove_from_enabled_workbenches_btn_clicked()
{
    if (lw_enabled_workbenches->currentItem()) {
        QListWidgetItem* item = lw_enabled_workbenches->currentItem()->clone();
        lw_disabled_workbenches->addItem(item);
        lw_disabled_workbenches->setCurrentItem(item);
        delete lw_enabled_workbenches->currentItem();
    }
}

void DlgWorkbenchesImp::on_move_workbench_up_btn_clicked()
{
    if (lw_enabled_workbenches->currentItem()) {
        int index = lw_enabled_workbenches->currentRow();
        QListWidgetItem *item = lw_enabled_workbenches->takeItem(index);
        lw_enabled_workbenches->insertItem(index - 1, item);
        lw_enabled_workbenches->setCurrentRow(index - 1);
    }
}

void DlgWorkbenchesImp::on_move_workbench_down_btn_clicked()
{
    if (lw_enabled_workbenches->currentItem()) {
        int index = lw_enabled_workbenches->currentRow();
        QListWidgetItem *item = lw_enabled_workbenches->takeItem(index);
        lw_enabled_workbenches->insertItem(index + 1, item);
        lw_enabled_workbenches->setCurrentRow(index + 1);
    }
}

void DlgWorkbenchesImp::on_sort_enabled_workbenches_btn_clicked()
{
    lw_enabled_workbenches->sortItems();
}

void DlgWorkbenchesImp::on_add_all_to_enabled_workbenches_btn_clicked()
{
    while (lw_disabled_workbenches->count() > 0) {
        QListWidgetItem* item = lw_disabled_workbenches->item(0);
        add_to_enabled_workbenches(item);
    }
}

QStringList DlgWorkbenchesImp::load_enabled_workbenches()
{
    QString enabled_wbs;
    QStringList enabled_wbs_list;
    ParameterGrp::handle hGrp;

    hGrp = App::GetApplication().GetParameterGroupByPath("User parameter:BaseApp/Workbenches");
    enabled_wbs = QString::fromStdString(hGrp->GetASCII("Enabled", "ALL").c_str());
    enabled_wbs_list = enabled_wbs.split(QLatin1String(","), QString::SkipEmptyParts);
    return enabled_wbs_list;
}

void DlgWorkbenchesImp::save_enabled_workbenches()
{
    QString enabled_wbs;
    ParameterGrp::handle hGrp;

    hGrp = App::GetApplication().GetParameterGroupByPath("User parameter:BaseApp/Workbenches");
    hGrp->Clear();

    if (lw_enabled_workbenches->count() == 0) {
        enabled_wbs.append(QString::fromAscii("NoneWorkbench"));
    } else {
        for (int i = 0; i < lw_enabled_workbenches->count(); i++) {
            QVariant item_data = lw_enabled_workbenches->item(i)->data(Qt::UserRole);
            QString name = item_data.toString();
            enabled_wbs.append(name + QString::fromAscii(","));
        }
    }
    hGrp->SetASCII("Enabled", enabled_wbs.toAscii());
}

#include "moc_DlgWorkbenchesImp.cpp"
