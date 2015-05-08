/***************************************************************************
 *   Copyright (c) 2015 FreeCAD Developers                                 *
 *   Author: Przemo Firszt <przemo@firszt.eu>                              *
 *   Based on DlgToolbars.h file                                           *
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

using namespace Gui::Dialog;

namespace Gui { namespace Dialog {
typedef std::vector< std::pair<QLatin1String, QString> > GroupMap;

struct GroupMap_find {
    const QLatin1String& item;
    GroupMap_find(const QLatin1String& item) : item(item) {}
    bool operator () (const std::pair<QLatin1String, QString>& elem) const
    {
        return elem.first == item;
    }
};
}
}

/* TRANSLATOR Gui::Dialog::DlgWorkbenches */

DlgWorkbenches::DlgWorkbenches(DlgWorkbenches::Type t, QWidget* parent)
    : CustomizeActionPage(parent), type(t)
{
    this->setupUi(this);
    add_to_active_workbenches_btn->setIcon(BitmapFactory().pixmap(":/icons/button_right.svg"));
    remove_from_active_workbenches_btn->setIcon(BitmapFactory().pixmap(":/icons/button_left.svg"));
    move_workbench_down_btn->setIcon(BitmapFactory().pixmap(":/icons/button_down.svg"));
    move_workbench_up_btn->setIcon(BitmapFactory().pixmap(":/icons/button_up.svg"));
/*
    CommandManager & cCmdMgr = Application::Instance->commandManager();
    std::map<std::string,Command*> sCommands = cCmdMgr.getCommands();

    GroupMap groupMap;
    groupMap.push_back(std::make_pair(QLatin1String("File"), QString()));
    groupMap.push_back(std::make_pair(QLatin1String("Edit"), QString()));
    groupMap.push_back(std::make_pair(QLatin1String("View"), QString()));
    groupMap.push_back(std::make_pair(QLatin1String("Standard-View"), QString()));
    groupMap.push_back(std::make_pair(QLatin1String("Tools"), QString()));
    groupMap.push_back(std::make_pair(QLatin1String("Window"), QString()));
    groupMap.push_back(std::make_pair(QLatin1String("Help"), QString()));
    groupMap.push_back(std::make_pair(QLatin1String("Macros"), qApp->translate("Gui::MacroCommand", "Macros")));

    for (std::map<std::string,Command*>::iterator it = sCommands.begin(); it != sCommands.end(); ++it) {
        QLatin1String group(it->second->getGroupName());
        QString text = qApp->translate(it->second->className(), it->second->getGroupName());
        GroupMap::iterator jt;
        jt = std::find_if(groupMap.begin(), groupMap.end(), GroupMap_find(group));
        if (jt != groupMap.end())
            jt->second = text;
        else
            groupMap.push_back(std::make_pair(group, text));
    }

    int index = 0;
    for (GroupMap::iterator it = groupMap.begin(); it != groupMap.end(); ++it, ++index) {
        categoryBox->addItem(it->second);
        categoryBox->setItemData(index, QVariant(it->first), Qt::UserRole);
    }
*/
    // fills the combo box with all available workbenches
    int index;
    QStringList workbenches = Application::Instance->workbenches();
    workbenches.sort();
    index = 0;
    //lw_all_workbenches->addItem(QApplication::windowIcon(), tr("Global"));
    //lw_all_workbenches->setItemData(0, QVariant(QString::fromAscii("Global")), Qt::UserRole);
    for (QStringList::Iterator it = workbenches.begin(); it != workbenches.end(); ++it) {
        QPixmap px = Application::Instance->workbenchIcon(*it);
        QString mt = Application::Instance->workbenchMenuText(*it);
        //if (mt != QLatin1String("<none>")) {
         //   if (px.isNull())
       //         lw_all_workbenches->addItem(mt);
	    lw_all_workbenches->addItem(new QListWidgetItem(QIcon(px), mt));
         //   else
         //       lw_all_workbenches->addItem(px, mt);
            lw_all_workbenches->item(index)->setData(Qt::UserRole, QVariant(*it));
            index++;
        //}
    }
/*
    QStringList labels; 
    labels << tr("Icon") << tr("Command");
    commandTreeWidget->setHeaderLabels(labels);
    commandTreeWidget->header()->hide();
    commandTreeWidget->setIconSize(QSize(32, 32));
    commandTreeWidget->header()->setResizeMode(0, QHeaderView::ResizeToContents);

    labels.clear(); labels << tr("Command");
    toolbarTreeWidget->setHeaderLabels(labels);
    toolbarTreeWidget->header()->hide();

    on_categoryBox_activated(categoryBox->currentIndex());
    Workbench* w = WorkbenchManager::instance()->active();
    if (w) {
        QString name = QString::fromAscii(w->name().c_str());
        int index = cbx_active_workbenches->findData(name);
        cbx_active_workbenches->setCurrentIndex(index);
    }
    on_cbx_active_workbenches_activated(cbx_active_workbenches->currentIndex());
*/
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
/*    QVariant data = cbx_active_workbenches->itemData(cbx_active_workbenches->currentIndex(), Qt::UserRole);
    QString workbench = data.toString();
    exportWorkbenches(workbench.toAscii());

    CustomizeActionPage::hideEvent(event);
*/
}

