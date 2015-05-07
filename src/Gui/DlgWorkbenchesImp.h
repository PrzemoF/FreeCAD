/***************************************************************************
 *   Copyright (c) 2015 FreeCAD Developers                                 *
 *   Author: Przemo Firszt <przemo@firszt.eu>                              *
 *   Based on DlgToolbars.h file                                           *
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


#ifndef GUI_DIALOG_DLGWORKBENCHES_IMP_H
#define GUI_DIALOG_DLGWORKBENCHES_IMP_H

#include "ui_DlgWorkbenches.h"
#include "PropertyPage.h"

namespace Gui {
namespace Dialog {

class DlgWorkbenches : public CustomizeActionPage, public Ui_DlgWorkbenches
{
    Q_OBJECT

protected:
    enum Type { Toolbar, Toolboxbar };

    DlgWorkbenches(Type, QWidget* parent = 0);
    virtual ~DlgWorkbenches();

protected Q_SLOTS:
    void on_add_to_enabled_workbenches_btn_clicked();
    void on_remove_from_enabled_workbenches_btn_clicked();
    void on_move_workbench_up_btn_clicked();
    void on_move_workbench_down_btn_clicked();
    void onAddMacroAction(const QByteArray&);
    void onRemoveMacroAction(const QByteArray&);
    void onModifyMacroAction(const QByteArray&);

protected:
    void changeEvent(QEvent *e);
    void hideEvent(QHideEvent * event);
    virtual void addCustomToolbar(const QString&);
    virtual void removeCustomToolbar(const QString&);
    virtual void renameCustomToolbar(const QString&, const QString&);
    virtual void addCustomCommand(const QString&, const QByteArray&);
    virtual void removeCustomCommand(const QString&, const QByteArray&);
    virtual void moveUpCustomCommand(const QString&, const QByteArray&);
    virtual void moveDownCustomCommand(const QString&, const QByteArray&);

private:
    QStringList load_enabled_workbenches();
    void save_enabled_workbenches();

private:
    Type type;
};

class DlgWorkbenchesImp : public DlgWorkbenches
{
    Q_OBJECT

public:
    DlgWorkbenchesImp(QWidget* parent = 0);
    ~DlgWorkbenchesImp();

protected:
    void changeEvent(QEvent *e);
    virtual void addCustomToolbar(const QString&);
    virtual void removeCustomToolbar(const QString&);
    virtual void renameCustomToolbar(const QString&, const QString&);
    virtual void addCustomCommand(const QString&, const QByteArray&);
    virtual void removeCustomCommand(const QString&, const QByteArray&);
    virtual void moveUpCustomCommand(const QString&, const QByteArray&);
    virtual void moveDownCustomCommand(const QString&, const QByteArray&);
};

} // namespace Dialog
} // namespace Gui

#endif // GUI_DIALOG_DLGWORKBENCHES_IMP_H
