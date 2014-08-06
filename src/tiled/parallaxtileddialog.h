#ifndef PARALLAXTILEDDIALOG_H
#define PARALLAXTILEDDIALOG_H

#include <QDialog>

namespace Ui {
class ParallaxTiledDialog;
}

namespace Tiled {
namespace Internal {

class ParallaxTiledDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParallaxTiledDialog(QWidget *parent = 0);
    ~ParallaxTiledDialog();

    double parallaxX();

private:
    Ui::ParallaxTiledDialog * m_ui;
};

}
}

#endif // PARALLAXTILEDDIALOG_H