void DlgWorkbenches::importWorkbenches(const QByteArray& name)
{
 /*   ParameterGrp::handle hGrp = App::GetApplication().GetUserParameter().GetGroup("BaseApp")->GetGroup("Workbench");
    const char* subgroup = (type == Toolbar ? "Toolbar" : "Toolboxbar");
    if (!hGrp->HasGroup(name.constData()))
        return;
    hGrp = hGrp->GetGroup(name.constData());
    if (!hGrp->HasGroup(subgroup))
        return;
    hGrp = hGrp->GetGroup(subgroup);
    std::string separator = "Separator";

    std::vector<Base::Reference<ParameterGrp> > hGrps = hGrp->GetGroups();
    CommandManager& rMgr = Application::Instance->commandManager();
    for (std::vector<Base::Reference<ParameterGrp> >::iterator it = hGrps.begin(); it != hGrps.end(); ++it) {
        // create a toplevel item
        QTreeWidgetItem* toplevel = new QTreeWidgetItem(toolbarTreeWidget);
        bool active = (*it)->GetBool("Active", true);
        toplevel->setCheckState(0, (active ? Qt::Checked : Qt::Unchecked));

        // get the elements of the subgroups
        std::vector<std::pair<std::string,std::string> > items = (*it)->GetASCIIMap();
        for (std::vector<std::pair<std::string,std::string> >::iterator it2 = items.begin(); it2 != items.end(); ++it2) {
            // since we have stored the separators to the user parameters as (key, pair) we had to
            // make sure to use a unique key because otherwise we cannot store more than
            // one.
            if (it2->first.substr(0, separator.size()) == separator) {
                QTreeWidgetItem* item = new QTreeWidgetItem(toplevel);
                item->setText(0, tr("<Separator>"));
                item->setData(0, Qt::UserRole, QByteArray("Separator"));
                item->setSizeHint(0, QSize(32, 32));
            }
            else if (it2->first == "Name") {
                QString toolbarName = QString::fromUtf8(it2->second.c_str());
                toplevel->setText(0, toolbarName);
            }
            else {
                Command* pCmd = rMgr.getCommandByName(it2->first.c_str());
                if (pCmd) {
                    // command name
                    QTreeWidgetItem* item = new QTreeWidgetItem(toplevel);
                    item->setText(0, qApp->translate(pCmd->className(), pCmd->getMenuText()));
                    item->setData(0, Qt::UserRole, QByteArray(it2->first.c_str()));
                    if (pCmd->getPixmap())
                        item->setIcon(0, BitmapFactory().pixmap(pCmd->getPixmap()));
                    item->setSizeHint(0, QSize(32, 32));
                }
            }
        }
    }
*/
}

void DlgWorkbenches::exportWorkbenches(const QByteArray& workbench)
{
/*
    ParameterGrp::handle hGrp = App::GetApplication().GetUserParameter().GetGroup("BaseApp")->GetGroup("Workbench");
    const char* subgroup = (type == Toolbar ? "Toolbar" : "Toolboxbar");
    hGrp = hGrp->GetGroup(workbench.constData())->GetGroup(subgroup);
    hGrp->Clear();

    CommandManager& rMgr = Application::Instance->commandManager();
    for (int i=0; i<toolbarTreeWidget->topLevelItemCount(); i++) {
        QTreeWidgetItem* toplevel = toolbarTreeWidget->topLevelItem(i);
        QString groupName = QString::fromAscii("Custom_%1").arg(i+1);
        QByteArray toolbarName = toplevel->text(0).toUtf8();
        ParameterGrp::handle hToolGrp = hGrp->GetGroup(groupName.toAscii());
        hToolGrp->SetASCII("Name", toolbarName.constData());
        hToolGrp->SetBool("Active", toplevel->checkState(0) == Qt::Checked);

        // since we store the separators to the user parameters as (key, pair) we must
        // make sure to use a unique key because otherwise we cannot store more than
        // one.
        int suffixSeparator = 1;
        for (int j=0; j<toplevel->childCount(); j++) {
            QTreeWidgetItem* child = toplevel->child(j);
            QByteArray commandName = child->data(0, Qt::UserRole).toByteArray();
            if (commandName == "Separator") {
                QByteArray key = commandName + QByteArray::number(suffixSeparator);
                suffixSeparator++;
                hToolGrp->SetASCII(key, commandName);
            }
            else {
                Command* pCmd = rMgr.getCommandByName(commandName);
                if (pCmd) {
                    hToolGrp->SetASCII(pCmd->getName(), pCmd->getAppModuleName());
                }
            }
        }
    }
*/
}

