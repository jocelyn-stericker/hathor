#ifndef HSHOUTBOX_H
#define HSHOUTBOX_H

#include <QWidget>
#include "hshout.h"
#include "hgrowingwidget.h"

namespace Ui {
class HShoutBox;
}

class HShoutBox : public QWidget
{
    Q_OBJECT
    
    HShout& s_rep;
public:
    explicit HShoutBox(HShout& rep, QWidget *parent);
    ~HShoutBox();

private:
    Ui::HShoutBox *ui;
};

#endif // HSHOUTBOX_H