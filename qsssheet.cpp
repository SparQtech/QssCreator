//Modified "property sheet by QCasper (Anton I Alferov)"
//-----------------------------------------------------------------------
#include "qsssheet.h"
#include <qheaderview.h>
#include <qtranslator.h>
#include <qapplication.h>
#include <qpainter.h>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QMenu>
#include<QMessageBox>

//-----------------------------------------------------------------------
void PlusMinusWg::paintEvent(QPaintEvent *) {
	QPainter p(this);
    p.setPen(QPen(QColor(m_BgColor)));
	p.setBrush(m_BgColor);
	p.drawRect(rect());

	p.setPen(QPen(QColor(142,153,125)));

	QLinearGradient lg(m_PMRect.topLeft(), m_PMRect.bottomRight());
	lg.setColorAt(0,QColor(255,255,255));
	lg.setColorAt(1,QColor(195,186,170));
    p.setBrush(lg);
    p.drawRoundRect(m_PMRect, 5, 5);///OBSOLETE!!!

	int l = 6*m_PMRect.width()/7;
	p.setPen(QPen(QColor(0,0,0)));
	p.drawLine(m_PMRect.x()+l, m_PMRect.center().y()+1,
		m_PMRect.x() + m_PMRect.width()-l, m_PMRect.center().y()+1);
	if (m_Closed)
		p.drawLine(m_PMRect.center().x()+1, m_PMRect.y()+l,
        m_PMRect.center().x()+1, m_PMRect.y() + m_PMRect.height() - l);
}
//-----------------------------------------------------------------------
void PlusMinusWg::resizeEvent(QResizeEvent *) {
    int h = rect().height();
    int rs = h*0.7;//2;
    int d = (h-rs)/2;

	m_PMRect.setSize(QSize(rs, rs));
	m_PMRect.moveTopLeft(QPoint(d, d));
}
//-----------------------------------------------------------------------
void PlusMinusWg::mousePressEvent(QMouseEvent *e) {
	if ((e->button() == Qt::LeftButton) && (m_PMRect.contains(e->pos()))) {
        m_Closed = !m_Closed;
		emit closed(m_Closed);
		update();
	}
}
//-----------------------------------------------------------------------
void PlusMinusWg::setBgColor(const QColor & color) {
	m_BgColor = color;
}
//-----------------------------------------------------------------------
void PlusMinusWg::open() {
	m_Closed = false;
	emit closed(m_Closed);
	update();
}
//-----------------------------------------------------------------------
void PlusMinusWg::close() {
	m_Closed = true;
	emit closed(m_Closed);
	update();
}
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
QssSheet::QssSheet(QWidget *parent):
QTableWidget(parent),
//m_CaptionColor(QColor(172,168,153)) {
m_CaptionColor(QColor(64,64,64)) {

	setColumnCount(2);
	setSelectionMode(QAbstractItemView::NoSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	horizontalHeader()->setStretchLastSection(true);
	setAlternatingRowColors(true);
	setTabKeyNavigation(true);

    setColumnWidth(0, 150);
	
	QStringList labels;
	labels << tr("Property") << tr("Value");
	setHorizontalHeaderLabels(labels);

    horizontalHeader()->setSectionsClickable(false);
    horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	verticalHeader()->hide();

    addNewRow = new QAction(tr("Add Row"), this);
    connect(addNewRow, SIGNAL(triggered()), this, SLOT(insertNewRow()));

}
//-----------------------------------------------------------------------
int QssSheet::addRow() {
	int row = rowCount();
	insertRow(row);
    m_RowHeight = static_cast<int>(rowHeight(row) * 0.65);//0.7
	setRowHeight(row, m_RowHeight);
	return row;
}
//-----------------------------------------------------------------------
void QssSheet::addCaption(const QString & name, const QString &hint, QColor &cap_color, bool closed) {
	if (name.isEmpty() || m_Captions.contains(name)) return;

    m_CaptionColor = cap_color;

	int row = addRow();

	PlusMinusWg *pm = new PlusMinusWg(name, this);
	(closed) ? pm->close() : pm->open();
    (closed) ? pm->setStatusTip("-") : pm->setStatusTip("+");
	m_Captions.insert(name, pm);
	connect(pm, SIGNAL(closed(bool)), this, SLOT(changeCaptionState(bool)));

	pm->setBgColor(m_CaptionColor);
    setCellWidget(row, 1, pm);

	QTableWidgetItem *value= new QTableWidgetItem(name);
	value->setBackgroundColor(m_CaptionColor);
    value->setToolTip(hint);

    QFont ft;
	ft.setPointSize(10);
	value->setFont(ft);
	value->setTextColor(Qt::white);
    value->setTextAlignment(Qt::AlignLeft);
    setItem(row, 0, value);

	m_LastAddedCaption = name;
}
//-----------------------------------------------------------------------
void QssSheet::addPropertyWidget(const QString & name, QWidget *widget) {
	if (name.isEmpty()) return;

	int row = addRow();
	QTableWidgetItem *property = new QTableWidgetItem(name);
    setItem(row, 0, property);
    property->setToolTip(name);
    setCellWidget(row, 1, widget);

	if (!m_LastAddedCaption.isEmpty())
		if (m_Captions[m_LastAddedCaption]->isClosed())
			setRowHeight(row, 0);
	m_Rows.insertMulti(m_LastAddedCaption, row);
}
//-----------------------------------------------------------------------
void QssSheet::changeCaptionState(bool closed) {
	PlusMinusWg *wg = (PlusMinusWg*)sender();
    if (closed){
		foreach(int row, m_Rows.values(wg->name()))
			setRowHeight(row, 0);
        wg->setStatusTip("-");
    }
    else{
		foreach(int row, m_Rows.values(wg->name()))
            setRowHeight(row, m_RowHeight);
        wg->setStatusTip("+");
    }
}
//-----------------------------------------------------------------------
QString QssSheet::getText(int row, int column) const
{
    QTableWidgetItem *it = this->item(row, column);
    QWidget *w = this->cellWidget(row, column);
    if (it) {
        return it->text();
    } else if (w) {
        return w->statusTip();
    }
    return "!";
}
//-----------------------------------------------------------------------
QString QssSheet::getObj(int row, int column)
{
    QString out, name;
    QWidget *w = this->cellWidget(row, column);
    if (w){
        name = w->accessibleName();
        out =  w->windowTitle();
        return out;
    }
    return "#";
}
//-----------------------------------------------------------------------
void QssSheet::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);
    menu->addAction(addNewRow);
    menu->exec(event->globalPos());
    delete menu;
}
//-----------------------------------------------------------------------
void QssSheet::insertNewRow()
{
    QMessageBox msgBox;
    msgBox.setText("Ooooops....");
    msgBox.exec();
}
//-----------------------------------------------------------------------