/** Adds a new action */
void DlgWorkbenches::on_add_to_active_workbenches_btn_clicked()
{
qDebug("on_add_to_active_workbenches_btn_clicked");

    QListWidgetItem* item = lw_all_workbenches->currentItem();
    if (item) {
	qDebug("gonna add " + item->text().toLatin1() + " to active wbs");
    }
 /*       QTreeWidgetItem* current = toolbarTreeWidget->currentItem();
        if (!current)
            current = toolbarTreeWidget->topLevelItem(0);
        else if (current->parent())
            current = current->parent();
        if (current && !current->parent()) {
            QTreeWidgetItem* copy = new QTreeWidgetItem(current);
            copy->setText(0, item->text(1));
            copy->setIcon(0, item->icon(0));
            QByteArray data = item->data(1, Qt::UserRole).toByteArray();
            copy->setData(0, Qt::UserRole, data);
            copy->setSizeHint(0, QSize(32, 32));
            addCustomCommand(current->text(0), data);
        }
    }

    QVariant data = cbx_active_workbenches->itemData(cbx_active_workbenches->currentIndex(), Qt::UserRole);
    QString workbench = data.toString();
    exportWorkbenches(workbench.toAscii());
*/
}

/** Removes an action */
void DlgWorkbenches::on_remove_from_active_workbenches_btn_clicked()
{
qDebug("on_remove_from_active_workbenches_btn_clicked");
/*
    QTreeWidgetItem* item = toolbarTreeWidget->currentItem();
    if (item && item->parent() && toolbarTreeWidget->isItemSelected(item)) {
        QTreeWidgetItem* parent = item->parent();
        int index = parent->indexOfChild(item);
        parent->takeChild(index);

        // In case a separator should be moved we have to count the separators
        // which come before this one.
        // This is needed so that we can distinguish in removeCustomCommand
        // which separator it is.
        QByteArray data = item->data(0, Qt::UserRole).toByteArray();
        if (data == "Separator") {
            int countSep = 1;
            for (int i=0; i<index-1; i++) {
                QByteArray d = parent->child(i)->data(0, Qt::UserRole).toByteArray();
                if (d == "Separator") {
                    countSep++;
                }
            }

            data += QByteArray::number(countSep);
        }
        removeCustomCommand(parent->text(0), data);
        delete item;
    }

    QVariant data = cbx_active_workbenches->itemData(cbx_active_workbenches->currentIndex(), Qt::UserRole);
    QString workbench = data.toString();
    exportWorkbenches(workbench.toAscii());
*/
}

/** Moves up an action */
void DlgWorkbenches::on_move_workbench_up_btn_clicked()
{
qDebug("on_move_workbench_up_btn_clicked");
/*
    QTreeWidgetItem* item = toolbarTreeWidget->currentItem();
    if (item && item->parent() && toolbarTreeWidget->isItemSelected(item)) {
        QTreeWidgetItem* parent = item->parent();
        int index = parent->indexOfChild(item);
        if (index > 0) {
            // In case a separator should be moved we have to count the separators
            // which come before this one.
            // This is needed so that we can distinguish in moveUpCustomCommand
            // which separator it is.
            QByteArray data = item->data(0, Qt::UserRole).toByteArray();
            if (data == "Separator") {
                int countSep = 1;
                for (int i=0; i<index; i++) {
                    QByteArray d = parent->child(i)->data(0, Qt::UserRole).toByteArray();
                    if (d == "Separator") {
                        countSep++;
                    }
                }

                data += QByteArray::number(countSep);
            }

            parent->takeChild(index);
            parent->insertChild(index-1, item);
            toolbarTreeWidget->setCurrentItem(item);

            moveUpCustomCommand(parent->text(0), data);
        }
    }

    QVariant data = cbx_active_workbenches->itemData(cbx_active_workbenches->currentIndex(), Qt::UserRole);
    QString workbench = data.toString();
    exportWorkbenches(workbench.toAscii());
*/
}

