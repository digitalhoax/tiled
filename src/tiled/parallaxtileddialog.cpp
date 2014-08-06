#include "parallaxtileddialog.h"
#include "ui_parallaxtileddialog.h"

using namespace Tiled;
using namespace Tiled::Internal;

ParallaxTiledDialog::ParallaxTiledDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ParallaxTiledDialog)
{
	m_ui->setupUi(this);
}

ParallaxTiledDialog::~ParallaxTiledDialog()
{
    delete m_ui;
}

double ParallaxTiledDialog::parallaxX()
{
    return m_ui->parallaxValueX->value();
}
