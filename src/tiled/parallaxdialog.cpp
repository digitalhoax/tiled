#include "parallaxdialog.h"
#include "ui_parallaxdialog.h"

#include <QSettings>
#include <QMessageBox>

using namespace Tiled;
using namespace Tiled::Internal;

ParallaxDialog::ParallaxDialog(QWidget *parent) : QDialog(parent), m_ui(new Ui::ParallaxDialog)
{
	m_ui->setupUi(this);
}

ParallaxDialog::~ParallaxDialog()
{
    delete m_ui;
}

double ParallaxDialog::parallaxX()
{
    return m_ui->parallaxValueX->value();
}

double ParallaxDialog::parallaxY()
{
    return m_ui->parallaxValueY->value();
}