/** Moves down an action */
void DlgWorkbenches::on_move_workbench_down_btn_clicked()
{
qDebug("on_move_workbench_down_btn_clicked");
/*
    QTreeWidgetItem* item = toolbarTreeWidget->currentItem();
    if (item && item->parent() && toolbarTreeWidget->isItemSelected(item)) {
        QTreeWidgetItem* parent = item->parent();
        int index = parent->indexOfChild(item);
        if (index < parent->childCount()-1) {
            // In case a separator should be moved we have to count the separators
            // which come before this one.
            // This is needed so that we can distinguish in moveDownCustomCommand
            // which separator it is.
            QByteArray data = item->data(0, Qt::UserRole).toByteArray();
            if (data == "Separator") {
                int countSep = 1;
                for (int i=0; i<index; i++) {
                    QByteArray d = parent->child(i)->data(0, Qt::UserRole).toByteArray();
                    if (d == "Separator") {
                        countSep++;
                    }
                }

                data += QByteArray::number(countSep);
            }

            parent->takeChild(index);
            parent->insertChild(index+1, item);
            toolbarTreeWidget->setCurrentItem(item);

            moveDownCustomCommand(parent->text(0), data);
        }
    }

    QVariant data = cbx_active_workbenches->itemData(cbx_active_workbenches->currentIndex(), Qt::UserRole);
    QString workbench = data.toString();
    exportWorkbenches(workbench.toAscii());
*/
}

void DlgWorkbenches::onAddMacroAction(const QByteArray& macro)
{
/*
    QVariant data = categoryBox->itemData(categoryBox->currentIndex(), Qt::UserRole);
    QString group = data.toString();
    if (group == QLatin1String("Macros"))
    {
        CommandManager & cCmdMgr = Application::Instance->commandManager();
        Command* pCmd = cCmdMgr.getCommandByName(macro);

        QTreeWidgetItem* item = new QTreeWidgetItem(commandTreeWidget);
        item->setText(1, QString::fromUtf8(pCmd->getMenuText()));
        item->setToolTip(1, QString::fromUtf8(pCmd->getToolTipText()));
        item->setData(1, Qt::UserRole, macro);
        item->setSizeHint(0, QSize(32, 32));
        item->setBackgroundColor(0, Qt::lightGray);
        if (pCmd->getPixmap())
            item->setIcon(0, BitmapFactory().pixmap(pCmd->getPixmap()));
    }
*/
}

void DlgWorkbenches::onRemoveMacroAction(const QByteArray& macro)
{
/*
    QVariant data = categoryBox->itemData(categoryBox->currentIndex(), Qt::UserRole);
    QString group = data.toString();
    if (group == QLatin1String("Macros"))
    {
        for (int i=0; i<commandTreeWidget->topLevelItemCount(); i++) {
            QTreeWidgetItem* item = commandTreeWidget->topLevelItem(i);
            QByteArray command = item->data(1, Qt::UserRole).toByteArray();
            if (command == macro) {
                commandTreeWidget->takeTopLevelItem(i);
                delete item;
                break;
            }
        }
    }
*/
}

void DlgWorkbenches::onModifyMacroAction(const QByteArray& macro)
{
/*
    QVariant data = categoryBox->itemData(categoryBox->currentIndex(), Qt::UserRole);
    QString group = data.toString();
    if (group == QLatin1String("Macros"))
    {
        CommandManager & cCmdMgr = Application::Instance->commandManager();
        Command* pCmd = cCmdMgr.getCommandByName(macro);
        // the left side
        for (int i=0; i<commandTreeWidget->topLevelItemCount(); i++) {
            QTreeWidgetItem* item = commandTreeWidget->topLevelItem(i);
            QByteArray command = item->data(1, Qt::UserRole).toByteArray();
            if (command == macro) {
                item->setText(1, QString::fromUtf8(pCmd->getMenuText()));
                item->setToolTip(1, QString::fromUtf8(pCmd->getToolTipText()));
                item->setData(1, Qt::UserRole, macro);
                item->setSizeHint(0, QSize(32, 32));
                item->setBackgroundColor(0, Qt::lightGray);
                if (pCmd->getPixmap())
                    item->setIcon(0, BitmapFactory().pixmap(pCmd->getPixmap()));
                break;
            }
        }
        // the right side
        for (int i=0; i<toolbarTreeWidget->topLevelItemCount(); i++) {
            QTreeWidgetItem* toplevel = toolbarTreeWidget->topLevelItem(i);
            for (int j=0; j<toplevel->childCount(); j++) {
                QTreeWidgetItem* item = toplevel->child(j);
                QByteArray command = item->data(0, Qt::UserRole).toByteArray();
                if (command == macro) {
                    item->setText(0, QString::fromUtf8(pCmd->getMenuText()));
                    if (pCmd->getPixmap())
                        item->setIcon(0, BitmapFactory().pixmap(pCmd->getPixmap()));
                }
            }
        }
    }
*/
}

