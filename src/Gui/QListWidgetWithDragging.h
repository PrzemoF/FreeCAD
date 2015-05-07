#ifndef QLISTWIDGETWITHDRAGGING_HPP
#define QLISTWIDGETWITHDRAGGING_HPP

#include <QListWidget>
#include <QDragMoveEvent>

class QListWidgetWithDragging : public QListWidget {

public:
    QListWidgetWithDragging (QWidget * parent) :
        QListWidget(parent) {}

protected:
/* Overriden dragMoveEvent prevents dragging items that originated
 * from the same list. Dragging from outside is still allowed
 */
    void dragMoveEvent(QDragMoveEvent *e) {
        if (e->source() != this) {
            e->accept();
        } else {
            e->ignore();
        }
    }
};

#endif
