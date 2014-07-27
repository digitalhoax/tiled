#ifndef PARALLAXDIALOG_H
#define PARALLAXDIALOG_H

#include <QDialog>

namespace Ui {
class ParallaxDialog;
}
namespace Tiled {
namespace Internal {


class ParallaxDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParallaxDialog(QWidget *parent = 0);
    ~ParallaxDialog();

    double parallaxX();
    double parallaxY();

private:
    Ui::ParallaxDialog * m_ui;
};

} // namespace Internal
} // namespace Tiled

#endif // PARALLAXDIALOG_H