void DlgWorkbenches::changeEvent(QEvent *e)
{
/*
    if (e->type() == QEvent::LanguageChange) {
        this->retranslateUi(this);
        int count = categoryBox->count();

        CommandManager & cCmdMgr = Application::Instance->commandManager();
        for (int i=0; i<count; i++) {
            QVariant data = categoryBox->itemData(i, Qt::UserRole);
            std::vector<Command*> aCmds = cCmdMgr.getGroupCommands(data.toByteArray());
            if (!aCmds.empty()) {
                QString text = qApp->translate(aCmds[0]->className(), aCmds[0]->getGroupName());
                categoryBox->setItemText(i, text);
            }
        }
        on_categoryBox_activated(categoryBox->currentIndex());
    }
    QWidget::changeEvent(e);
*/
}

// -------------------------------------------------------------

/* TRANSLATOR Gui::Dialog::DlgWorkbenchesImp */

/**
 *  Constructs a DlgWorkbenchesImp which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
DlgWorkbenchesImp::DlgWorkbenchesImp( QWidget* parent )
    : DlgWorkbenches(DlgWorkbenches::Toolbar, parent)
{
}

/** Destroys the object and frees any allocated resources */
DlgWorkbenchesImp::~DlgWorkbenchesImp()
{
}

void DlgWorkbenchesImp::addCustomToolbar(const QString& name)
{/*
    QVariant data = cbx_active_workbenches->itemData(cbx_active_workbenches->currentIndex(), Qt::UserRole);
    Workbench* w = WorkbenchManager::instance()->active();
    if (w && w->name() == std::string((const char*)data.toByteArray())) {
        QToolBar* bar = getMainWindow()->addToolBar(name);
        bar->setObjectName(name);
    }
*/
}

void DlgWorkbenchesImp::removeCustomToolbar(const QString& name)
{
/*
    QVariant data = cbx_active_workbenches->itemData(cbx_active_workbenches->currentIndex(), Qt::UserRole);
    Workbench* w = WorkbenchManager::instance()->active();
    if (w && w->name() == std::string((const char*)data.toByteArray())) {
        QList<QToolBar*> bars = getMainWindow()->findChildren<QToolBar*>(name);
        if (bars.size() != 1)
            return;

        QToolBar* tb = bars.front();
        getMainWindow()->removeToolBar(tb);
        delete tb;
    }
*/
}

void DlgWorkbenchesImp::renameCustomToolbar(const QString& old_name, const QString& new_name)
{
 /*   QVariant data = cbx_active_workbenches->itemData(cbx_active_workbenches->currentIndex(), Qt::UserRole);

    Workbench* w = WorkbenchManager::instance()->active();
    if (w && w->name() == std::string((const char*)data.toByteArray())) {
        QList<QToolBar*> bars = getMainWindow()->findChildren<QToolBar*>(old_name);
        if (bars.size() != 1)
            return;

        QToolBar* tb = bars.front();
        tb->setObjectName(new_name);
        tb->setWindowTitle(new_name);
    }
*/
}

void DlgWorkbenchesImp::addCustomCommand(const QString& name, const QByteArray& cmd)
{
 /*   QVariant data = cbx_active_workbenches->itemData(cbx_active_workbenches->currentIndex(), Qt::UserRole);
    Workbench* w = WorkbenchManager::instance()->active();
    if (w && w->name() == std::string((const char*)data.toByteArray())) {
        QList<QToolBar*> bars = getMainWindow()->findChildren<QToolBar*>(name);
        if (bars.size() != 1)
            return;

        if (cmd == "Separator") {
            QAction* action = bars.front()->addSeparator();
            action->setData(QByteArray("Separator"));
        }
        else {
            CommandManager& mgr = Application::Instance->commandManager();
            mgr.addTo(cmd, bars.front());
        }
    }
*/
}

