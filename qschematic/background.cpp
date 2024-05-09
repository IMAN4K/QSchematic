#include "background.hpp"

#include <QPen>
#include <QBrush>
#include <QPainter>

using namespace QSchematic;

Background::Background(QGraphicsItem* parent) :
    QGraphicsRectItem(parent)
{
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption, true);  // For QStyleOptionGraphicsItem::exposedRect
}

void
Background::setSettings(const Settings& settings)
{
    m_settings = settings;
}

void
Background::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    // ToDo: Use QStyleOptionGraphicsItem::exposedRect

    // Background pen
    QPen backgroundPen;

    // Background brush
    QBrush backgroundBrush;

    // Grid pen
    QPen gridPen;
    gridPen.setStyle(Qt::SolidLine);
    gridPen.setColor(Qt::gray);
    gridPen.setCapStyle(Qt::RoundCap);
    gridPen.setWidth(m_settings.gridPointSize);

    // Grid brush
    QBrush gridBrush;
    gridBrush.setStyle(Qt::NoBrush);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, m_settings.antialiasing);

    qDebug() << rect();

    // Draw background
    painter->setPen(backgroundPen);
    painter->setBrush(backgroundBrush);
    painter->drawRect(rect());

    // Draw the grid if supposed to
    if (m_settings.showGrid && (m_settings.gridSize > 0)) {
        qreal left = int(rect().left()) - (int(rect().left()) % m_settings.gridSize);
        qreal top = int(rect().top()) - (int(rect().top()) % m_settings.gridSize);

        // Create a list of points
        QVector<QPointF> points;
        for (qreal x = left; x < rect().right(); x += m_settings.gridSize) {
            for (qreal y = top; y < rect().bottom(); y += m_settings.gridSize)
                points.append(QPointF(x,y));
        }

        // Draw the actual grid points
        painter->setPen(gridPen);
        painter->setBrush(gridBrush);
        painter->drawPoints(points.data(), points.size());
    }

    // Mark the origin if supposed to
    if (m_settings.debug) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(Qt::red));
        painter->drawEllipse(-6, -6, 12, 12);
    }

    painter->restore();
}