void DlgWorkbenchesImp::removeCustomCommand(const QString& name, const QByteArray& userdata)
{
 /*   QVariant data = cbx_active_workbenches->itemData(cbx_active_workbenches->currentIndex(), Qt::UserRole);
    Workbench* w = WorkbenchManager::instance()->active();
    if (w && w->name() == std::string((const char*)data.toByteArray())) {
        QList<QToolBar*> bars = getMainWindow()->findChildren<QToolBar*>(name);
        if (bars.size() != 1)
            return;

        QByteArray cmd = userdata;
        int numSep = 0, indexSep = 0;
        if (cmd.startsWith("Separator")) {
            numSep = cmd.mid(9).toInt();
            cmd = "Separator";
        }
        QList<QAction*> actions = bars.front()->actions();
        for (QList<QAction*>::ConstIterator it = actions.begin(); it != actions.end(); ++it) {
            if ((*it)->data().toByteArray() == cmd) {
                // if we move a separator then make sure to pick up the right one
                if (numSep > 0) {
                    if (++indexSep < numSep)
                        continue;
                }
                bars.front()->removeAction(*it);
                break;
            }
        }
    }
*/
}

void DlgWorkbenchesImp::moveUpCustomCommand(const QString& name, const QByteArray& userdata)
{
/*    QVariant data = cbx_active_workbenches->itemData(cbx_active_workbenches->currentIndex(), Qt::UserRole);
    Workbench* w = WorkbenchManager::instance()->active();
    if (w && w->name() == std::string((const char*)data.toByteArray())) {
        QList<QToolBar*> bars = getMainWindow()->findChildren<QToolBar*>(name);
        if (bars.size() != 1)
            return;

        QByteArray cmd = userdata;
        int numSep = 0, indexSep = 0;
        if (cmd.startsWith("Separator")) {
            numSep = cmd.mid(9).toInt();
            cmd = "Separator";
        }
        QList<QAction*> actions = bars.front()->actions();
        QAction* before=0;
        for (QList<QAction*>::ConstIterator it = actions.begin(); it != actions.end(); ++it) {
            if ((*it)->data().toByteArray() == cmd) {
                // if we move a separator then make sure to pick up the right one
                if (numSep > 0) {
                    if (++indexSep < numSep) {
                        before = *it;
                        continue;
                    }
                }
                if (before != 0) {
                    bars.front()->removeAction(*it);
                    bars.front()->insertAction(before, *it);
                    break;
                }
            }

            before = *it;
        }
    }
*/
}

void DlgWorkbenchesImp::moveDownCustomCommand(const QString& name, const QByteArray& userdata)
{
 /*   QVariant data = cbx_active_workbenches->itemData(cbx_active_workbenches->currentIndex(), Qt::UserRole);
    Workbench* w = WorkbenchManager::instance()->active();
    if (w && w->name() == std::string((const char*)data.toByteArray())) {
        QList<QToolBar*> bars = getMainWindow()->findChildren<QToolBar*>(name);
        if (bars.size() != 1)
            return;

        QByteArray cmd = userdata;
        int numSep = 0, indexSep = 0;
        if (cmd.startsWith("Separator")) {
            numSep = cmd.mid(9).toInt();
            cmd = "Separator";
        }
        QList<QAction*> actions = bars.front()->actions();
        for (QList<QAction*>::ConstIterator it = actions.begin(); it != actions.end(); ++it) {
            if ((*it)->data().toByteArray() == cmd) {
                // if we move a separator then make sure to pick up the right one
                if (numSep > 0) {
                    if (++indexSep < numSep)
                        continue;
                }
                QAction* act = *it;
                if (*it == actions.back())
                    break; // we're already on the last element
                ++it;
                // second last item
                if (*it == actions.back()) {
                    bars.front()->removeAction(act);
                    bars.front()->addAction(act);
                    break;
                }
                ++it;
                bars.front()->removeAction(act);
                bars.front()->insertAction(*it, act);
                break;
            }
        }
    }
*/
}

void DlgWorkbenchesImp::changeEvent(QEvent *e)
{
/*    if (e->type() == QEvent::LanguageChange) {
    }
    DlgWorkbenches::changeEvent(e);
*/
}

#include "moc_DlgWorkbenchesImp.cpp"
